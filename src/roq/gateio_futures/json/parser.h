/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/server.h"

#include "roq/gateio_futures/json/book_ticker.h"
#include "roq/gateio_futures/json/subscribe.h"
#include "roq/gateio_futures/json/tickers.h"
#include "roq/gateio_futures/json/trades.h"

namespace roq {
namespace gateio_futures {
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
}  // namespace gateio_futures
}  // namespace roq
