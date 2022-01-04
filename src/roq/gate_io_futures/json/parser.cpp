/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gate_io/json/parser.h"

#include "roq/logging.h"

using namespace std::literals;

namespace roq {
namespace gate_io {
namespace json {

bool Parser::dispatch(
    Handler &handler,
    std::string_view const &message,
    core::json::Buffer &buffer,
    server::TraceInfo const &trace_info) {
  return false;
}

}  // namespace json
}  // namespace gate_io
}  // namespace roq
