/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/buffer.hpp"
#include "roq/core/json/parser.hpp"

#include "roq/server.hpp"

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
    virtual void operator()(Trace<json::Subscribe const> const &) = 0;
    virtual void operator()(Trace<json::Tickers const> const &) = 0;
    virtual void operator()(Trace<json::Trades const> const &) = 0;
    virtual void operator()(Trace<json::BookTicker const> const &) = 0;
    virtual void operator()(Trace<json::OrderBookUpdate const> const &) = 0;
  };

  static bool dispatch(
      Handler &, std::string_view const &message, core::json::Buffer &, TraceInfo const &);
};

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
