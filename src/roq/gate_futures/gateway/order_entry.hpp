/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/gate_futures/protocol/json/accounts.hpp"
#include "roq/gate_futures/protocol/json/positions.hpp"
#include "roq/gate_futures/protocol/json/user_trades.hpp"

#include "roq/gate_futures/gateway/account.hpp"
#include "roq/gate_futures/gateway/shared.hpp"

namespace roq {
namespace gate_futures {
namespace gateway {

struct OrderEntry final : public web::rest::Client::Handler {
  struct Handler {};

  OrderEntry(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  OrderEntry(OrderEntry const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

 protected:
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Client::Connected> const &) override;
  void operator()(Trace<web::rest::Client::Disconnected> const &) override;
  void operator()(Trace<web::rest::Client::Latency> const &) override;

  // helpers

  bool ready() const { return connection_status_ == ConnectionStatus::READY; }

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  enum class State {
    UNDEFINED = 0,
    ACCOUNTS,
    POSITIONS,
    TRADES,
    DONE,
  };

  uint32_t download(State);

  // accounts

  void get_accounts();
  void get_accounts_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<protocol::json::Accounts> const &);

  // positions

  void get_positions();
  void get_positions_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<protocol::json::Positions> const &);

  // trades

  void get_trades();
  void get_trades_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<protocol::json::UserTrades> const &);

  // helpers

  void process_response(web::rest::Response const &, auto error_handler, auto success_handler);

  void waf_limit_violation();

 private:
  [[maybe_unused]] Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // connection
  std::unique_ptr<web::rest::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile  //
        accounts,
        accounts_ack,              //
        positions, positions_ack,  //
        trades, trades_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // account
  Account &account_;
  // shared
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  core::Download<State> download_;
  bool download_trades_is_first_ = true;
};

}  // namespace gateway
}  // namespace gate_futures
}  // namespace roq
