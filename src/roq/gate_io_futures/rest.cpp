/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gate_io/rest.h"

#include <algorithm>
#include <utility>

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/back_emplacer.h"
#include "roq/core/charconv.h"

#include "roq/core/json/parser.h"

#include "roq/core/metrics/factory.h"

#include "roq/gate_io/flags.h"

using namespace std::literals;

namespace roq {
namespace gate_io {

namespace {
static const auto NAME = "rest"sv;

static const auto SUPPORTS = utils::Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};

static const auto ALLOW_PIPELINING = true;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

Rest::Rest(Handler &handler, core::io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      connection_(
          *this,
          context,
          Flags::decode_buffer_size(),
          Flags::encode_buffer_size(),
          core::URI(Flags::rest_uri()),
          ROQ_PACKAGE_NAME,
          core::http::Connection::KEEP_ALIVE,
          ALLOW_PIPELINING,
          Flags::rest_request_timeout(),
          Flags::rest_ping_freq(),
          Flags::rest_ping_path()),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .currencies = create_metrics(name_, "currencies"sv),
          .currencies_ack = create_metrics(name_, "currencies_ack"sv),
          .currency_pairs = create_metrics(name_, "currency_pairs"sv),
          .currency_pairs_ack = create_metrics(name_, "currency_pairs_ack"sv),
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
}

void Rest::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.currencies, metrics::PROFILE)
      .write(profile_.currencies_ack, metrics::PROFILE)
      .write(profile_.currency_pairs, metrics::PROFILE)
      .write(profile_.currency_pairs_ack, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

void Rest::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::REST,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"sv, stream_status);
    server::create_trace_and_dispatch(handler_, trace_info, stream_status);
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
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

uint32_t Rest::download(RestState state) {
  switch (state) {
    case RestState::UNDEFINED:
      assert(false);
      break;
    case RestState::CURRENCIES:
      get_currencies();
      return 1;
    case RestState::CURRENCY_PAIRS:
      get_currency_pairs();
      return 1;
    case RestState::DONE:
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
          server::Trace event(trace_info, response);
          get_currencies_ack(event, sequence);
        });
  });
}

void Rest::get_currencies_ack(const server::Trace<core::web::Response> &event, uint32_t sequence) {
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
      auto currencies = core::json::Parser::create<json::Currencies>(body, buffer);
      server::Trace event(trace_info, currencies);
      (*this)(event);
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void Rest::operator()(const server::Trace<json::Currencies> &event) {
  auto &[trace_info, currencies] = event;
  log::info<4>("currencies={}"sv, currencies);
}

// currency_pairs

void Rest::get_currency_pairs() {
  profile_.currency_pairs([&]() {
    auto method = core::http::Method::GET;
    auto path = "/spot/currency_pairs"sv;
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
        "currency_pairs"sv,
        request,
        [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          server::Trace event(trace_info, response);
          get_currency_pairs_ack(event, sequence);
        });
  });
}

void Rest::get_currency_pairs_ack(
    const server::Trace<core::web::Response> &event, uint32_t sequence) {
  profile_.currency_pairs_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = RestState::CURRENCY_PAIRS;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      auto currency_pairs = core::json::Parser::create<json::CurrencyPairs>(body, buffer);
      server::Trace event(trace_info, currency_pairs);
      (*this)(event);
      download_.check(state);
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void Rest::operator()(const server::Trace<json::CurrencyPairs> &event) {
  auto &[trace_info, currency_pairs] = event;
  log::info<4>("currency_pairs={}"sv, currency_pairs);
  std::vector<std::string> symbols;
  symbols.reserve(std::size(currency_pairs.data));
  size_t counter = 0;
  for (size_t i = 0; i < std::size(currency_pairs.data); ++i) {
    auto &item = currency_pairs.data[i];
    log::info<2>("item={}"sv, item);
    auto symbol = item.id;
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
        .base_currency = item.base,
        .quote_currency = item.quote,
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = NaN,  // precision ?
        .multiplier = 1.0,
        .min_trade_vol = item.min_quote_amount,
        .max_trade_vol = NaN,
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
    server::create_trace_and_dispatch(handler_, trace_info, reference_data, true);
  }
  if (!std::empty(symbols)) {
    SymbolsUpdate currency_pairs_update{
        .symbols = symbols,
    };
    handler_(currency_pairs_update);
  }
  if (ROQ_UNLIKELY(counter > 0))
    log::info("Symbols {} / {}"sv, counter, std::size(currency_pairs.data));
}

}  // namespace gate_io
}  // namespace roq
