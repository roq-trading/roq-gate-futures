/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/gate_futures/json/trade_login.hpp"

#include "roq/gate_futures/json/trade_subscribe.hpp"

#include "roq/gate_futures/json/trade_balances.hpp"
#include "roq/gate_futures/json/trade_orders.hpp"
#include "roq/gate_futures/json/trade_positions.hpp"
#include "roq/gate_futures/json/trade_trades.hpp"

#include "roq/gate_futures/json/trade_order_amend.hpp"
#include "roq/gate_futures/json/trade_order_cancel.hpp"
#include "roq/gate_futures/json/trade_order_cancel_cp.hpp"
#include "roq/gate_futures/json/trade_order_place.hpp"

#include "roq/gate_futures/json/trade_order_list.hpp"

namespace roq {
namespace gate_futures {
namespace json {

struct TradeParser final {
  struct Handler {
    virtual void operator()(Trace<json::TradeLogin> const &) = 0;
    //
    virtual void operator()(Trace<json::TradeSubscribe> const &) = 0;
    //
    virtual void operator()(Trace<json::TradeBalances> const &) = 0;
    virtual void operator()(Trace<json::TradePositions> const &) = 0;
    virtual void operator()(Trace<json::TradeOrders> const &) = 0;
    virtual void operator()(Trace<json::TradeTrades> const &) = 0;
    //
    virtual void operator()(Trace<json::TradeOrderPlace> const &) = 0;
    virtual void operator()(Trace<json::TradeOrderAmend> const &) = 0;
    virtual void operator()(Trace<json::TradeOrderCancel> const &) = 0;
    virtual void operator()(Trace<json::TradeOrderCancelCP> const &) = 0;
    //
    virtual void operator()(Trace<json::TradeOrderList> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types = false);
};

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
