/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/trades.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_trades_update", "[json_trades]") {
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
  std::vector<std::byte> buffer(8192);
  json::Trades trades{message, buffer};
  CHECK(trades.time == 1641366055s);
  REQUIRE(std::size(trades.result) == 1);
  auto &result_0 = trades.result[0];
  CHECK(result_0.create_time == 1641366055s);
  CHECK(result_0.create_time_ms == 1641366055959ms);
}
