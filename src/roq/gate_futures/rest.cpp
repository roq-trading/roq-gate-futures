/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gate_futures/rest.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"
#include "roq/core/charconv.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/gate_futures/flags.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

namespace {
const auto NAME = "rest"sv;

const Mask SUPPORTS{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};

const auto ALLOW_PIPELINING = true;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::rest_uri();
  core::web::Client::Config config{
      .decode_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
      .validate_certificate = server::Flags::tls_validate_certificate(),
      .uris = {&uri, 1},
      .proxy = Flags::rest_proxy(),
      .user_agent = ROQ_PACKAGE_NAME,
      .connection = core::http::Connection::KEEP_ALIVE,
      .allow_pipelining = true,
      .request_timeout = Flags::rest_request_timeout(),
      .ping_frequency = Flags::rest_ping_freq(),
      .ping_path = Flags::rest_ping_path(),
  };
  return core::web::Client{handler, context, config};
}

template <typename T>
void emplace(MBPUpdate &result, const T &item) {
  new (&result) MBPUpdate{
      .price = item.price,
      .quantity = item.size,
      .implied_quantity = NaN,
      .price_level = {},
      .number_of_orders = {},
  };
}
}  // namespace

Rest::Rest(Handler &handler, core::io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      connection_(create_connection(*this, context)), decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .currencies = create_metrics(name_, "currencies"sv),
          .currencies_ack = create_metrics(name_, "currencies_ack"sv),
          .contracts = create_metrics(name_, "contracts"sv),
          .contracts_ack = create_metrics(name_, "contracts_ack"sv),
          .order_book = create_metrics(name_, "order_book"sv),
          .order_book_ack = create_metrics(name_, "order_book_ack"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
      },
      shared_(shared),
      download_(Flags::rest_request_timeout(), [this](auto state) { return download(state); }) {
}

void Rest::operator()(const Event<Start> &) {
  connection_.start();
}

void Rest::operator()(const Event<Stop> &) {
  connection_.stop();
}

void Rest::operator()(const Event<Timer> &event) {
  auto now = event.value.now;
  connection_.refresh(now);
  if (ready())
    check_request_queue(now);
}

void Rest::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.currencies, metrics::PROFILE)
      .write(profile_.currencies_ack, metrics::PROFILE)
      .write(profile_.contracts, metrics::PROFILE)
      .write(profile_.contracts_ack, metrics::PROFILE)
      .write(profile_.order_book, metrics::PROFILE)
      .write(profile_.order_book_ack, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

void Rest::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    const StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::HTTP,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void Rest::operator()(const core::web::Client::Connected &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void Rest::operator()(const core::web::Client::Disconnected &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
}

void Rest::operator()(const core::web::Client::Latency &latency) {
  auto trace_info = server::create_trace_info();
  const ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

uint32_t Rest::download(RestState state) {
  switch (state) {
    using enum RestState;
    case UNDEFINED:
      assert(false);
      break;
    case CURRENCIES:
      get_currencies();
      return 1;
    case CONTRACTS:
      get_contracts();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

// currencies

void Rest::get_currencies() {
  profile_.currencies([&]() {
    auto method = core::http::Method::GET;
    auto path = "/spot/currencies"sv;
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    connection_(
        "currencies"sv,
        request,
        [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          get_currencies_ack(event, sequence);
        });
  });
}

void Rest::get_currencies_ack(const Trace<core::web::Response const> &event, uint32_t sequence) {
  profile_.currencies_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = RestState::CURRENCIES;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      const auto currencies = core::json::Parser::create<json::Currencies>(body, buffer);
      Trace event(trace_info, currencies);
      (*this)(event);
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void Rest::operator()(const Trace<json::Currencies const> &event) {
  auto &[trace_info, currencies] = event;
  log::info<4>("currencies={}"sv, currencies);
}

// contracts

void Rest::get_contracts() {
  profile_.contracts([&]() {
    auto method = core::http::Method::GET;
    auto path = shared_.api.get_contracts;
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    connection_(
        "contracts"sv,
        request,
        [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          get_contracts_ack(event, sequence);
        });
  });
}

void Rest::get_contracts_ack(const Trace<core::web::Response const> &event, uint32_t sequence) {
  profile_.contracts_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = RestState::CONTRACTS;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      const auto contracts = core::json::Parser::create<json::Contracts>(body, buffer);
      Trace event(trace_info, contracts);
      (*this)(event);
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void Rest::operator()(const Trace<json::Contracts const> &event) {
  auto &[trace_info, contracts] = event;
  log::info<4>("contracts={}"sv, contracts);
  std::vector<Symbol> symbols;
  symbols.reserve(std::size(contracts.data));
  size_t counter = 0;
  for (size_t i = 0; i < std::size(contracts.data); ++i) {
    auto &item = contracts.data[i];
    log::info<2>("item={}"sv, item);
    log::debug("item={}"sv, item);
    auto symbol = item.name;
    if (shared_.discard_symbol(symbol))
      continue;
    if (all_symbols_.emplace(symbol).second)  // only include new
      symbols.emplace_back(symbol);
    ++counter;
    const ReferenceData reference_data{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .description = symbol,
        .security_type = {},
        .base_currency = {},
        .quote_currency = {},
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = item.order_price_round,
        .multiplier = 1.0,
        .min_trade_vol = item.order_size_min,
        .max_trade_vol = item.order_size_max,
        .trade_vol_step_size = NaN,
        .option_type = {},
        .strike_currency = {},
        .strike_price = NaN,
        .underlying = {},
        .time_zone = {},
        .issue_date = {},
        .settlement_date = {},
        .expiry_datetime = {},
        .expiry_datetime_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, reference_data, true);
  }
  if (!std::empty(symbols)) {
    SymbolsUpdate contracts_update{
        .symbols = symbols,
    };
    handler_(contracts_update);
  }
  if (counter > 0) [[unlikely]]
    log::info("Symbols {} / {}"sv, counter, std::size(contracts.data));
}

// order book

void Rest::get_order_book(const std::string_view &symbol) {
  profile_.order_book([&]() {
    auto method = core::http::Method::GET;
    auto path = shared_.api.get_order_book;
    auto query =
        fmt::format("?contract={}&limit={}&with_id=true"sv, symbol, Flags::order_book_depth());
    core::web::Request request{
        .method = method,
        .path = path,
        .query = query,
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    connection_(
        "order_book"sv,
        request,
        [this, symbol = std::string{symbol}]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          get_order_book_ack(event, symbol);
        });
  });
}

void Rest::get_order_book_ack(
    const Trace<core::web::Response const> &event, const std::string_view &symbol) {
  profile_.order_book_ack([&]() {
    auto &[trace_info, response] = event;
    try {
      auto [status, category, body] = response.result();
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      const auto order_book = core::json::Parser::create<json::OrderBook>(body, buffer);
      Trace event(trace_info, order_book);
      (*this)(event, symbol);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    }
  });
}

void Rest::operator()(const Trace<json::OrderBook const> &event, const std::string_view &symbol) {
  // auto &[trace_info, order_book] = event;
  auto &trace_info = event.trace_info;
  auto &order_book = event.value;
  log::info<3>("trace_info={}, order_book={}"sv, trace_info, order_book);
  auto sequence = order_book.id;
  auto &collector = shared_.mbp_collector[symbol];
  core::back_emplacer bids(shared_.bids), asks(shared_.asks);
  for (auto &item : order_book.bids)
    bids.emplace_back([&item](auto &result) { emplace(result, item); });
  for (auto &item : order_book.asks)
    asks.emplace_back([&item](auto &result) { emplace(result, item); });
  auto exchange_time_utc = std::chrono::nanoseconds{};
  try {
    collector(
        bids,
        asks,
        sequence,
        [&](auto &bids, auto &asks, auto sequence) {  // snapshot
          log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
          const MarketByPriceUpdate market_by_price_update{
              .stream_id = stream_id_,
              .exchange = Flags::exchange(),
              .symbol = symbol,
              .bids = bids,
              .asks = asks,
              .update_type = UpdateType::SNAPSHOT,
              .exchange_time_utc = exchange_time_utc,
              .exchange_sequence = collector.last_sequence(),
              .price_decimals = {},
              .quantity_decimals = {},
              .checksum = {},
          };
          Trace event(trace_info, market_by_price_update);
          shared_(event, true, [&](auto &market_by_price) {
            collector.apply(market_by_price, sequence, true);
          });
        },
        [&](auto retries) {  // request
          log::debug(R"(REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
          if (Flags::ws_mbp_request_max_retries() &&
              Flags::ws_mbp_request_max_retries() < retries) {
            log::fatal(R"(Unexpected: symbol="{}", retries={})"sv, symbol, retries);
          }
          shared_.depth_request_queue.emplace_back(symbol);
        });
  } catch (BadState &) {
    log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
    // XXX HANS publish stale
    collector.clear();
    shared_.depth_request_queue.emplace_back(symbol);
  }
}

// queue

void Rest::check_request_queue(std::chrono::nanoseconds now) {
  shared_.depth_request_queue.dispatch(
      [&](auto now) { return shared_.rate_limiter.can_request(now); },
      [&](auto &symbol) {
        log::debug(R"(Requesting order book snapshot symbol="{}")"sv, symbol);
        get_order_book(symbol);
      },
      now);
}

}  // namespace gate_futures
}  // namespace roq
