/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/gate_futures/order_entry.hpp"

#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/gate_futures/json/map.hpp"
#include "roq/gate_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "om"sv;

auto const SUPPORTS = Mask{
    SupportType::TRADE,
    SupportType::POSITION,
    SupportType::FUNDS,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account);
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
      // implementatoin
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::rest::Client::create(handler, context, config);
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};

auto get_download_trades_lookback(auto const &settings, auto download_trades_is_first) {
  if (download_trades_is_first) {
    if (settings.download.trades_lookback_on_restart.count())
      return settings.download.trades_lookback_on_restart;
  }
  return settings.download.trades_lookback;
}

std::string_view get_client_order_id(auto &text) {
  if (!text.starts_with("t-"sv))
    return {};
  return text.substr(2);
}
}  // namespace

// === IMPLEMENTATION ===

OrderEntry::OrderEntry(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account.name)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_(shared.settings.misc.decode_buffer_size),
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .accounts = create_metrics(shared.settings, name_, "accounts"sv),
          .accounts_ack = create_metrics(shared.settings, name_, "accounts_ack"sv),
          .positions = create_metrics(shared.settings, name_, "positions"sv),
          .positions_ack = create_metrics(shared.settings, name_, "positions_ack"sv),
          .trades = create_metrics(shared.settings, name_, "trades"sv),
          .trades_ack = create_metrics(shared.settings, name_, "trades_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      account_{account}, shared_{shared}, download_{shared.settings.rest.request_timeout, [this](auto state) { return download(state); }} {
}

void OrderEntry::operator()(Event<Start> const &) {
  (*connection_).start();
}

void OrderEntry::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void OrderEntry::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
}

void OrderEntry::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.accounts, metrics::Type::PROFILE)
      .write(profile_.accounts_ack, metrics::Type::PROFILE)
      .write(profile_.positions, metrics::Type::PROFILE)
      .write(profile_.positions_ack, metrics::Type::PROFILE)
      .write(profile_.trades, metrics::Type::PROFILE)
      .write(profile_.trades_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

void OrderEntry::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void OrderEntry::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
}

void OrderEntry::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void OrderEntry::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = account_.name,
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::HTTP,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
        .interface = (*connection_).get_interface(),
        .authority = (*connection_).get_current_authority(),
        .path = (*connection_).get_current_path(),
        .proxy = (*connection_).get_proxy(),
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

uint32_t OrderEntry::download(OrderEntryState state) {
  switch (state) {
    using enum OrderEntryState;
    case UNDEFINED:
      assert(false);
      break;
    case ACCOUNTS:
      get_accounts();
      return 1;
    case POSITIONS:
      get_positions();
      return 1;
    case TRADES:
      get_trades();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

void OrderEntry::get_accounts() {
  profile_.accounts([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.accounts;
    auto headers = account_.create_headers(method, path, {}, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_accounts_ack(event, sequence);
    };
    (*connection_)("accounts"sv, request, callback);
  });
}

void OrderEntry::get_accounts_ack(Trace<web::rest::Response> const &event, [[maybe_unused]] uint32_t sequence) {
  auto constexpr const STATE = OrderEntryState::ACCOUNTS;
  profile_.accounts_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::Accounts accounts{body, decode_buffer_};
      Trace event_2{event, accounts};
      (*this)(event_2);
      download_.check_relaxed(STATE);
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      if (download_.downloading())
        download_.retry(STATE);
    };
    process_response(event, handle_success, handle_error);
  });
}

void OrderEntry::operator()(Trace<json::Accounts> const &event) {
  auto &[trace_info, accounts] = event;
  log::info<2>("accounts={}"sv, accounts);
}

void OrderEntry::get_positions() {
  profile_.positions([&]() {
    auto method = web::http::Method::GET;
    auto path = shared_.api.positions;
    auto headers = account_.create_headers(method, path, {}, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_positions_ack(event, sequence);
    };
    (*connection_)("positions"sv, request, callback);
  });
}

void OrderEntry::get_positions_ack(Trace<web::rest::Response> const &event, [[maybe_unused]] uint32_t sequence) {
  auto constexpr const STATE = OrderEntryState::POSITIONS;
  profile_.positions_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::Positions positions{body, decode_buffer_};
      Trace event_2{event, positions};
      (*this)(event_2);
      download_.check_relaxed(STATE);
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      if (download_.downloading())
        download_.retry(STATE);
    };
    process_response(event, handle_success, handle_error);
  });
}

void OrderEntry::operator()(Trace<json::Positions> const &event) {
  auto &[trace_info, positions] = event;
  log::info<2>("positions={}"sv, positions);
  for (auto &item : positions.data) {
    log::info<2>("item={}"sv, item);
    if (shared_.discard_symbol(item.contract))
      continue;
    auto exchange_time_utc = [&]() -> std::chrono::nanoseconds {
      assert(item.update_time.count() != 0);
      return item.update_time;
    }();
    auto margin_mode = [&]() -> MarginMode {
      return {};  // XXX TODO item.mode ???
    }();
    auto long_quantity = std::max<double>(0.0, item.size);
    auto short_quantity = std::max<double>(0.0, -item.size);
    auto position_update = PositionUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.contract,
        .margin_mode = margin_mode,
        .external_account = {},
        .long_quantity = long_quantity,
        .short_quantity = short_quantity,
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = exchange_time_utc,
        .sending_time_utc = {},
    };
    log::debug("position_update={}"sv, position_update);
    create_trace_and_dispatch(handler_, trace_info, position_update, true);
  }
}

void OrderEntry::get_trades() {
  profile_.trades([&]() {
    auto now = clock::get_realtime<std::chrono::milliseconds>();
    auto lookback = get_download_trades_lookback(shared_.settings, download_trades_is_first_);
    log::info<1>("Download trades: lookback={}"sv, lookback);
    auto from = std::chrono::duration_cast<std::chrono::seconds>(now - lookback);
    auto method = web::http::Method::GET;
    auto path = shared_.api.trades;
    auto query = fmt::format("?from={}"sv, from.count());
    auto headers = account_.create_headers(method, path, query, {});
    auto request = web::rest::Request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = {},
    };
    auto callback = [this, sequence = download_.sequence()]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      get_trades_ack(event, sequence);
    };
    (*connection_)("trades"sv, request, callback);
  });
}

void OrderEntry::get_trades_ack(Trace<web::rest::Response> const &event, [[maybe_unused]] uint32_t sequence) {
  auto constexpr const STATE = OrderEntryState::TRADES;
  profile_.trades_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::UserTrades trades{body, decode_buffer_};
      Trace event_2{event, trades};
      (*this)(event_2);
      download_trades_is_first_ = false;
      download_.check_relaxed(STATE);
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      if (download_.downloading())
        download_.retry(STATE);
    };
    process_response(event, handle_success, handle_error);
  });
}

void OrderEntry::operator()(Trace<json::UserTrades> const &event) {
  auto &[trace_info, trades] = event;
  log::info<2>("trades={}"sv, trades);
  for (auto &item : trades.data) {
    log::info<2>("item={}"sv, item);
    auto client_order_id = get_client_order_id(item.text);
    if (std::empty(client_order_id)) {
      log::warn("*** EXTERNAL ORDER ***"sv);
      continue;
    }
    auto exchange_time_utc = [&]() -> std::chrono::nanoseconds { return item.create_time; }();
    auto external_order_id = fmt::format("{}"sv, item.order_id);
    auto side = item.size < 0 ? Side::SELL : Side::BUY;
    auto quantity = static_cast<double>(std::abs(item.size));
    auto fill = Fill{
        .exchange_time_utc = exchange_time_utc,
        .external_trade_id = item.trade_id,
        .quantity = quantity,
        .price = item.price,
        .liquidity = json::Map{item.role},
        .quote_quantity = NaN,
        .commission_quantity = item.fee,  // ???
        .commission_currency = {},
    };
    auto trade_update = TradeUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .order_id = {},
        .exchange = shared_.settings.exchange,
        .symbol = item.contract,
        .side = side,
        .position_effect = {},
        .margin_mode = {},
        .create_time_utc = exchange_time_utc,
        .update_time_utc = exchange_time_utc,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = client_order_id,
        .fills = {&fill, 1},
        .routing_id = {},
        .update_type = UpdateType::SNAPSHOT,
        .sending_time_utc = {},
        .user = {},
        .strategy_id = {},
    };
    create_trace_and_dispatch(handler_, trace_info, trade_update, true, SOURCE_NONE, client_order_id);
  }
}

// helpers

template <typename SuccessHandler, typename ErrorHandler>
void OrderEntry::process_response(web::rest::Response const &response, SuccessHandler success_handler, ErrorHandler error_handler) {
  try {
    log::debug("response={}"sv, response);
    auto [status, category, body] = response.result();
    switch (category) {
      using enum web::http::Category;
      case SUCCESS:  // 2xx
        success_handler(body);
        break;
      case CLIENT_ERROR:  // 4xx
        switch (status) {
          using enum web::http::Status;
          case FORBIDDEN:           // 403
            waf_limit_violation();  // note! this is *very* serious
            [[fallthrough]];
          case I_AM_A_TEAPOT:        // 418
          case TOO_MANY_REQUESTS: {  // 429
            auto text = fmt::format("{}"sv, status);
            error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::REQUEST_RATE_LIMIT_REACHED, text);
            break;
          }
          case CONFLICT:  // 409
            assert(false);
            [[fallthrough]];
          default: {
            // json::Error error{body};
            // error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, json::guess_error(error.code), error.msg);
          }
        }
        break;
      case SERVER_ERROR: {  // 5xx
        auto text = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, text);
        break;
      }
      default:
        response.expect(web::http::Status::OK);  // throws
    }
  } catch (server::oms::Exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(e.origin, e.status, e.error, e.what());
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::GATEWAY, e.request_status(), e.error(), e.what());
  } catch (std::exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, e.what());
  }
}

template <typename... Args>
void OrderEntry::operator()(Trace<server::oms::Response> const &event, uint8_t user_id, uint64_t order_id, Args &&...args) {
  auto &[trace_info, response] = event;
  if (shared_.update_order(user_id, order_id, stream_id_, trace_info, response, std::forward<Args>(args)..., []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
  }
}

void OrderEntry::operator()(Trace<server::oms::OrderUpdate> const &event, std::string_view const &client_order_id) {
  auto &[trace_info, order_update] = event;
  if (shared_.update_order(client_order_id, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

void OrderEntry::waf_limit_violation() {
  /*
  if (shared_.settings.rest.terminate_on_403) {
    log::fatal("WAF limit violation"sv);
  } else {
    log::warn("WAF limit violation"sv);
    (*connection_).suspend(shared_.settings.rest.back_off_delay);
  }
  */
}

}  // namespace gate_futures
}  // namespace roq
