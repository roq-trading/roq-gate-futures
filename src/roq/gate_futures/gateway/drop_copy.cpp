/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/gate_futures/gateway/drop_copy.hpp"

#include "roq/mask.hpp"

#include "roq/utils/common.hpp"
#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/core/json/buffer.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/gate_futures/json/encoder.hpp"
#include "roq/gate_futures/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {
namespace gateway {

// === CONSTANTS ===

namespace {
auto const NAME = "ex"sv;

auto const SUPPORTS = Mask{
    SupportType::CREATE_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::POSITION,
    SupportType::FUNDS,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.ws.uri;
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = {},
      .always_reconnect = true,
      // proxy
      .proxy = {},
      // http
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};

std::string_view get_client_order_id(auto &text) {
  if (!text.starts_with("t-"sv)) {
    return {};
  }
  return text.substr(2);
}

OrderStatus get_order_status(json::FinishAs finish_as, json::OrderStatus status) {
  auto result = map(finish_as).template get<OrderStatus>();
  if (result != OrderStatus{}) {
    return result;
  }
  if (status == json::OrderStatus::OPEN) {
    return OrderStatus::WORKING;
  }
  log::fatal("Unexpected: result={}"sv, result);
}
}  // namespace

// === IMPLEMENTATION ===

DropCopy::DropCopy(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared}, download_{{}, [this](auto state) { return download(state); }} {
}

bool DropCopy::ready() const {
  return (*connection_).ready();
}

void DropCopy::operator()(Event<Start> const &) {
  (*connection_).start();
}

void DropCopy::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void DropCopy::operator()(Event<Timer> const &event) {
  (*connection_).refresh(event.value.now);
}

void DropCopy::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

uint16_t DropCopy::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  auto message = json::Encoder::order_place(encode_buffer_, event, order, ref_data, request_id, ++request_id_);
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
  return stream_id_;
}

uint16_t DropCopy::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  auto message = json::Encoder::order_amend(encode_buffer_, event, order, ref_data, request_id, previous_request_id, ++request_id_);
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
  return stream_id_;
}

uint16_t DropCopy::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  auto message = json::Encoder::order_cancel(encode_buffer_, event, order, ref_data, request_id, previous_request_id, ++request_id_);
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
  return stream_id_;
}

uint16_t DropCopy::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  auto helper = [&](auto &symbol) {
    auto message = json::Encoder::order_cancel_cp(encode_buffer_, event, request_id, ++request_id_, symbol);
    log::warn(R"(DEBUG message="{}")"sv, message);
    (*connection_).send_text(message);
  };
  if (shared_.get_all_order_symbols(helper, account_.name)) {
  } else {
    log::warn("DEBUG *** NO ORDERS ***"sv);
  }
  return stream_id_;
}

void DropCopy::operator()(web::socket::Client::Connected const &) {
}

void DropCopy::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
  logon_timeout_ = {};
  next_ping_ = {};
}

void DropCopy::operator()(web::socket::Client::Ready const &) {
  download_.begin();
}

void DropCopy::operator()(web::socket::Client::Close const &) {
}

void DropCopy::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void DropCopy::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
  connection_status_ = connection_status;
  TraceInfo trace_info;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = account_.name,
      .supports = SUPPORTS,
      .transport = Transport::TCP,
      .protocol = Protocol::WS,
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
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

uint32_t DropCopy::download(State state) {
  switch (state) {
    using enum State;
    case UNDEFINED:
      assert(false);
      break;
    case LOGIN:
      (*this)(ConnectionStatus::LOGIN_SENT);
      login();
      return 1;
    case SUBSCRIBE:
      (*this)(ConnectionStatus::DOWNLOADING, "subscribe"sv);
      subscribe();
      return 0;
    case ORDERS:
      (*this)(ConnectionStatus::DOWNLOADING, "orders"sv);
      get_orders();
      return 1;
    case DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return 0;
  }
  assert(false);
  return 0;
}

void DropCopy::login() {
  auto request_id = ++request_id_;
  auto now = clock::get_realtime<std::chrono::seconds>();
  auto event = "api"sv;
  auto channel = "futures.login"sv;
  auto signature = account_.create_signature_login(event, channel, {}, now);
  auto message = fmt::format(
      R"({{)"
      R"("id":{},)"
      R"("time":{},)"
      R"("channel":"{}",)"
      R"("event":"{}",)"
      R"("payload":{{)"
      R"("req_id":"{}",)"
      R"("timestamp":"{}",)"
      R"("api_key":"{}",)"
      R"("signature":"{}")"
      R"(}})"
      R"(}})"sv,
      request_id,
      now.count(),
      channel,
      event,
      request_id,
      now.count(),
      account_.get_key(),
      signature);
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
}

void DropCopy::subscribe() {
  subscribe_balances();
  subscribe_positions();
  subscribe_orders();
  subscribe_trades();
}

void DropCopy::subscribe_balances() {
  assert(user_id_);
  auto payload = fmt::format(R"(["{}"])"sv, user_id_);
  subscribe("futures.balances"sv, "subscribe"sv, payload);
}

void DropCopy::subscribe_positions() {
  assert(user_id_);
  auto payload = fmt::format(R"(["{}","!all"])"sv, user_id_);
  subscribe("futures.positions"sv, "subscribe"sv, payload);
}

void DropCopy::subscribe_orders() {
  assert(user_id_);
  auto payload = fmt::format(R"(["{}","!all"])"sv, user_id_);
  subscribe("futures.orders"sv, "subscribe"sv, payload);
}

void DropCopy::subscribe_trades() {
  assert(user_id_);
  auto payload = fmt::format(R"(["{}","!all"])"sv, user_id_);
  subscribe("futures.usertrades"sv, "subscribe"sv, payload);
}

void DropCopy::subscribe(std::string_view const &channel, std::string_view const &event, std::string_view const &payload) {
  auto request_id = ++request_id_;
  auto now = clock::get_realtime<std::chrono::seconds>();
  std::string signature = account_.create_signature(channel, event, now);
  auto message = fmt::format(
      R"({{)"
      R"("id":{},)"
      R"("time":{},)"
      R"("channel":"{}",)"
      R"("event":"{}",)"
      R"("payload":{},)"
      R"("auth":{{)"  // <<== from here it's different from login
      R"("method":"api_key",)"
      R"("KEY":"{}",)"
      R"("SIGN":"{}")"
      R"(}})"
      R"(}})"sv,
      request_id,
      now.count(),
      channel,
      event,
      payload,
      account_.get_key(),
      signature);
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
}

void DropCopy::get_orders() {
  auto request_id = ++request_id_;
  auto now = clock::get_realtime<std::chrono::seconds>();
  auto const channel = "futures.order_list"sv;
  auto const event = "api"sv;
  auto message = fmt::format(
      R"({{)"
      R"("id":{},)"
      R"("time":{},)"
      R"("channel":"{}",)"
      R"("event":"{}",)"
      R"("payload":{{)"
      R"("req_id":"{}",)"
      R"("req_param":{{)"
      R"("status":"open")"
      R"(}})"
      R"(}})"
      R"(}})"sv,
      request_id,
      now.count(),
      channel,
      event,
      request_id);
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
}

void DropCopy::parse(std::string_view const &message) {
  log::debug(R"(message="{}")"sv, message);
  profile_.parse([&]() {
    auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!json::TradeParser::dispatch(*this, message, decode_buffer_, trace_info, shared_.settings.experimental.allow_unknown_event_types)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

void DropCopy::operator()(Trace<json::TradeLogin> const &event) {
  auto &[trace_info, login] = event;
  log::info<5>("login={}"sv, login);
  if (login.header.status != 200) {
    log::fatal("Unexpected: login={}"sv, login);
  }
  if (login.data.result.uid <= 0) {
    log::fatal("Unexpected: user_id must be positive (login={})"sv, login);
  }
  user_id_ = login.data.result.uid;
  auto const STATE = State::LOGIN;
  download_.check_relaxed(STATE);
}

void DropCopy::operator()(Trace<json::TradeSubscribe> const &event) {
  auto &subscribe = event.value;
  if (subscribe.result.status != json::Status::SUCCESS) {
    log::fatal("subscribe={}"sv, subscribe);
  }
}

void DropCopy::operator()(Trace<json::TradeBalances> const &event) {
  auto &[trace_info, balances] = event;
  for (auto &item : balances.result) {
    auto funds_update = FundsUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .currency = item.currency,
        .margin_mode = {},  // XXX ???
        .balance = item.balance,
        .hold = NaN,
        .borrowed = NaN,
        .unrealized_pnl = NaN,
        .external_account = {},
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = item.time_ms,
        .sending_time_utc = balances.time_ms,
    };
    create_trace_and_dispatch(handler_, trace_info, funds_update, true);
  }
}

void DropCopy::operator()(Trace<json::TradePositions> const &event) {
  auto &[trace_info, positions] = event;
  for (auto &item : positions.result) {
    if (shared_.discard_symbol(item.contract)) {
      continue;
    }
    auto exchange_time_utc = [&]() -> std::chrono::nanoseconds {
      assert(item.time_ms.count() != 0);
      return item.time_ms;
    }();
    auto margin_mode = [&]() -> MarginMode {
      return {};  // XXX TODO item.mode ???
    }();
    auto long_quantity = std::max<double>(0.0, utils::safe_cast(item.size));
    auto short_quantity = std::max<double>(0.0, utils::safe_cast(-item.size));
    auto position_update = PositionUpdate{
        .stream_id = stream_id_,
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.contract,
        .margin_mode = margin_mode,
        .external_account = {},
        .long_quantity = long_quantity,
        .short_quantity = short_quantity,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = exchange_time_utc,
        .sending_time_utc = positions.time_ms,
    };
    log::debug("position_update={}"sv, position_update);
    create_trace_and_dispatch(handler_, trace_info, position_update, true);
  }
}

void DropCopy::operator()(Trace<json::TradeOrders> const &event) {
  auto &[trace_info, orders] = event;
  auto helper = [&](auto &order_update) {
    Trace event_2{trace_info, order_update};
    (*this)(event_2, order_update.client_order_id);
  };
  for (auto &item : orders.result) {
    log::info<2>("item={}"sv, item);
    create_order_update(helper, item, UpdateType::INCREMENTAL);
  }
}

void DropCopy::operator()(Trace<json::TradeTrades> const &event) {
  auto &[trace_info, trades] = event;
  for (auto &item : trades.result) {
    log::info<2>("item={}"sv, item);
    auto client_order_id = get_client_order_id(item.text);
    if (std::empty(client_order_id)) {
      log::warn("*** EXTERNAL ORDER ***"sv);
      continue;
    }
    auto exchange_time_utc = [&]() -> std::chrono::nanoseconds {
      assert(item.create_time_ms.count() != 0);
      return item.create_time_ms;
    }();
    auto external_order_id = fmt::format("{}"sv, item.order_id);
    auto side = item.size < 0 ? Side::SELL : Side::BUY;
    auto quantity = static_cast<double>(std::abs(item.size));
    auto ref_data = shared_.get_ref_data(shared_.settings.exchange, item.contract);
    auto profit_loss_amount = utils::compute_profit_loss_amount(side, quantity, item.price, ref_data.multiplier);
    auto fill = Fill{
        .exchange_time_utc = exchange_time_utc,
        .external_trade_id = item.id,
        .quantity = quantity,
        .price = item.price,
        .liquidity = map(item.role),
        .commission_amount = item.fee,  // ???
        .commission_currency = {},
        .base_amount = NaN,
        .quote_amount = NaN,
        .profit_loss_amount = profit_loss_amount,
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
        .quantity_type = {},
        .create_time_utc = exchange_time_utc,
        .update_time_utc = exchange_time_utc,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = client_order_id,
        .fills = {&fill, 1},
        .routing_id = {},
        .update_type = UpdateType::INCREMENTAL,  // ???
        .sending_time_utc = trades.time_ms,
        .user = {},
        .strategy_id = {},
    };
    create_trace_and_dispatch(handler_, trace_info, trade_update, true, SOURCE_NONE, client_order_id);
  }
}

void DropCopy::operator()(Trace<json::TradeOrderPlace> const &event) {
  auto &[trace_info, order_place] = event;
  if (order_place.header.status == 200) {
    auto &result = order_place.data.result;
    // note! first ack doesn't contain the actual order (probably just validation)
    if (!std::empty(result.text)) {
      auto response = server::oms::Response{
          .request_type = RequestType::CREATE_ORDER,
          .origin = Origin::EXCHANGE,
          .request_status = RequestStatus::ACCEPTED,
          .error = {},
          .text = {},
          .version = {},
          .request_id = order_place.request_id,
          .external_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      auto helper = [&](auto &order_update) {
        Trace event_2{trace_info, response};
        (*this)(event_2, order_update.client_order_id, order_update);
      };
      create_order_update(helper, result, UpdateType::INCREMENTAL);
    }
  } else {
    auto response = server::oms::Response{
        .request_type = RequestType::CREATE_ORDER,
        .origin = Origin::EXCHANGE,
        .request_status = RequestStatus::REJECTED,
        .error = Error::UNKNOWN,  // XXX TODO parse error code
        .text = order_place.data.errs.message,
        .version = 1,
        .request_id = order_place.request_id,
        .external_order_id = {},
        .quantity = NaN,
        .price = NaN,
    };
    Trace event_2{trace_info, response};
    (*this)(event_2, order_place.request_id);
  }
}

void DropCopy::operator()(Trace<json::TradeOrderAmend> const &event) {
  auto &[trace_info, order_amend] = event;
  auto &header = order_amend.header;
  if (header.status == 200) {
    auto response = server::oms::Response{
        .request_type = RequestType::MODIFY_ORDER,
        .origin = Origin::EXCHANGE,
        .request_status = RequestStatus::ACCEPTED,
        .error = {},
        .text = {},
        .version = {},
        .request_id = order_amend.request_id,
        .external_order_id = {},
        .quantity = NaN,
        .price = NaN,
    };
    auto helper = [&](auto &order_update) {
      Trace event_2{trace_info, response};
      (*this)(event_2, order_update.client_order_id, order_update);
    };
    create_order_update(helper, order_amend.data.result, UpdateType::INCREMENTAL);
  } else {
    auto response = server::oms::Response{
        .request_type = RequestType::CANCEL_ORDER,
        .origin = Origin::EXCHANGE,
        .request_status = RequestStatus::REJECTED,
        .error = Error::UNKNOWN,  // XXX TODO parse error code
        .text = order_amend.data.errs.message,
        .version = {},
        .request_id = order_amend.request_id,
        .external_order_id = {},
        .quantity = NaN,
        .price = NaN,
    };
    Trace event_2{trace_info, response};
    (*this)(event_2, order_amend.request_id);
  }
}

void DropCopy::operator()(Trace<json::TradeOrderCancel> const &event) {
  auto &[trace_info, order_cancel] = event;
  auto &header = order_cancel.header;
  if (header.status == 200) {
    auto response = server::oms::Response{
        .request_type = RequestType::CANCEL_ORDER,
        .origin = Origin::EXCHANGE,
        .request_status = RequestStatus::ACCEPTED,
        .error = {},
        .text = {},
        .version = {},
        .request_id = order_cancel.request_id,
        .external_order_id = {},
        .quantity = NaN,
        .price = NaN,
    };
    auto helper = [&](auto &order_update) {
      Trace event_2{trace_info, response};
      (*this)(event_2, order_update.client_order_id, order_update);
    };
    create_order_update(helper, order_cancel.data.result, UpdateType::INCREMENTAL);
  } else {
    auto response = server::oms::Response{
        .request_type = RequestType::CANCEL_ORDER,
        .origin = Origin::EXCHANGE,
        .request_status = RequestStatus::REJECTED,
        .error = Error::UNKNOWN,  // XXX TODO parse error code
        .text = order_cancel.data.errs.message,
        .version = {},
        .request_id = order_cancel.request_id,
        .external_order_id = {},
        .quantity = NaN,
        .price = NaN,
    };
    Trace event_2{trace_info, response};
    (*this)(event_2, order_cancel.request_id);
  }
}

void DropCopy::operator()(Trace<json::TradeOrderCancelCP> const &event) {
  auto &[trace_info, order_cancel_cp] = event;
  auto &header = order_cancel_cp.header;
  if (header.status == 200) {
    auto cancel_all_orders_ack = CancelAllOrdersAck{
        .stream_id = stream_id_,
        .account = account_.name,
        .order_id = {},
        .exchange = {},
        .symbol = {},
        .side = {},
        .origin = Origin::EXCHANGE,
        .request_status = RequestStatus::ACCEPTED,
        .error = {},
        .text = {},
        .request_id = order_cancel_cp.request_id,
        .external_account = {},
        .number_of_affected_orders = {},
        .round_trip_latency = {},
        .user = {},
        .strategy_id = {},
    };
    Trace event_2{trace_info, cancel_all_orders_ack};
    shared_(event_2);
  } else {
    auto cancel_all_orders_ack = CancelAllOrdersAck{
        .stream_id = stream_id_,
        .account = account_.name,
        .order_id = {},
        .exchange = {},
        .symbol = {},
        .side = {},
        .origin = Origin::EXCHANGE,
        .request_status = RequestStatus::REJECTED,
        .error = Error::UNKNOWN,  // XXX TODO parse error code
        .text = order_cancel_cp.data.errs.message,
        .request_id = order_cancel_cp.request_id,
        .external_account = {},
        .number_of_affected_orders = {},
        .round_trip_latency = {},
        .user = {},
        .strategy_id = {},
    };
    Trace event_2{trace_info, cancel_all_orders_ack};
    shared_(event_2);
  }
}

void DropCopy::operator()(Trace<json::TradeOrderList> const &event) {
  auto &[trace_info, order_list] = event;
  auto helper = [&](auto &order_update) {
    Trace event_2{trace_info, order_update};
    (*this)(event_2, order_update.client_order_id);
  };
  log::info<2>("order_list={}"sv, order_list);
  for (auto &item : order_list.data.result) {
    log::info<2>("item={}"sv, item);
    create_order_update(helper, item, UpdateType::SNAPSHOT);
  }
  auto const STATE = State::ORDERS;
  download_.check_relaxed(STATE);
}

template <typename Callback, typename T>
void DropCopy::create_order_update(Callback callback, T const &value, UpdateType update_type) {
  auto client_order_id = get_client_order_id(value.text);
  if (std::empty(client_order_id)) {
    log::warn("*** EXTERNAL ORDER ***"sv);
    return;
  }
  auto external_order_id = fmt::format("{}"sv, value.id);
  auto side = value.size < 0 ? Side::SELL : Side::BUY;
  auto order_type = [&]() -> OrderType {
    if (update_type == UpdateType::SNAPSHOT) {
      return OrderType::LIMIT;  // download orders must be limit
    }
    return {};
  }();
  auto create_time_utc = [&]() -> std::chrono::nanoseconds {
    constexpr bool has_create_time_ms = requires(T const &type) { type.create_time_ms; };
    if constexpr (has_create_time_ms) {
      if (value.create_time_ms.count()) {
        return value.create_time_ms;
      }
    }
    return value.create_time;
  }();
  auto update_time_utc = [&]() -> std::chrono::nanoseconds {
    constexpr bool has_update_time_ms = requires(T const &type) { type.update_time_ms; };
    if constexpr (has_update_time_ms) {
      if (value.update_time_ms.count()) {
        return value.update_time_ms;
      }
    }
    return value.update_time;
  }();
  auto order_status = get_order_status(value.finish_as, value.status);
  auto quantity = static_cast<double>(std::abs(value.size));
  auto remaining_quantity = static_cast<double>(std::abs(value.left));
  auto traded_quantity = quantity - remaining_quantity;
  auto average_traded_price = [&]() -> double {
    if (utils::compare(traded_quantity, 0.0) > 0) {
      return value.fill_price;
    }
    return NaN;
  }();
  auto order_update = server::oms::OrderUpdate{
      .account = account_.name,
      .exchange = shared_.settings.exchange,
      .symbol = value.contract,
      .side = side,
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = order_type,
      .time_in_force = map(value.tif),
      .execution_instructions = {},
      .create_time_utc = create_time_utc,
      .update_time_utc = update_time_utc,
      .external_account = {},
      .external_order_id = external_order_id,
      .client_order_id = client_order_id,
      .order_status = order_status,
      .error = {},
      .text = {},
      .quantity = quantity,
      .price = value.price,
      .stop_price = NaN,
      .leverage = NaN,
      .remaining_quantity = remaining_quantity,
      .traded_quantity = traded_quantity,
      .average_traded_price = average_traded_price,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = update_type,
      .sending_time_utc = value.update_time,
  };
  callback(order_update);
}

template <typename... Args>
void DropCopy::operator()(Trace<server::oms::Response> const &event, std::string_view const &client_order_id, Args &&...args) {
  auto &[trace_info, response] = event;
  if (shared_.update_order(client_order_id, stream_id_, trace_info, response, std::forward<Args>(args)..., []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn(R"(Did not find order: client_order_id="{}")"sv, client_order_id);
  }
}

void DropCopy::operator()(Trace<server::oms::OrderUpdate> const &event, std::string_view const &client_order_id) {
  auto &[trace_info, order_update] = event;
  if (shared_.update_order(client_order_id, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

}  // namespace gateway
}  // namespace gate_futures
}  // namespace roq
