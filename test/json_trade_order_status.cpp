/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "trade_parser_tester.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::TradeOrderStatus;

// does not exist
TEST_CASE("does_not_exist", "[json_trade_order_status]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1793517305496",)"
                 R"("status":"404",)"
                 R"("channel":"futures.order_status",)"
                 R"("event":"api",)"
                 R"("client_id":"178.198.131.37-0xc16620a408",)"
                 R"("conn_id":"a5da40b5243a63b8",)"
                 R"("conn_trace_id":"610709dbae7b1ca4d26d4e563da4a6c0",)"
                 R"("trace_id":"570988d55775fe834c198976137a9493",)"
                 R"("x_in_time":1793517305493609,)"
                 R"("x_out_time":1793517305496669)"
                 R"(},)"
                 R"("data":{)"
                 R"("errs":{)"
                 R"("label":"ORDER_NOT_FOUND",)"
                 R"("message":"Order with ID t-qggC79f7o4YAAQAAAAAA not found")"
                 R"(})"
                 R"(},)"
                 R"("request_id":"21")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1793517305496ms);
    CHECK(obj.header.status == 404);
    CHECK(obj.data.errs.message == "Order with ID t-qggC79f7o4YAAQAAAAAA not found"sv);
    CHECK(obj.data.result.id == 0);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("simple", "[json_trade_order_status]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1793519362174",)"
                 R"("status":"200",)"
                 R"("channel":"futures.order_status",)"
                 R"("event":"api",)"
                 R"("client_id":"178.198.131.37-0xc134242008",)"
                 R"("conn_id":"2c11d969d7bb6c6f",)"
                 R"("conn_trace_id":"8c2b8367126eeab2502ddcbb802942e0",)"
                 R"("trace_id":"04d0544366ca3136924f66a2b00797de",)"
                 R"("x_in_time":1793519362172875,)"
                 R"("x_out_time":1793519362174105)"
                 R"(},)"
                 R"("data":{)"
                 R"("result":{)"
                 R"("id":168322045408588871,)"
                 R"("user":51007317,)"
                 R"("create_time":1793519359.304,)"
                 R"("update_time":1793519359.304,)"
                 R"("status":"open",)"
                 R"("contract":"SOL_USDT",)"
                 R"("price":"50",)"
                 R"("tif":"gtc",)"
                 R"("fill_price":"0",)"
                 R"("text":"t-HQgCdzI3pYYAAQAAAAAA",)"
                 R"("tkfr":"0.0003",)"
                 R"("mkfr":"0.0001",)"
                 R"("stp_act":"-",)"
                 R"("amend_text":"-",)"
                 R"("biz_info":"-",)"
                 R"("bbo":"-",)"
                 R"("market_order_slip_ratio":"0",)"
                 R"("pos_margin_mode":"cross",)"
                 R"("size":1,)"
                 R"("left":1)"
                 R"(})"
                 R"(},)"
                 R"("request_id":"HQgCdzI3pYYAAQAAAAAA")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1793519362174ms);
    CHECK(obj.header.status == 200);
    CHECK(std::empty(obj.data.errs.message));
    CHECK(obj.data.result.id == 168322045408588871);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
