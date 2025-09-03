/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/gate_futures/json/trades.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_trades_update_1", "[json_trades]") {
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
  core::json::BufferStack buffer{8192, 1};
  json::Trades trades{message, buffer};
  CHECK(trades.time == 1641366055s);
  REQUIRE(std::size(trades.result) == 1);
  auto &result_0 = trades.result[0];
  CHECK(result_0.create_time == 1641366055s);
  CHECK(result_0.create_time_ms == 1641366055959ms);
}

TEST_CASE("json_trades_update_2", "[json_trades]") {
  auto message = R"({)"
                 R"("channel":"futures.trades",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("id":360366365,)"
                 R"("size":-500,)"
                 R"("create_time":1727407595,)"
                 R"("create_time_ms":1727407595183,)"
                 R"("price":"64957",)"
                 R"("contract":"BTC_USDT")"
                 R"(})"
                 R"(],)"
                 R"("time":1727407595,)"
                 R"("time_ms":1727407595193)"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::Trades trades{message, buffer};
  REQUIRE(std::size(trades.result) == 1);
  auto &result_0 = trades.result[0];
  CHECK(result_0.create_time == 1727407595s);
  CHECK(result_0.create_time_ms == 1727407595183ms);
  CHECK(trades.time == 1727407595s);
  CHECK(trades.time_ms == 1727407595193ms);
}
