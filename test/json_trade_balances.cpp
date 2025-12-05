/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "trade_parser_tester.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::TradeBalances;

TEST_CASE("json_balances_update_1", "[json_balances]") {
  auto message = R"({)"
                 R"("time":1727169518,)"
                 R"("time_ms":1727169518435,)"
                 R"("channel":"futures.balances",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("text":"SOL_USDT:533324294866",)"
                 R"("time":1727169518,)"
                 R"("time_ms":1727169518413,)"
                 R"("type":"fee",)"
                 R"("user":"15564602",)"
                 R"("currency":"usdt",)"
                 R"("balance":296.16455822737,)"
                 R"("change":0.00738)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.time == 1727169518s);
    CHECK(obj.time_ms == 1727169518435ms);
    REQUIRE(std::size(obj.result) == 1);
    auto &result_0 = obj.result[0];
    CHECK(result_0.time == 1727169518s);
    CHECK(result_0.time_ms == 1727169518413ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
