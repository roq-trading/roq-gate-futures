/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gateio_futures/json/parser.h"

#include "roq/logging.h"

#include "roq/gateio_futures/json/message.h"

using namespace std::literals;

namespace roq {
namespace gateio_futures {
namespace json {

bool Parser::dispatch(
    Handler &handler,
    std::string_view const &message,
    core::json::Buffer &buffer,
    server::TraceInfo const &trace_info) {
  auto message_ = core::json::Parser::create<Message>(message, buffer);
  switch (message_.event) {
    case Event::UNDEFINED:
      break;
    case Event::UNKNOWN:
      assert(false);
      break;
    case Event::SUBSCRIBE:
      return true;
    case Event::UPDATE:
      switch (message_.channel) {
        case Channel::UNDEFINED:
          break;
        case Channel::UNKNOWN:
          assert(false);
          break;
        case Channel::TICKERS:
          return true;
        case Channel::TRADES:
          return true;
        case Channel::BOOK_TICKER:
          return true;
        case Channel::ORDER_BOOK_UPDATE:
          return true;
      }
      break;
  }
  return false;
}

}  // namespace json
}  // namespace gateio_futures
}  // namespace roq
