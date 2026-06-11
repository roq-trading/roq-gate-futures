/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "trade_parser_tester.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::TradePositions;

TEST_CASE("json_positions_update_1", "[json_positions]") {
  auto message = R"({)"
                 R"("time":1727169518,)"
                 R"("time_ms":1727169518423,)"
                 R"("channel":"futures.positions",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("liq_price":0,)"
                 R"("mode":"single",)"
                 R"("realised_pnl":-0.287384,)"
                 R"("contract":"SOL_USDT",)"
                 R"("history_point":0,)"
                 R"("leverage_max":50,)"
                 R"("time":1727169518,)"
                 R"("time_ms":1727169518413,)"
                 R"("leverage":0,)"
                 R"("margin":296.16455822737,)"
                 R"("realised_point":0,)"
                 R"("maintenance_rate":0.01,)"
                 R"("size":1,)"
                 R"("entry_price":147.88,)"
                 R"("history_pnl":0,)"
                 R"("last_close_pnl":0,)"
                 R"("update_id":3,)"
                 R"("cross_leverage_limit":10,)"
                 R"("risk_limit":500000,)"
                 R"("user":"15564602")"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.time == 1727169518s);
    CHECK(obj.time_ms == 1727169518423ms);
    REQUIRE(std::size(obj.result) == 1);
    auto &result_0 = obj.result[0];
    CHECK(result_0.time == 1727169518s);
    CHECK(result_0.time_ms == 1727169518413ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
