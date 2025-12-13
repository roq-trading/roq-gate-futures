/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/gate_futures/account.hpp"
#include "roq/gate_futures/drop_copy_state.hpp"
#include "roq/gate_futures/shared.hpp"

#include "roq/gate_futures/json/trade_parser.hpp"

namespace roq {
namespace gate_futures {

struct DropCopy final : public web::socket::Client::Handler, json::TradeParser::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<TradeUpdate> const &, bool is_last, uint8_t user_id, std::string_view const &request_id) = 0;
    virtual void operator()(Trace<PositionUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<FundsUpdate> const &, bool is_last) = 0;
  };

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  DropCopy(DropCopy const &) = delete;

  bool ready() const;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, std::string_view const &request_id);
  uint16_t operator()(Event<ModifyOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);
  uint16_t operator()(Event<CancelOrder> const &, server::oms::Order const &, std::string_view const &request_id, std::string_view const &previous_request_id);

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id);

 protected:
  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

  void operator()(Trace<json::TradeLogin> const &) override;

  void operator()(Trace<json::TradeSubscribe> const &) override;

  void operator()(Trace<json::TradeBalances> const &) override;
  void operator()(Trace<json::TradePositions> const &) override;
  void operator()(Trace<json::TradeOrders> const &) override;
  void operator()(Trace<json::TradeTrades> const &) override;

  void operator()(Trace<json::TradeOrderPlace> const &) override;
  void operator()(Trace<json::TradeOrderAmend> const &) override;
  void operator()(Trace<json::TradeOrderCancel> const &) override;
  void operator()(Trace<json::TradeOrderCancelCP> const &) override;

  void operator()(Trace<json::TradeOrderList> const &) override;

 private:
  void operator()(ConnectionStatus);

  uint32_t download(DropCopyState);

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

  template <typename... Args>
  void operator()(Trace<server::oms::Response> const &, std::string_view const &client_order_id, Args &&...);
  void operator()(Trace<server::oms::OrderUpdate> const &, std::string_view const &client_order_id);

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
  ConnectionStatus status_ = {};
  core::Download<DropCopyState> download_;
  std::chrono::nanoseconds logon_timeout_ = {};
  std::chrono::nanoseconds next_ping_ = {};
  // ...
  int64_t user_id_ = {};
  std::string encode_buffer_;
};

}  // namespace gate_futures
}  // namespace roq
