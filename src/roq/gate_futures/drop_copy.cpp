/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/gate_futures/drop_copy.hpp"

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/tools/exception.hpp"

#include "roq/core/json/buffer.hpp"

#include "roq/gate_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "ex"sv;

auto const SUPPORTS = Mask{
    SupportType::ORDER,
    SupportType::TRADE,
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
  /*
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
      R"("text":"t-{}",)"        // XXX
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
      Decimal{create_order.price, order.price_precision.precision}),
      request_id);
  // XXX stp_act
  log::debug(R"(message="{}")"sv, message);
  (*connection_).send_text(message);
  */
  return stream_id_;
}
/*
contract  string  true  Futures contract
size  int64 true  Order size. Specify positive number to make a bid, and negative number to ask
iceberg int64 true  Display size for iceberg order. 0 for non-iceberg. Note that you will have to pay the taker fee for the hidden size
price string  false Order price. 0 for market order with tif set as `ioc
close bool  false Set as true to close the position, with size set to 0
reduce_only bool  false Set as true to be reduce-only order
tif string  false Time in force
text  string  false User defined information. If not empty, must follow the rules below:
auto_size string  false Set side to close dual-mode position. close_long closes the long side; while close_short the short one. Note size also needs to be set
to 0 stp_act string  false Self-Trading Prevention Action
*/

uint16_t DropCopy::operator()(
    Event<ModifyOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id) {
  return stream_id_;
}

uint16_t DropCopy::operator()(
    Event<CancelOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id) {
  return stream_id_;
}

uint16_t DropCopy::operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) {
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

}  // namespace gate_futures
}  // namespace roq
