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

namespace roq {
namespace gate_futures {
namespace protocol {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<protocol::json::Subscribe> const &) = 0;
    virtual void operator()(Trace<protocol::json::Tickers> const &) = 0;
    virtual void operator()(Trace<protocol::json::Trades> const &) = 0;
    virtual void operator()(Trace<protocol::json::BookTicker> const &) = 0;
    virtual void operator()(Trace<protocol::json::OrderBookUpdate> const &) = 0;
    virtual void operator()(Trace<protocol::json::Candlesticks> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace protocol
}  // namespace gate_futures
}  // namespace roq
