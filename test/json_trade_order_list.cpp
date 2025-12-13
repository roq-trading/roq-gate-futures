/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "trade_parser_tester.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::TradeOrderList;

// empty
TEST_CASE("json_trade_order_list_1", "[json_trade_order_list]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727250838664",)"
                 R"("status":"200",)"
                 R"("channel":"futures.order_list",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc1224a3900")"
                 R"(},)"
                 R"("data":{)"
                 R"("result":[])"
                 R"(},)"
                 R"("request_id":"6")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) { CHECK(obj.header.response_time == 1727250838664ms); };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("json_trade_order_list_2", "[json_trade_order_list]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727251141654",)"
                 R"("status":"200",)"
                 R"("channel":"futures.order_list",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc0e3620b40")"
                 R"(},)"
                 R"("data":{)"
                 R"("result":[{)"
                 R"("text":"t-KAICitvsyhsCAQAAAAAA",)"
                 R"("price":"100",)"
                 R"("biz_info":"-",)"
                 R"("tif":"gtc",)"
                 R"("amend_text":"-",)"
                 R"("status":"open",)"
                 R"("contract":"SOL_USDT",)"
                 R"("stp_act":"-",)"
                 R"("fill_price":"0",)"
                 R"("id":533716470457,)"
                 R"("create_time":1727251125.952,)"
                 R"("size":1,)"
                 R"("update_time":1727251125.952,)"
                 R"("left":1,)"
                 R"("user":15564602)"
                 R"(})"
                 R"(])"
                 R"(},)"
                 R"("request_id":"6")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1727251141654ms);
    auto &result_0 = obj.data.result[0];
    CHECK(result_0.create_time == 1727251125952ms);
    CHECK(result_0.update_time == 1727251125952ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
