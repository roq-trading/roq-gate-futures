/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/gate_futures/json/trade_parser.hpp"

#include "roq/logging.hpp"

#include "roq/gate_futures/json/trade_message.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {
namespace json {

// XXX TODO protocol error:
//
// R"({)"
// R"("time":1727318820,)"
// R"("time_ms":1727318820205,)"
// R"("conn_id":"c7799af51014ea53",)"
// R"("trace_id":"4d07b2abbe99cc9656b50d9dc980cb60",)"
// R"("channel":"",)"
// R"("event":"",)"
// R"("error":{)"
// R"("code":1,)"
// R"("message":"request message need json scheme")"
// R"(},)"
// R"("result":{)"
// R"("status":"fail")"
// R"(})"
// R"(})"

// === HELPERS ===

namespace {
template <typename T>
void dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool TradeParser::dispatch(
    Handler &handler, std::string_view const &message, core::json::BufferStack &buffer_stack, TraceInfo const &trace_info, bool allow_unknown_event_types) {
  TradeMessage message_2{message, buffer_stack};
  log::debug("{}"sv, message_2);
  auto &error = message_2.error;
  if (error.code != 0) {
    log::fatal(R"(Unexpected: message="{}")"sv, message);
  }
  auto &header = message_2.header;
  if (header.status != 0) {  // api has "header"
    assert(header.event == Event::API);
    switch (header.channel) {
      using enum Channel::type_t;
      case UNDEFINED_INTERNAL:
        break;
      case UNKNOWN_INTERNAL:
        if (allow_unknown_event_types) {
          return false;
        }
        break;
      case TICKERS:
      case TRADES:
      case BOOK_TICKER:
      case ORDER_BOOK_UPDATE:
      case CANDLESTICKS:
        break;
      case LOGIN:
        dispatch_helper<TradeLogin>(handler, message, buffer_stack, trace_info);
        return true;
      case BALANCES:
      case POSITIONS:
      case ORDERS:
      case USERTRADES:
        break;
      case ORDER_PLACE:
        dispatch_helper<TradeOrderPlace>(handler, message, buffer_stack, trace_info);
        return true;
      case ORDER_AMEND:
        dispatch_helper<TradeOrderAmend>(handler, message, buffer_stack, trace_info);
        return true;
      case ORDER_CANCEL:
        dispatch_helper<TradeOrderCancel>(handler, message, buffer_stack, trace_info);
        return true;
      case ORDER_CANCEL_CP:
        dispatch_helper<TradeOrderCancelCP>(handler, message, buffer_stack, trace_info);
        return true;
      case ORDER_LIST:
        dispatch_helper<TradeOrderList>(handler, message, buffer_stack, trace_info);
        return true;
    }
  } else {  // subscribe or update do not have "header"
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
          case TRADES:
          case BOOK_TICKER:
          case ORDER_BOOK_UPDATE:
          case CANDLESTICKS:
          case LOGIN:
            break;
          case BALANCES:
          case POSITIONS:
          case ORDERS:
          case USERTRADES:
            dispatch_helper<TradeSubscribe>(handler, message, buffer_stack, trace_info);
            return true;
          case ORDER_PLACE:
          case ORDER_AMEND:
          case ORDER_CANCEL:
          case ORDER_CANCEL_CP:
          case ORDER_LIST:
            break;
        }
        break;
      case UPDATE:
        switch (message_2.channel) {
          using enum Channel::type_t;
          case UNDEFINED_INTERNAL:
            break;
          case UNKNOWN_INTERNAL:
            assert(false);
            break;
          case TICKERS:
          case TRADES:
          case BOOK_TICKER:
          case ORDER_BOOK_UPDATE:
          case CANDLESTICKS:
          case LOGIN:
            break;
          case BALANCES:
            dispatch_helper<TradeBalances>(handler, message, buffer_stack, trace_info);
            return true;
          case POSITIONS:
            dispatch_helper<TradePositions>(handler, message, buffer_stack, trace_info);
            return true;
          case ORDERS:
            dispatch_helper<TradeOrders>(handler, message, buffer_stack, trace_info);
            return true;
          case USERTRADES:
            dispatch_helper<TradeTrades>(handler, message, buffer_stack, trace_info);
            return true;
          case ORDER_PLACE:
          case ORDER_AMEND:
          case ORDER_CANCEL:
          case ORDER_CANCEL_CP:
          case ORDER_LIST:
            break;
        }
        break;
      case API:
        if (allow_unknown_event_types) {
          return false;
        }
        break;
    }
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
