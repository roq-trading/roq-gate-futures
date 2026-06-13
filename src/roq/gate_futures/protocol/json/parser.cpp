/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/gate_futures/protocol/json/parser.hpp"

#include "roq/logging.hpp"

#include "roq/gate_futures/protocol/json/message.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {
namespace protocol {
namespace json {

// === HELPERS ===

namespace {
template <typename T>
void dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool Parser::dispatch(
    Handler &handler, std::string_view const &message, core::json::BufferStack &buffer_stack, TraceInfo const &trace_info, bool allow_unknown_event_types) {
  Message message_2{message, buffer_stack};
  switch (message_2.event) {
    using enum Event::type_t;
    case UNDEFINED_INTERNAL:
      break;
    case UNKNOWN_INTERNAL:
      if (allow_unknown_event_types) {
        return false;
      }
      break;
    case SUBSCRIBE:
      dispatch_helper<Subscribe>(handler, message, buffer_stack, trace_info);
      return true;
    case UPDATE:
      switch (message_2.channel) {
        using enum Channel::type_t;
        case UNDEFINED_INTERNAL:
          break;
        case UNKNOWN_INTERNAL:
          if (allow_unknown_event_types) {
            return false;
          }
          break;
        case TICKERS:
          dispatch_helper<Tickers>(handler, message, buffer_stack, trace_info);
          return true;
        case TRADES:
          dispatch_helper<Trades>(handler, message, buffer_stack, trace_info);
          return true;
        case BOOK_TICKER:
          dispatch_helper<BookTicker>(handler, message, buffer_stack, trace_info);
          return true;
        case ORDER_BOOK_UPDATE:
          dispatch_helper<OrderBookUpdate>(handler, message, buffer_stack, trace_info);
          return true;
        case CANDLESTICKS:
          dispatch_helper<Candlesticks>(handler, message, buffer_stack, trace_info);
          return true;
        case LOGIN:
        case BALANCES:
        case POSITIONS:
        case ORDERS:
        case USERTRADES:
        case ORDER_PLACE:
        case ORDER_AMEND:
        case ORDER_CANCEL:
        case ORDER_CANCEL_CP:
        case ORDER_LIST:
          log::fatal("Unexpected"sv);
        case FUTURES_SYSTEM:
          dispatch_helper<FuturesSystem>(handler, message, buffer_stack, trace_info);
          return true;
      }
      break;
    case API:
      // note! don't parse
      return false;
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace protocol
}  // namespace gate_futures
}  // namespace roq
