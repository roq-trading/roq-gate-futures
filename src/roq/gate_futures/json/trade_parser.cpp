/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/gate_futures/json/trade_parser.hpp"

#include "roq/logging.hpp"

#include "roq/gate_futures/json/trade_message.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {
namespace json {

// === IMPLEMENTATION ===

bool TradeParser::dispatch(Handler &handler, std::string_view const &message, std::span<std::byte> const &buffer, TraceInfo const &trace_info) {
  TradeMessage message_2{message, buffer};
  log::debug("{}"sv, message_2);
  auto &error = message_2.error;
  if (error.code) {
    log::fatal(R"(Unexpected: message="{}")"sv, message);
  }
  auto &header = message_2.header;
  if (header.status) {  // api has "header"
    assert(header.event == Event::API);
    switch (header.channel) {
      using enum Channel::type_t;
      case UNDEFINED__:
        break;
      case UNKNOWN__:
        assert(false);
        break;
      case TICKERS:
      case TRADES:
      case BOOK_TICKER:
      case ORDER_BOOK_UPDATE:
        break;
      case LOGIN:
        if (header.status == 200) {
          TradeLogin login{message, buffer};
          create_trace_and_dispatch(handler, trace_info, login);
          return true;
        }
        break;
      case BALANCES:
      case POSITIONS:
      case ORDERS:
      case USERTRADES:
        log::fatal("Unexpected"sv);
        break;
      case ORDER_PLACE: {
        TradeOrderPlace order_place{message, buffer};
        create_trace_and_dispatch(handler, trace_info, order_place);
        return true;
      }
      case ORDER_AMEND: {
        TradeOrderAmend order_amend{message, buffer};
        create_trace_and_dispatch(handler, trace_info, order_amend);
        return true;
      }
      case ORDER_CANCEL: {
        TradeOrderCancel order_cancel{message, buffer};
        create_trace_and_dispatch(handler, trace_info, order_cancel);
        return true;
      }
      case ORDER_CANCEL_CP: {
        TradeOrderCancelCP order_cancel_cp{message, buffer};
        create_trace_and_dispatch(handler, trace_info, order_cancel_cp);
        return true;
      }
    }
    log::fatal("Unexpected"sv);
  } else {  // subscribe or update do not have "header"
    switch (message_2.event) {
      using enum Event::type_t;
      case UNDEFINED__:
        break;
      case UNKNOWN__:
        assert(false);
        break;
      case SUBSCRIBE:
        switch (message_2.channel) {
          using enum Channel::type_t;
          case UNDEFINED__:
            break;
          case UNKNOWN__:
            assert(false);
            break;
          case TICKERS:
          case TRADES:
          case BOOK_TICKER:
          case ORDER_BOOK_UPDATE:
          case LOGIN:
            log::fatal("Unexpected"sv);
            break;
          case BALANCES:
          case POSITIONS:
          case ORDERS:
          case USERTRADES: {
            TradeSubscribe subscribe{message, buffer};
            create_trace_and_dispatch(handler, trace_info, subscribe);
            return true;
          }
          case ORDER_PLACE:
          case ORDER_AMEND:
          case ORDER_CANCEL:
          case ORDER_CANCEL_CP:
            log::fatal("Unexpected"sv);
            break;
        }
        break;
      case UPDATE:
        switch (message_2.channel) {
          using enum Channel::type_t;
          case UNDEFINED__:
            break;
          case UNKNOWN__:
            assert(false);
            break;
          case TICKERS:
          case TRADES:
          case BOOK_TICKER:
          case ORDER_BOOK_UPDATE:
          case LOGIN:
            log::fatal("Unexpected"sv);
            break;
          case BALANCES: {
            TradeBalances balances{message, buffer};
            create_trace_and_dispatch(handler, trace_info, balances);
            return true;
          }
          case POSITIONS: {
            TradePositions positions{message, buffer};
            create_trace_and_dispatch(handler, trace_info, positions);
            return true;
          }
          case ORDERS: {
            TradeOrders orders{message, buffer};
            create_trace_and_dispatch(handler, trace_info, orders);
            return true;
          }
          case USERTRADES: {
            TradeTrades trades{message, buffer};
            create_trace_and_dispatch(handler, trace_info, trades);
            return true;
          }
          case ORDER_PLACE:
          case ORDER_AMEND:
          case ORDER_CANCEL:
          case ORDER_CANCEL_CP:
            log::fatal("Unexpected"sv);
            break;
        }
        break;
      case API:
        log::fatal("Unexpected"sv);
        break;
    }
  }
  return false;
}

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
