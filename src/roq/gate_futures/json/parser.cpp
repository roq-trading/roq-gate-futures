/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/gate_futures/json/parser.hpp"

#include "roq/logging.hpp"

#include "roq/gate_futures/json/message.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {
namespace json {

bool Parser::dispatch(Handler &handler, std::string_view const &message, std::span<std::byte> const &buffer, TraceInfo const &trace_info) {
  Message message_{message, buffer};
  switch (message_.event) {
    using enum Event::type_t;
    case UNDEFINED__:
      break;
    case UNKNOWN__:
      assert(false);
      break;
    case SUBSCRIBE: {
      Subscribe subscribe{message, buffer};
      create_trace_and_dispatch(handler, trace_info, subscribe);
      return true;
    }
    case UPDATE:
      switch (message_.channel) {
        using enum Channel::type_t;
        case UNDEFINED__:
          break;
        case UNKNOWN__:
          assert(false);
          break;
        case TICKERS: {
          Tickers tickers{message, buffer};
          create_trace_and_dispatch(handler, trace_info, tickers);
          return true;
        }
        case TRADES: {
          Trades trades{message, buffer};
          create_trace_and_dispatch(handler, trace_info, trades);
          return true;
        }
        case BOOK_TICKER: {
          BookTicker book_ticker{message, buffer};
          create_trace_and_dispatch(handler, trace_info, book_ticker);
          return true;
        }
        case ORDER_BOOK_UPDATE: {
          OrderBookUpdate order_book_update{message, buffer};
          create_trace_and_dispatch(handler, trace_info, order_book_update);
          return true;
        }
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
      }
      break;
    case API:
      break;
  }
  return false;
}

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
