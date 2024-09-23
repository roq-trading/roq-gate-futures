/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/gate_futures/json/trade_login.hpp"

namespace roq {
namespace gate_futures {
namespace json {

struct TradeParser final {
  struct Handler {
    virtual void operator()(Trace<json::TradeLogin> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, std::span<std::byte> const &, TraceInfo const &);
};

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
