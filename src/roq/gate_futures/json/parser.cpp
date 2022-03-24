/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gate_futures/json/parser.hpp"

#include "roq/logging.hpp"

#include "roq/gate_futures/json/message.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {
namespace json {

bool Parser::dispatch(
    Handler &handler,
    std::string_view const &message,
    core::json::Buffer &buffer,
    TraceInfo const &trace_info) {
  auto message_ = core::json::Parser::create<Message>(message, buffer);
  switch (message_.event) {
    case Event::UNDEFINED:
      break;
    case Event::UNKNOWN:
      assert(false);
      break;
    case Event::SUBSCRIBE: {
      auto subscribe = core::json::Parser::create<Subscribe>(message, buffer);
      create_trace_and_dispatch(handler, trace_info, subscribe);
      return true;
    }
    case Event::UPDATE:
      switch (message_.channel) {
        case Channel::UNDEFINED:
          break;
        case Channel::UNKNOWN:
          assert(false);
          break;
        case Channel::TICKERS: {
          auto tickers = core::json::Parser::create<Tickers>(message, buffer);
          create_trace_and_dispatch(handler, trace_info, tickers);
          return true;
        }
        case Channel::TRADES: {
          auto trades = core::json::Parser::create<Trades>(message, buffer);
          create_trace_and_dispatch(handler, trace_info, trades);
          return true;
        }
        case Channel::BOOK_TICKER: {
          auto book_ticker = core::json::Parser::create<BookTicker>(message, buffer);
          create_trace_and_dispatch(handler, trace_info, book_ticker);
          return true;
        }
        case Channel::ORDER_BOOK_UPDATE:
          auto order_book_update = core::json::Parser::create<OrderBookUpdate>(message, buffer);
          create_trace_and_dispatch(handler, trace_info, order_book_update);
          return true;
      }
      break;
  }
  return false;
}

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
