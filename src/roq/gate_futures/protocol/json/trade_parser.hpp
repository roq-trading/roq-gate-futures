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

#include "roq/gate_futures/protocol/json/futures_system.hpp"

namespace roq {
namespace gate_futures {
namespace protocol {
namespace json {

struct TradeParser final {
  struct Handler {
    virtual void operator()(Trace<TradeLogin> const &) = 0;
    //
    virtual void operator()(Trace<TradeSubscribe> const &) = 0;
    //
    virtual void operator()(Trace<TradeBalances> const &) = 0;
    virtual void operator()(Trace<TradePositions> const &) = 0;
    virtual void operator()(Trace<TradeOrders> const &) = 0;
    virtual void operator()(Trace<TradeTrades> const &) = 0;
    //
    virtual void operator()(Trace<TradeOrderPlace> const &) = 0;
    virtual void operator()(Trace<TradeOrderAmend> const &) = 0;
    virtual void operator()(Trace<TradeOrderCancel> const &) = 0;
    virtual void operator()(Trace<TradeOrderCancelCP> const &) = 0;
    //
    virtual void operator()(Trace<TradeOrderList> const &) = 0;
    //
    virtual void operator()(Trace<FuturesSystem> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types = false);
};

}  // namespace json
}  // namespace protocol
}  // namespace gate_futures
}  // namespace roq
