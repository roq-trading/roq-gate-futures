/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "trade_parser_tester.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::TradeTrades;

TEST_CASE("json_trades_update_1", "[json_trades]") {
  auto message = R"({)"
                 R"("channel":"futures.usertrades",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("id":"60117338",)"
                 R"("order_id":"533321246654",)"
                 R"("contract":"SOL_USDT",)"
                 R"("create_time":1727168891,)"
                 R"("create_time_ms":1727168891629,)"
                 R"("size":1,)"
                 R"("role":"maker",)"
                 R"("price":"148",)"
                 R"("text":"t-sAIC5kyY3xkCAQAAAAAA",)"
                 R"("fee":-0.0074,)"
                 R"("point_fee":0,)"
                 R"("amend_text":"-",)"
                 R"("biz_info":"-",)"
                 R"("close_size":"0")"
                 R"(})"
                 R"(],)"
                 R"("time":1727168891,)"
                 R"("time_ms":1727168891630)"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.time == 1727168891s);
    CHECK(obj.time_ms == 1727168891630ms);
    REQUIRE(std::size(obj.result) == 1);
    auto &result_0 = obj.result[0];
    CHECK(result_0.create_time == 1727168891s);
    CHECK(result_0.create_time_ms == 1727168891629ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

// open 1
TEST_CASE("json_trades_update_2", "[json_trades]") {
  auto message = R"({)"
                 R"("channel":"futures.usertrades",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("id":"60117508",)"
                 R"("order_id":"533322588362",)"
                 R"("contract":"SOL_USDT",)"
                 R"("create_time":1727169120,)"
                 R"("create_time_ms":1727169120012,)"
                 R"("size":1,)"
                 R"("role":"taker",)"
                 R"("price":"147.76",)"
                 R"("text":"t-DAIC8k8O4hkCAQAAAAAA",)"
                 R"("fee":0.022164,)"
                 R"("point_fee":0,)"
                 R"("amend_text":"-",)"
                 R"("biz_info":"-",)"
                 R"("close_size":"0")"
                 R"(})"
                 R"(],)"
                 R"("time":1727169120,)"
                 R"("time_ms":1727169120015)"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.time == 1727169120s);
    CHECK(obj.time_ms == 1727169120015ms);
    REQUIRE(std::size(obj.result) == 1);
    auto &result_0 = obj.result[0];
    CHECK(result_0.create_time == 1727169120s);
    CHECK(result_0.create_time_ms == 1727169120012ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

// close 1
TEST_CASE("json_trades_update_3", "[json_trades]") {
  auto message = R"({)"
                 R"("channel":"futures.usertrades",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("id":"60117765",)"
                 R"("order_id":"533324294866",)"
                 R"("contract":"SOL_USDT",)"
                 R"("create_time":1727169518,)"
                 R"("create_time_ms":1727169518413,)"
                 R"("size":-1,)"
                 R"("role":"maker",)"
                 R"("price":"147.6",)"
                 R"("text":"t-KAICtn9Y5BkCAQAAAAAA",)"
                 R"("fee":-0.00738,)"
                 R"("point_fee":0,)"
                 R"("amend_text":"-",)"
                 R"("biz_info":"-",)"
                 R"("close_size":"-1")"
                 R"(})"
                 R"(],)"
                 R"("time":1727169518,)"
                 R"("time_ms":1727169518414)"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.time == 1727169518s);
    CHECK(obj.time_ms == 1727169518414ms);
    REQUIRE(std::size(obj.result) == 1);
    auto &result_0 = obj.result[0];
    CHECK(result_0.create_time == 1727169518s);
    CHECK(result_0.create_time_ms == 1727169518413ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
