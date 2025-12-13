/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "trade_parser_tester.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::TradeOrderCancel;

TEST_CASE("json_order_cancel_success_1", "[json_order_cancel]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727228915694",)"
                 R"("status":"200",)"
                 R"("channel":"futures.order_cancel",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc05759e3c0")"
                 R"(},)"
                 R"("data":{)"
                 R"("result":{)"
                 R"("text":"t-IQICM2phRhsCAQAAAAAA",)"
                 R"("price":"100",)"
                 R"("biz_info":"-",)"
                 R"("tif":"gtc",)"
                 R"("amend_text":"-",)"
                 R"("status":"finished",)"
                 R"("contract":"SOL_USDT",)"
                 R"("stp_act":"-",)"
                 R"("finish_as":"cancelled",)"
                 R"("fill_price":"0",)"
                 R"("id":533621167472,)"
                 R"("create_time":1727228909.079,)"
                 R"("size":1,)"
                 R"("finish_time":1727228915.694,)"
                 R"("update_time":1727228915.694,)"
                 R"("left":1,)"
                 R"("user":15564602)"
                 R"(})"
                 R"(},)"
                 R"("request_id":"IgICM2phRhsCAgAAAAAA")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1727228915694ms);
    CHECK(obj.data.result.create_time == 1727228909079ms);
    CHECK(obj.data.result.finish_time == 1727228915694ms);
    CHECK(obj.data.result.update_time == 1727228915694ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

// no order
TEST_CASE("json_order_cancel_error_1", "[json_order_cancel]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727166448270",)"
                 R"("status":"400",)"
                 R"("channel":"futures.order_cancel",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc15590ab40")"
                 R"(},)"
                 R"("data":{)"
                 R"("errs":{)"
                 R"("label":"ORDER_NOT_FOUND",)"
                 R"("message":"label: ORDER_NOT_FOUND, message: ")"
                 R"(})"
                 R"(},)"
                 R"("request_id":"LAICgYu1zRkCAgAAAAAA")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1727166448270ms);
    CHECK(obj.data.result.create_time == 0ms);
    CHECK(obj.data.result.finish_time == 0ms);
    CHECK(obj.data.result.update_time == 0ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

// too late
TEST_CASE("json_order_cancel_error_2", "[json_order_cancel]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727231274435",)"
                 R"("status":"404",)"
                 R"("channel":"futures.order_cancel",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc15662cc80"},)"
                 R"("data":{)"
                 R"("errs":{)"
                 R"("label":"ORDER_NOT_FOUND",)"
                 R"("message":"label: ORDER_NOT_FOUND, message: ")"
                 R"(})"
                 R"(},)"
                 R"("request_id":"VgICOeqNVBsCAgAAAAAA")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1727231274435ms);
    CHECK(obj.data.result.create_time == 0ms);
    CHECK(obj.data.result.finish_time == 0ms);
    CHECK(obj.data.result.update_time == 0ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
