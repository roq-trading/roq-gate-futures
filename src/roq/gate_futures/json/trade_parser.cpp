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
  if (header.status) {  // login has "header")
    if (header.channel == Channel::LOGIN && header.event == Event::API && header.status == 200) {
      TradeLogin login{message, buffer};
      create_trace_and_dispatch(handler, trace_info, login);
      return true;
    } else {
      log::fatal(R"(Unexpected: message="{}")"sv, message);
    }
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
          case BALANCES:
          case POSITIONS:
          case ORDERS:
          case USERTRADES:
            if (message_2.result.status == Status::SUCCESS) {
              return true;
            } else {
              log::fatal(R"(Unexpected: message="{}")"sv, message);
            }
            break;
        }
        break;
      case UPDATE:
        log::fatal(R"(NOT IMPLEMENTED: message="{}")"sv, message);
        break;
      case API:
        log::fatal("Unexpected"sv);
    }
  }
  return false;
}

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
