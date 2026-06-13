/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/gate_futures/protocol/json/book_ticker.hpp"
#include "roq/gate_futures/protocol/json/candlesticks.hpp"
#include "roq/gate_futures/protocol/json/order_book_update.hpp"
#include "roq/gate_futures/protocol/json/subscribe.hpp"
#include "roq/gate_futures/protocol/json/tickers.hpp"
#include "roq/gate_futures/protocol/json/trades.hpp"

#include "roq/gate_futures/protocol/json/futures_system.hpp"

namespace roq {
namespace gate_futures {
namespace protocol {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<Subscribe> const &) = 0;
    virtual void operator()(Trace<Tickers> const &) = 0;
    virtual void operator()(Trace<Trades> const &) = 0;
    virtual void operator()(Trace<BookTicker> const &) = 0;
    virtual void operator()(Trace<OrderBookUpdate> const &) = 0;
    virtual void operator()(Trace<Candlesticks> const &) = 0;
    //
    virtual void operator()(Trace<FuturesSystem> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace protocol
}  // namespace gate_futures
}  // namespace roq
