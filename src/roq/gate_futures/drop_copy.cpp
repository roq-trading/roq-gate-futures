/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/gate_futures/drop_copy.hpp"

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/tools/exception.hpp"

#include "roq/core/json/buffer.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/gate_futures/json/map.hpp"
#include "roq/gate_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

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
      .query = {},
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
}  // namespace

// === IMPLEMENTATION ===

DropCopy::DropCopy(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_(shared.settings.misc.decode_buffer_size),
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

void DropCopy::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

uint16_t DropCopy::operator()(Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id) {
  auto &create_order = event.value;
  auto request_id_2 = ++request_id_;
  auto now = clock::get_realtime<std::chrono::seconds>();
  auto const channel = "futures.order_place"sv;
  auto const event_2 = "api"sv;
  auto message = fmt::format(
      R"({{)"
      R"("id":{},)"
      R"("time":{},)"
      R"("channel":"{}",)"
      R"("event":"{}",)"
      R"("payload":{{)"
      R"("req_id":"{}",)"
      R"("req_param":{{)"
      R"("contract":"{}",)"
      R"("size":{},)"
      R"("iceberg":0,)"
      R"("price":"{}",)"
      R"("close":false,)"
      R"("reduce_only":false,)"  // XXX
      R"("tif":"GTC",)"          // XXX
      R"("text":"t-{}")"         // XXX
      R"(}})"
      R"(}})"
      R"(}})"sv,
      request_id_2,
      now.count(),
      channel,
      event_2,
      request_id_2,
      order.symbol,
      Decimal{create_order.quantity, order.quantity_precision.precision},
      Decimal{create_order.price, order.price_precision.precision},
      request_id);
  // XXX stp_act ?
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
  return stream_id_;
}

uint16_t DropCopy::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  auto &modify_order = event.value;
  auto request_id_2 = ++request_id_;
  auto now = clock::get_realtime<std::chrono::seconds>();
  auto const channel = "futures.order_amend"sv;
  auto const event_2 = "api"sv;
  auto message = fmt::format(
      R"({{)"
      R"("id":{},)"
      R"("time":{},)"
      R"("channel":"{}",)"
      R"("event":"{}",)"
      R"("payload":{{)"
      R"("req_id":"{}",)"
      R"("req_param":{{)"
      R"("order_id":"t-{}",)"  // XXX
      R"("size":{},)"
      R"("price":"{}",)"
      R"("amend_text":"{}")"
      R"(}})"
      R"(}})"
      R"(}})"sv,
      request_id_2,
      now.count(),
      channel,
      event_2,
      request_id,
      order.client_order_id,
      Decimal{modify_order.quantity, order.quantity_precision.precision},
      Decimal{modify_order.price, order.price_precision.precision},
      request_id);
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
  return stream_id_;
}

uint16_t DropCopy::operator()(
    Event<CancelOrder> const &,
    server::oms::Order const &order,
    std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  auto request_id_2 = ++request_id_;
  auto now = clock::get_realtime<std::chrono::seconds>();
  auto const channel = "futures.order_cancel"sv;
  auto const event_2 = "api"sv;
  auto message = fmt::format(
      R"({{)"
      R"("id":{},)"
      R"("time":{},)"
      R"("channel":"{}",)"
      R"("event":"{}",)"
      R"("payload":{{)"
      R"("req_id":"{}",)"
      R"("req_param":{{)"
      R"("order_id":"t-{}")"  // XXX
      R"(}})"
      R"(}})"
      R"(}})"sv,
      request_id_2,
      now.count(),
      channel,
      event_2,
      request_id,
      order.client_order_id);
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
  return stream_id_;
}

uint16_t DropCopy::operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) {
  auto request_id_2 = ++request_id_;
  auto now = clock::get_realtime<std::chrono::seconds>();
  auto const channel = "futures.order_cancel_cp"sv;
  auto const event_2 = "api"sv;
  auto message = fmt::format(
      R"({{)"
      R"("id":{},)"
      R"("time":{},)"
      R"("channel":"{}",)"
      R"("event":"{}",)"
      R"("payload":{{)"
      R"("req_id":"{}",)"
      R"("req_param":{{)"
      R"("contract":"SOL_USDT")"  // XXX
      R"(}})"
      R"(}})"
      R"(}})"sv,
      request_id_2,
      now.count(),
      channel,
      event_2,
      request_id);
  // XXX side ?
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
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
  // (*this)(ConnectionStatus::CONNECTED);
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

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = account_.name,
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
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

uint32_t DropCopy::download(DropCopyState state) {
  switch (state) {
    using enum DropCopyState;
    case UNDEFINED:
      assert(false);
      break;
    case LOGIN:
      login();
      return 1;
    case SUBSCRIBE:
      subscribe();
      return 0;
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
  auto const channel = "futures.login"sv;
  auto const event = ""sv;
  std::string signature = account_.create_signature(channel, event, now);
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
      std::empty(event) ? "api"sv : event,
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
      R"("method":"api",)"
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

void DropCopy::parse(std::string_view const &message) {
  log::debug(R"(message="{}")"sv, message);
  profile_.parse([&]() {
    auto log_message = [&]() { log::warn(R"(message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!json::TradeParser::dispatch(*this, message, decode_buffer_, trace_info)) {
        log_message();
        log::fatal("HERE"sv);
      }
    } catch (...) {
      log_message();
      core::tools::UnhandledException::terminate();
    }
  });
}

void DropCopy::operator()(Trace<json::TradeLogin> const &event) {
  auto &[trace_info, login] = event;
  log::info<5>("login={}"sv, login);
  if (login.data.result.uid <= 0)
    log::fatal("Unexpected: user_id must be positive (login={})"sv, login);
  user_id_ = login.data.result.uid;
  auto const STATE = DropCopyState::LOGIN;
  download_.check_relaxed(STATE);
}

void DropCopy::operator()(Trace<json::TradeSubscribe> const &event) {
  auto &subscribe = event.value;
  if (subscribe.result.status != json::Status::SUCCESS)
    log::fatal("subscribe={}"sv, subscribe);
}

void DropCopy::operator()(Trace<json::TradeBalances> const &) {
}

void DropCopy::operator()(Trace<json::TradePositions> const &) {
}

void DropCopy::operator()(Trace<json::TradeOrders> const &event) {
  auto &[trace_info, orders] = event;
  for (auto &item : orders.result) {
    log::info<2>("item={}"sv, item);
    auto cl_ord_id = [&]() -> std::string_view {
      if (!item.text.starts_with("t-"sv))
        return {};
      return item.text.substr(2);
    }();
    if (std::empty(cl_ord_id)) {
      log::warn("*** EXTERNAL ORDER ***"sv);
      continue;
    }
    auto external_order_id = fmt::format("{}"sv, item.id);
    auto side = item.size < 0 ? Side::SELL : Side::BUY;
    auto quantity = static_cast<double>(std::abs(item.size));
    auto remaining_quantity = static_cast<double>(std::abs(item.left));
    auto traded_quantity = quantity - remaining_quantity;  // XXX ???
    auto order_update = server::oms::OrderUpdate{
        .account = account_.name,
        .exchange = shared_.settings.exchange,
        .symbol = item.contract,
        .side = side,
        .position_effect = {},
        .margin_mode = {},
        .max_show_quantity = NaN,
        .order_type = OrderType::LIMIT,
        .time_in_force = json::Map{item.tif},
        .execution_instructions = {},
        .create_time_utc = item.create_time,
        .update_time_utc = item.update_time,
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = {},
        .order_status = json::Map{item.status},
        .quantity = quantity,
        .price = item.price,
        .stop_price = NaN,
        .remaining_quantity = remaining_quantity,
        .traded_quantity = traded_quantity,
        .average_traded_price = item.fill_price,  // ???
        .last_traded_quantity = NaN,
        .last_traded_price = NaN,
        .last_liquidity = {},
        .routing_id = {},
        .max_request_version = {},
        .max_response_version = {},
        .max_accepted_version = {},
        .update_type = UpdateType::INCREMENTAL,
        .sending_time_utc = item.update_time,
    };
    if (shared_.update_order(cl_ord_id, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {
          // no fills here
        })) {
    } else {
      log::warn<1>(R"(*** EXTERNAL ORDER *** (id={}, cl_ord_id="{}"))"sv, item.id, cl_ord_id);
    }
  }
}

void DropCopy::operator()(Trace<json::TradeTrades> const &event) {
  auto &[trace_info, trades] = event;
  for (auto &item : trades.result) {
    log::info<2>("item={}"sv, item);
    auto cl_ord_id = [&]() -> std::string_view {
      if (!item.text.starts_with("t-"sv))
        return {};
      return item.text.substr(2);
    }();
    if (std::empty(cl_ord_id)) {
      log::warn("*** EXTERNAL ORDER ***"sv);
      continue;
    }
    auto external_order_id = fmt::format("{}"sv, item.id);
    auto side = item.size < 0 ? Side::SELL : Side::BUY;
    auto quantity = static_cast<double>(std::abs(item.size));
    auto fill = Fill{
        .exchange_time_utc = item.create_time_ms,
        .external_trade_id = item.id,  // note!
        .quantity = quantity,
        .price = item.price,
        .liquidity = json::Map{item.role},
        .quote_quantity = NaN,
        .commission_quantity = item.fee,  // XXX ???
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
        .create_time_utc = utils::safe_cast(item.create_time_ms),
        .update_time_utc = utils::safe_cast(item.create_time_ms),
        .external_account = {},
        .external_order_id = external_order_id,
        .client_order_id = {},
        .fills = {&fill, 1},
        .routing_id = {},
        .update_type = UpdateType::SNAPSHOT,
        .sending_time_utc = item.create_time_ms,
        .user = {},
        .strategy_id = {},
    };
    create_trace_and_dispatch(handler_, trace_info, trade_update, true, SOURCE_NONE, cl_ord_id);
  }
}

}  // namespace gate_futures
}  // namespace roq
