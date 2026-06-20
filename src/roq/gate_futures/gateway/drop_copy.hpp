/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/gate_futures/gateway/account.hpp"
#include "roq/gate_futures/gateway/shared.hpp"

#include "roq/gate_futures/protocol/json/trade_parser.hpp"

namespace roq {
namespace gate_futures {
namespace gateway {

struct DropCopy final : public web::socket::Client::Handler, protocol::json::TradeParser::Handler {
  struct Handler {};

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  DropCopy(DropCopy const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id);
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id);

 protected:
  // web::socket::Client::Handler

  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

  // protocol::json::TradeParser::Handler

  void operator()(Trace<protocol::json::TradeLogin> const &) override;

  void operator()(Trace<protocol::json::TradeSubscribe> const &) override;

  void operator()(Trace<protocol::json::TradeBalances> const &) override;
  void operator()(Trace<protocol::json::TradePositions> const &) override;
  void operator()(Trace<protocol::json::TradeOrders> const &) override;
  void operator()(Trace<protocol::json::TradeTrades> const &) override;

  void operator()(Trace<protocol::json::TradeOrderPlace> const &) override;
  void operator()(Trace<protocol::json::TradeOrderAmend> const &) override;
  void operator()(Trace<protocol::json::TradeOrderCancel> const &) override;
  void operator()(Trace<protocol::json::TradeOrderCancelCP> const &) override;

  void operator()(Trace<protocol::json::TradeOrderList> const &) override;

  void operator()(Trace<protocol::json::FuturesSystem> const &) override;

  // helpers

  bool ready() const;

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  enum class State {
    UNDEFINED = 0,
    LOGIN,
    SUBSCRIBE,
    ORDERS,
    DONE,
  };

  uint32_t download(State);

  void login();

  void subscribe();

  void subscribe_balances();
  void subscribe_positions();
  void subscribe_orders();
  void subscribe_trades();

  void subscribe(std::string_view const &channel, std::string_view const &event, std::string_view const &payload);

  void get_orders();

  void parse(std::string_view const &message);

  template <typename Callback, typename T>
  void create_order_update(Callback, T const &value, UpdateType);

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // web socket
  std::unique_ptr<web::socket::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // account
  Account &account_;
  Shared &shared_;
  // state
  uint32_t request_id_ = {};
  bool ready_ = false;
  ConnectionStatus connection_status_ = {};
  core::Download<State> download_;
  std::chrono::nanoseconds logon_timeout_ = {};
  std::chrono::nanoseconds next_ping_ = {};
  // ...
  int64_t user_id_ = {};
  std::string encode_buffer_;
};

}  // namespace gateway
}  // namespace gate_futures
}  // namespace roq
