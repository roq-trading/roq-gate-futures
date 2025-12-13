/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "trade_parser_tester.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::TradeOrderPlace;

// {"time":1727318820,"time_ms":1727318820205,"conn_id":"c7799af51014ea53","trace_id":"4d07b2abbe99cc9656b50d9dc980cb60","channel":"","event":"","error":{"code":1,"message":"request
// message need json scheme"},"result":{"status":"fail"}}

// note! there are two acks -- received and success/failure

TEST_CASE("json_order_place_received_1", "[json_order_place]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727163050403",)"
                 R"("status":"200",)"
                 R"("channel":"futures.order_place",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc11f193540",)"
                 R"("conn_id":"362dc864d9e0358a",)"
                 R"("trace_id":"b337ba50adcf08ee497cd14801eada95")"
                 R"(},)"
                 R"("data":{)"
                 R"("result":{)"
                 R"("req_id":"6",)"
                 R"("api_key":"",)"
                 R"("timestamp":"",)"
                 R"("signature":"",)"
                 R"("trace_id":"b337ba50adcf08ee497cd14801eada95",)"
                 R"("text":"",)"
                 R"("req_header":{)"
                 R"("trace_id":"b337ba50adcf08ee497cd14801eada95")"
                 R"(},)"
                 R"("req_param":{)"
                 R"("text":"t-mgICtyDtvRkCAQAAAAAA",)"
                 R"("contract":"SOL_USDT",)"
                 R"("size":1,)"
                 R"("iceberg":0,)"
                 R"("price":"100.00",)"
                 R"("close":false,)"
                 R"("reduce_only":false,)"
                 R"("tif":"GTC")"
                 R"(})"
                 R"(})"
                 R"(},)"
                 R"("request_id":"6",)"
                 R"("ack":true)"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1727163050403ms);
    CHECK(obj.data.result.create_time == 0ms);
    CHECK(obj.data.result.finish_time == 0ms);
    CHECK(obj.data.result.update_time == 0ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

// passive
TEST_CASE("json_order_place_success_1", "[json_order_place]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727227280918",)"
                 R"("status":"200",)"
                 R"("channel":"futures.order_place",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc17bf95180")"
                 R"(},)"
                 R"("data":{)"
                 R"("result":{)"
                 R"("text":"t-JQICBoPDPBsCAQAAAAAA",)"
                 R"("price":"100",)"
                 R"("biz_info":"-",)"
                 R"("tif":"gtc",)"
                 R"("amend_text":"-",)"
                 R"("status":"open",)"
                 R"("contract":"SOL_USDT",)"
                 R"("stp_act":"-",)"
                 R"("fill_price":"0",)"
                 R"("id":533610324468,)"
                 R"("create_time":1727227280.916,)"
                 R"("size":1,)"
                 R"("update_time":1727227280.916,)"
                 R"("left":1,)"
                 R"("user":15564602}},)"
                 R"("request_id":"JQICBoPDPBsCAQAAAAAA")"
                 R"(})"
                 R"(})"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1727227280918ms);
    CHECK(obj.data.result.create_time == 1727227280916ms);
    CHECK(obj.data.result.finish_time == 0ms);
    CHECK(obj.data.result.update_time == 1727227280916ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

// aggressive
TEST_CASE("json_order_place_success_2", "[json_order_place]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727232753841",)"
                 R"("status":"200",)"
                 R"("channel":"futures.order_place",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc05874e280")"
                 R"(},)"
                 R"("data":{)"
                 R"("result":{)"
                 R"("text":"t-OgICmpo4XRsCAQAAAAAA",)"
                 R"("price":"151.7",)"
                 R"("biz_info":"-",)"
                 R"("tif":"gtc",)"
                 R"("amend_text":"-",)"
                 R"("status":"finished",)"
                 R"("contract":"SOL_USDT",)"
                 R"("stp_act":"-",)"
                 R"("finish_as":"filled",)"
                 R"("fill_price":"151.62",)"
                 R"("id":533638125442,)"
                 R"("create_time":1727232753.84,)"
                 R"("size":1,)"
                 R"("finish_time":1727232753.84,)"
                 R"("update_time":1727232753.84,)"
                 R"("left":0,)"
                 R"("user":15564602)"
                 R"(})"
                 R"(},)"
                 R"("request_id":"OgICmpo4XRsCAQAAAAAA")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1727232753841ms);
    CHECK(obj.data.result.create_time == 1727232753840ms);
    CHECK(obj.data.result.finish_time == 1727232753840ms);
    CHECK(obj.data.result.update_time == 1727232753840ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("json_order_place_error_1", "[json_order_place]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727164011854",)"
                 R"("status":"400",)"
                 R"("channel":"futures.order_place",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc03161b400")"
                 R"(},)"
                 R"("data":{)"
                 R"("errs":{)"
                 R"("label":"INVALID_ARGUMENT",)"
                 R"("message":"label: INVALID_ARGUMENT, message: invalid TIF")"
                 R"(})"
                 R"(},)"
                 R"("request_id":"6")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1727164011854ms);
    CHECK(obj.data.result.create_time == 0ms);
    CHECK(obj.data.result.finish_time == 0ms);
    CHECK(obj.data.result.update_time == 0ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("json_order_place_error_2", "[json_order_place]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727184373784",)"
                 R"("status":"400",)"
                 R"("channel":"futures.order_place",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc11f338b40"},)"
                 R"("data":{)"
                 R"("errs":{)"
                 R"("label":"INVALID_PARAM_VALUE",)"
                 R"("message":"label: INVALID_PARAM_VALUE, message: set stp_act without stp_id")"
                 R"(})"
                 R"(},)"
                 R"("request_id":"6")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1727184373784ms);
    CHECK(obj.data.result.create_time == 0ms);
    CHECK(obj.data.result.finish_time == 0ms);
    CHECK(obj.data.result.update_time == 0ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
