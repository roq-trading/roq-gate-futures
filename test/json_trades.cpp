/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::Trades;

TEST_CASE("simple_1", "[json_trades]") {
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.time == 1641366055s);
    REQUIRE(std::size(obj.result) == 1);
    auto &result_0 = obj.result[0];
    CHECK(result_0.create_time == 1641366055s);
    CHECK(result_0.create_time_ms == 1641366055959ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("simple_2", "[json_trades]") {
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
  auto helper = [](value_type const &obj) {
    REQUIRE(std::size(obj.result) == 1);
    auto &result_0 = obj.result[0];
    CHECK(result_0.create_time == 1727407595s);
    CHECK(result_0.create_time_ms == 1727407595183ms);
    CHECK(obj.time == 1727407595s);
    CHECK(obj.time_ms == 1727407595193ms);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
