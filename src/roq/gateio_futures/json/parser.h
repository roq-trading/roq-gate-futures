/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/server.h"

namespace roq {
namespace gateio_futures {
namespace json {

struct Parser final {
  struct Handler {};

  static bool dispatch(
      Handler &handler,
      std::string_view const &message,
      core::json::Buffer &,
      server::TraceInfo const &);
};

}  // namespace json
}  // namespace gateio_futures
}  // namespace roq
