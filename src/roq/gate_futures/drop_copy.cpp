/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gate_futures/drop_copy.hpp"

#include "roq/mask.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/core/json/buffer.hpp"

#include "roq/gate_futures/flags.hpp"

#include "roq/gate_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

namespace {
const auto NAME = "ex"sv;
const Mask SUPPORTS{
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::FUNDS,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context, const auto &uri, const auto &query) {
  core::URI uri_{uri};
  core::web::ClientSocket::Config config{
      .validate_certificate = server::Flags::tls_validate_certificate(),
      .uris = {&uri_, 1},
      .query = query,
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return core::web::ClientSocket{handler, context, config, []() { return std::string(); }};
}

/*
OrderStatus compute_order_status(
    json::OrderChangeType change_type, json::OrderStatus status, double remain_size) {
  switch (change_type) {
    using enum json::OrderChangeType::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case OPEN:
      break;
    case MATCH:
    case FILLED:
      return utils::is_zero(remain_size) ?  OrderStatus::COMPLETED : OrderStatus::WORKING;
    case CANCELED: return OrderStatus::CANCELED;
  }
  return json::map(status);
}
*/
}  // namespace

DropCopy::DropCopy(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared,
    const std::string_view &uri,
    const std::string_view &query,
    std::chrono::nanoseconds ping_frequency)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      connection_(create_connection(*this, context, uri, query)), ping_frequency_(ping_frequency),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
      },
      security_(security), shared_(shared),
      download_({}, [this](auto state) { return download(state); }) {
}

bool DropCopy::ready() const {
  return connection_.ready();
}

void DropCopy::operator()(const Event<Start> &) {
  connection_.start();
}

void DropCopy::operator()(const Event<Stop> &) {
  connection_.stop();
}

void DropCopy::operator()(const Event<Timer> &event) {
  connection_.refresh(event.value.now);
}

void DropCopy::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void DropCopy::operator()(const core::web::ClientSocket::Connected &) {
  assert(logon_timeout_.count() == 0);
  auto now = core::clock::GetSystem();
  logon_timeout_ = now + Flags::ws_request_timeout();
}

void DropCopy::operator()(const core::web::ClientSocket::Disconnected &) {
  ++counter_.disconnect;
  ready_ = false;
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
  welcome_ = false;
  logon_timeout_ = {};
  next_ping_ = {};
}

void DropCopy::operator()(const core::web::ClientSocket::Ready &) {
  // note! wait for welcome
}

void DropCopy::operator()(const core::web::ClientSocket::Close &) {
}

void DropCopy::operator()(const core::web::ClientSocket::Latency &latency) {
  auto trace_info = server::create_trace_info();
  const ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = security_.get_account(),
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(const core::web::ClientSocket::Text &text) {
  parse(text.payload);
}

void DropCopy::operator()(const core::web::ClientSocket::Binary &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    const StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
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
    case SUBSCRIBE:
      subscribe();
      return {};
    case DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

void DropCopy::subscribe() {
  subscribe("/account/balance"sv);
  subscribe("/spotMarket/tradeOrders"sv);
}

void DropCopy::subscribe(const std::string_view &topic) {
  auto now = core::clock::GetSystem();
  auto message = fmt::format(
      R"({{)"
      R"("id":"{}",)"
      R"("type":"subscribe",)"
      R"("privateChannel":true,)"
      R"("topic":"{}",)"
      R"("response":true)"
      R"(}})"sv,
      now.count(),
      topic);
  log::debug("message={}"sv, message);
  connection_.send_text(message);
}

void DropCopy::parse(const std::string_view &message) {
  profile_.parse([&]() {
    try {
      auto trace_info = server::create_trace_info();
      core::json::Buffer buffer(decode_buffer_);
      json::Parser::dispatch(*this, message, buffer, trace_info);
    } catch (...) {
      log::warn(R"(message="{}")"sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

void DropCopy::operator()(Trace<json::Subscribe const> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Tickers const> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::Trades const> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::BookTicker const> const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(Trace<json::OrderBookUpdate const> const &) {
  log::fatal("Unexpected"sv);
}

}  // namespace gate_futures
}  // namespace roq
