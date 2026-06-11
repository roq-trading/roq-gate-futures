/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/gate_futures/protocol/json/trade_login.hpp"

#include "roq/gate_futures/protocol/json/trade_subscribe.hpp"

#include "roq/gate_futures/protocol/json/trade_balances.hpp"
#include "roq/gate_futures/protocol/json/trade_orders.hpp"
#include "roq/gate_futures/protocol/json/trade_positions.hpp"
#include "roq/gate_futures/protocol/json/trade_trades.hpp"

#include "roq/gate_futures/protocol/json/trade_order_amend.hpp"
#include "roq/gate_futures/protocol/json/trade_order_cancel.hpp"
#include "roq/gate_futures/protocol/json/trade_order_cancel_cp.hpp"
#include "roq/gate_futures/protocol/json/trade_order_place.hpp"

#include "roq/gate_futures/protocol/json/trade_order_list.hpp"

namespace roq {
namespace gate_futures {
namespace protocol {
namespace json {

struct TradeParser final {
  struct Handler {
    virtual void operator()(Trace<protocol::json::TradeLogin> const &) = 0;
    //
    virtual void operator()(Trace<protocol::json::TradeSubscribe> const &) = 0;
    //
    virtual void operator()(Trace<protocol::json::TradeBalances> const &) = 0;
    virtual void operator()(Trace<protocol::json::TradePositions> const &) = 0;
    virtual void operator()(Trace<protocol::json::TradeOrders> const &) = 0;
    virtual void operator()(Trace<protocol::json::TradeTrades> const &) = 0;
    //
    virtual void operator()(Trace<protocol::json::TradeOrderPlace> const &) = 0;
    virtual void operator()(Trace<protocol::json::TradeOrderAmend> const &) = 0;
    virtual void operator()(Trace<protocol::json::TradeOrderCancel> const &) = 0;
    virtual void operator()(Trace<protocol::json::TradeOrderCancelCP> const &) = 0;
    //
    virtual void operator()(Trace<protocol::json::TradeOrderList> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types = false);
};

}  // namespace json
}  // namespace protocol
}  // namespace gate_futures
}  // namespace roq
