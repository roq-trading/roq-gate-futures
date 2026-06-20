/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/gate_futures/gateway/rest.hpp"

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/core/json/parser.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {
namespace gateway {

// === CONSTANTS ===

namespace {
auto const NAME = "rest"sv;

auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.rest.uri;
  auto config = web::rest::Client::Config{
      // connection
      .interface = {},
      .proxy = settings.rest.proxy,
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = {},
      .disconnect_on_idle_timeout = {},
      .connection = web::http::Connection::KEEP_ALIVE,
      // request
      .allow_pipelining = true,
      .request_timeout = settings.rest.request_timeout,
      // response
      .suspend_on_retry_after = {},
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .ping_frequency = settings.rest.ping_freq,
      .ping_path = settings.rest.ping_path,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::rest::Client::create(handler, context, config);
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

Rest::Rest(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .currencies = create_metrics(shared.settings, name_, "currencies"sv),
          .currencies_ack = create_metrics(shared.settings, name_, "currencies_ack"sv),
          .contracts = create_metrics(shared.settings, name_, "contracts"sv),
          .contracts_ack = create_metrics(shared.settings, name_, "contracts_ack"sv),
          .order_book = create_metrics(shared.settings, name_, "order_book"sv),
          .order_book_ack = create_metrics(shared.settings, name_, "order_book_ack"sv),
          .candlesticks = create_metrics(shared.settings, name_, "candlesticks"sv),
          .candlesticks_ack = create_metrics(shared.settings, name_, "candlesticks_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      shared_{shared}, download_{shared.settings.rest.request_timeout, [this](auto state) { return download(state); }} {
}

void Rest::operator()(Event<Start> const &) {
  (*connection_).start();
}

void Rest::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void Rest::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if (ready()) {
    check_request_queue(now);
  }
}

void Rest::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.currencies, metrics::Type::PROFILE)
      .write(profile_.currencies_ack, metrics::Type::PROFILE)
      .write(profile_.contracts, metrics::Type::PROFILE)
      .write(profile_.contracts_ack, metrics::Type::PROFILE)
      .write(profile_.order_book, metrics::Type::PROFILE)
      .write(profile_.order_book_ack, metrics::Type::PROFILE)
      .write(profile_.candlesticks, metrics::Type::PROFILE)
      .write(profile_.candlesticks_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

void Rest::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
  connection_status_ = connection_status;
  TraceInfo trace_info;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = {},
      .supports = SUPPORTS,
      .transport = Transport::TCP,
      .protocol = Protocol::HTTP,
      .encoding = {Encoding::JSON},
      .priority = Priority::PRIMARY,
      .connection_status = connection_status_,
      .reason = reason,
      .interface = (*connection_).get_interface(),
      .authority = (*connection_).get_current_authority(),
      .path = (*connection_).get_current_path(),
      .proxy = (*connection_).get_proxy(),
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(shared_.dispatcher, trace_info, stream_status);
}

void Rest::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void Rest::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading()) {
    download_.reset();
  }
}

void Rest::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(shared_.dispatcher, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

uint32_t Rest::download(State state) {
  switch (state) {
    using enum State;
    case UNDEFINED:
      assert(false);
      break;
    case CURRENCIES:
      if (shared_.settings.rest.disable_currencies_download) {
        return 0;
      }
      get_currencies();
      return 1;
    case CONTRACTS:
      get_contracts();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

// currencies

void Rest::get_currencies() {
  profile_.currencies([&]() {
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.spot_currencies,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_currencies_ack(event, sequence);
    };
    (*connection_)("spot-currencies"sv, request, callback);
  });
}

void Rest::get_currencies_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const STATE = State::CURRENCIES;
  profile_.currencies_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        protocol::json::CurrenciesAck currencies_ack{body, decode_buffer_};
        Trace event_2{event, currencies_ack};
        (*this)(event_2);
        download_.check(STATE);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void Rest::operator()(Trace<protocol::json::CurrenciesAck> const &event) {
  auto &[trace_info, currencies_ack] = event;
  log::info<4>("currencies_ack={}"sv, currencies_ack);
}

// contracts

void Rest::get_contracts() {
  profile_.contracts([&]() {
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.futures_contracts,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_contracts_ack(event, sequence);
    };
    (*connection_)("futures-contracts"sv, request, callback);
  });
}

void Rest::get_contracts_ack(Trace<web::rest::Response> const &event, uint32_t sequence) {
  auto const STATE = State::CONTRACTS;
  profile_.contracts_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      download_.retry(STATE);
    };
    auto handle_success = [&](auto &body) {
      if (download_.skip(sequence, STATE)) {
        log::info("Download state={} has already been processed"sv, STATE);
      } else {
        protocol::json::ContractsAck contracts_ack{body, decode_buffer_};
        Trace event_2{event, contracts_ack};
        (*this)(event_2);
        download_.check(STATE);
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

void Rest::operator()(Trace<protocol::json::ContractsAck> const &event) {
  auto &[trace_info, contracts_ack] = event;
  log::info<4>("contracts_ack={}"sv, contracts_ack);
  std::vector<Symbol> symbols;
  symbols.reserve(std::size(contracts_ack.data));
  size_t counter = 0;
  for (auto &item : contracts_ack.data) {
    log::info<2>("item={}"sv, item);
    auto symbol = item.name;
    auto discard = shared_.dispatcher.discard_symbol(symbol);
    auto [base_currency, quote_currency] = [&]() -> std::pair<std::string_view, std::string_view> {
      auto sep = symbol.find('_');
      if (sep == std::string_view::npos) [[unlikely]] {
        log::fatal(R"(Unexpected: symbol="{}")"sv, symbol);
      }
      return {
          symbol.substr(0, sep),
          symbol.substr(sep + 1),
      };
    }();
    auto settlement_currency = [&]() -> std::string_view {
      switch (item.type) {
        using enum protocol::json::ContractType::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
        case INDEX:
          break;
        case DIRECT:
          return quote_currency;
        case INVERSE:
          return base_currency;
        case STOCKS:
          break;
      }
      return {};
    }();
    auto min_trade_vol = [&]() {
      if (utils::is_zero(item.order_size_min)) {
        return 1.0;
      }
      return item.order_size_min;
    }();
    auto reference_data = ReferenceData{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = symbol,
        .description = symbol,
        .security_type = SecurityType::SWAP,  // XXX always ???
        .external_security_id = {},
        .cfi_code = {},
        .base_currency = base_currency,
        .quote_currency = quote_currency,
        .settlement_currency = settlement_currency,
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = item.order_price_round,
        .tick_size_steps = {},
        .multiplier = NaN,
        .min_notional = NaN,
        .min_trade_vol = min_trade_vol,
        .max_trade_vol = item.order_size_max,
        .trade_vol_step_size = NaN,
        .option_type = {},
        .strike_currency = {},
        .strike_price = NaN,
        .underlying = {},
        .time_zone = {},
        .issue_date = utils::safe_cast{item.create_time},
        .settlement_date = {},
        .expiry_datetime = {},
        .expiry_datetime_utc = {},
        .exchange_time_utc = item.config_change_time,
        .exchange_sequence = {},
        .sending_time_utc = {},
        .discard = discard,
    };
    create_trace_and_dispatch(shared_.dispatcher, trace_info, reference_data, true);
    if (discard) {
      log::info<1>(R"(Drop symbol="{}")"sv, symbol);
      continue;
    }
    if (shared_.all_symbols.emplace(symbol).second) {  // only include new
      symbols.emplace_back(symbol);
    }
    ++counter;
  }
  if (!std::empty(symbols)) {
    auto contracts_update = SymbolsUpdate{
        .symbols = symbols,
    };
    handler_(contracts_update);
  }
  if (counter > 0) [[unlikely]] {
    log::info("Symbols {} / {}"sv, counter, std::size(contracts_ack.data));
  }
}

// order-book

void Rest::get_order_book(std::string_view const &symbol) {
  profile_.order_book([&]() {
    auto query = fmt::format("?contract={}&limit={}&with_id=true"sv, symbol, shared_.settings.misc.order_book_depth);
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.futures_order_book,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, symbol = std::string{symbol}]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_order_book_ack(event, symbol);
    };
    (*connection_)("futures-order-book"sv, request, callback);
  });
}

void Rest::get_order_book_ack(Trace<web::rest::Response> const &event, std::string_view const &symbol) {
  profile_.order_book_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      // XXX WHAT ???
    };
    auto handle_success = [&](auto &body) {
      protocol::json::OrderBookAck order_book_ack{body, decode_buffer_};
      Trace event_2{event, order_book_ack};
      (*this)(event_2, symbol);
    };
    process_response(event, handle_error, handle_success);
  });
}

void Rest::operator()(Trace<protocol::json::OrderBookAck> const &event, std::string_view const &symbol) {
  auto &[trace_info, order_book_ack] = event;
  log::info<3>("order_book_ack={}"sv, order_book_ack);
  auto sequence = order_book_ack.id;
  auto &sequencer = shared_.mbp_sequencer[symbol];
  auto &mbp = shared_.get_mbp();
  auto emplace_back = [](auto &result, auto &item) {
    auto mbp_update = MBPUpdate{
        .price = item.price,
        .quantity = item.size,
        .implied_quantity = NaN,
        .number_of_orders = {},
        .update_action = {},
        .price_level = {},
    };
    result.emplace_back(std::move(mbp_update));
  };
  for (auto &item : order_book_ack.bids) {
    emplace_back(mbp.bids, item);
  }
  for (auto &item : order_book_ack.asks) {
    emplace_back(mbp.asks, item);
  }
  try {
    auto publish_snapshot = [&](auto &bids, auto &asks, auto sequence, auto retries, auto delay) {
      log::info(
          R"(DEBUG PUBLISH SNAPSHOT symbol="{}", sequence={}, retries={}, delay={})"sv,
          symbol,
          sequence,
          retries,
          std::chrono::duration_cast<std::chrono::milliseconds>(delay));
      auto market_by_price_update = MarketByPriceUpdate{
          .stream_id = stream_id_,
          .exchange = shared_.settings.exchange,
          .symbol = symbol,
          .bids = bids,
          .asks = asks,
          .update_type = UpdateType::SNAPSHOT,
          .exchange_time_utc = order_book_ack.update,
          .exchange_sequence = sequencer.last_sequence(),
          .sending_time_utc = order_book_ack.current,
          .price_precision = {},
          .quantity_precision = {},
          .checksum = {},
      };
      auto apply_updates = [&](auto &market_by_price) { sequencer.apply(market_by_price, sequence, true); };
      create_trace_and_dispatch(shared_.dispatcher, trace_info, market_by_price_update, true, apply_updates);
    };
    auto request_snapshot = [&](auto retries) {
      log::info(R"(DEBUG REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
      if (shared_.settings.ws.mbp_request_max_retries && shared_.settings.ws.mbp_request_max_retries < retries) {
        log::fatal(R"(Unexpected: symbol="{}", retries={})"sv, symbol, retries);
      }
      shared_.depth_request_queue.emplace_back(symbol);
    };
    sequencer(mbp.bids, mbp.asks, sequence, false, publish_snapshot, request_snapshot);
  } catch (BadState &) {
    log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
    // XXX HANS publish stale
    sequencer.clear();
    shared_.depth_request_queue.emplace_back(symbol);
  }
}

// candlesticks

void Rest::get_candlesticks(std::string_view const &symbol) {
  profile_.candlesticks([&]() {
    auto query = fmt::format("?contract={}&interval=1m"sv, symbol);
    auto request = web::rest::Request{
        .method = web::http::Method::GET,
        .path = shared_.api.futures_candlesticks,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, symbol = std::string{symbol}]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_candlesticks_ack(event, symbol);
    };
    log::info("{}"sv, request);
    (*connection_)("futures-order-book"sv, request, callback);
  });
}

void Rest::get_candlesticks_ack(Trace<web::rest::Response> const &event, std::string_view const &symbol) {
  profile_.candlesticks_ack([&]() {
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      // XXX WHAT ???
    };
    auto handle_success = [&](auto &body) {
      protocol::json::CandlesticksAck candlesticks_ack{body, decode_buffer_};
      Trace event_2{event, candlesticks_ack};
      (*this)(event_2, symbol);
    };
    process_response(event, handle_error, handle_success);
  });
}

void Rest::operator()(Trace<protocol::json::CandlesticksAck> const &event, std::string_view const &symbol) {
  auto &[trace_info, candlesticks_ack] = event;
  log::info<3>("candlesticks_ack={}"sv, candlesticks_ack);
  auto &bars = shared_.bars;
  bars.clear();
  for (auto &item : candlesticks_ack.data) {
    auto bar = Bar{
        .begin_time_utc = item.time,
        .confirmed = true,
        .open_price = item.open,
        .high_price = item.open,
        .low_price = item.open,
        .close_price = item.open,
        .quantity = item.volume,
        .base_amount = roq::NaN,
        .quote_amount = item.sum,  // note! different field from WS
        .number_of_trades = {},
        .vwap = roq::NaN,
    };
    bars.emplace_back(std::move(bar));
  }
  auto time_series_update = TimeSeriesUpdate{
      .stream_id = stream_id_,
      .exchange = shared_.settings.exchange,
      .symbol = symbol,
      .data_source = DataSource::TRADE_SUMMARY,
      .interval = shared_.settings.time_series.interval,
      .origin = Origin::EXCHANGE,
      .bars = bars,
      .update_type = UpdateType::SNAPSHOT,
      .exchange_time_utc = {},  // XXX FIXME
  };
  create_trace_and_dispatch(shared_.dispatcher, trace_info, time_series_update, true);
  bars.clear();
}

// queue

void Rest::check_request_queue(std::chrono::nanoseconds now) {
  auto depth_helper = [&](auto &symbol) { get_order_book(symbol); };
  shared_.depth_request_queue.dispatch([&](auto now) { return shared_.rate_limiter.can_request(now); }, depth_helper, now);
  auto time_series_helper = [&](auto &symbol) { get_candlesticks(symbol); };
  shared_.time_series_request_queue.dispatch([&](auto now) { return shared_.rate_limiter.can_request(now); }, time_series_helper, now);
}

// helpers

void Rest::process_response(web::rest::Response const &response, auto error_handler, auto success_handler) {
  try {
    auto [status, category, body] = response.result();
    switch (category) {
      using enum web::http::Category;
      case UNKNOWN:
      case INFORMATIONAL_RESPONSE:
        response.expect(web::http::Status::OK);  // throws
        break;
      case SUCCESS:
        success_handler(body);
        break;
      case REDIRECTION:
        log::fatal("Unexpected: URL is being redirected"sv);
      case CLIENT_ERROR: {
        auto text = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, text);
        break;
      }
      case SERVER_ERROR: {
        auto message = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, message);
        break;
      }
    }
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::GATEWAY, e.request_status(), e.error(), e.what());
  } catch (std::exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, e.what());
  }
}

}  // namespace gateway
}  // namespace gate_futures
}  // namespace roq
