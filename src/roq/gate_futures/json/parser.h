/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/server.h"

#include "roq/gate_futures/json/book_ticker.h"
#include "roq/gate_futures/json/subscribe.h"
#include "roq/gate_futures/json/tickers.h"
#include "roq/gate_futures/json/trades.h"

namespace roq {
namespace gate_futures {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(server::Trace<json::Subscribe> const &) = 0;
    virtual void operator()(server::Trace<json::Tickers> const &) = 0;
    virtual void operator()(server::Trace<json::Trades> const &) = 0;
    virtual void operator()(server::Trace<json::BookTicker> const &) = 0;
  };

  static bool dispatch(
      Handler &handler,
      std::string_view const &message,
      core::json::Buffer &,
      server::TraceInfo const &);
};

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
