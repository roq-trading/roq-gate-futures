/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/gate_futures/json/book_ticker.hpp"
#include "roq/gate_futures/json/order_book_update.hpp"
#include "roq/gate_futures/json/subscribe.hpp"
#include "roq/gate_futures/json/tickers.hpp"
#include "roq/gate_futures/json/trades.hpp"

namespace roq {
namespace gate_futures {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(Trace<json::Subscribe> const &) = 0;
    virtual void operator()(Trace<json::Tickers> const &) = 0;
    virtual void operator()(Trace<json::Trades> const &) = 0;
    virtual void operator()(Trace<json::BookTicker> const &) = 0;
    virtual void operator()(Trace<json::OrderBookUpdate> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, std::span<std::byte> const &, TraceInfo const &);
};

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
