/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/gate_futures/json/trades.h"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_trades, update) {
  auto message = R"({)"
                 R"("id":null,)"
                 R"("time":1641366055,)"
                 R"("channel":"futures.trades",)"
                 R"("event":"update",)"
                 R"("error":null,)"
                 R"("result":[{)"
                 R"("size":-913,)"
                 R"("id":638643,)"
                 R"("create_time":1641366055,)"
                 R"("create_time_ms":1641366055959,)"
                 R"("price":"98.8",)"
                 R"("contract":"XCH_USDT")"
                 R"(})"
                 R"(])"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Trades>(message, buffer_);
}
