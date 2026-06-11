/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "trade_parser_tester.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::TradeLogin;

TEST_CASE("json_login_success", "[json_login]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727100172731",)"
                 R"("status":"200",)"
                 R"("channel":"futures.login",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc03381edc0")"
                 R"(},)"
                 R"("data":{)"
                 R"("result":{)"
                 R"("uid":"15564602",)"
                 R"("api_key":"98251e573f850df7d52e022c85cd2570")"
                 R"(})"
                 R"(},)"
                 R"("request_id":"1")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1727100172731ms);
    CHECK(obj.header.status == 200);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("json_login_failure", "[json_login]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1757642539954",)"
                 R"("status":"400",)"
                 R"("channel":"futures.login",)"
                 R"("event":"api",)"
                 R"("client_id":"178.198.131.37-0xc0be07c008",)"
                 R"("conn_id":"ba1505e5e03d3f8d",)"
                 R"("conn_trace_id":"543e485b7d4f91cfd125e5ab250b3595",)"
                 R"("trace_id":"5468e0f73b66069ed7d5fcb7a9a25aee",)"
                 R"("x_in_time":1757642539952584,)"
                 R"("x_out_time":1757642539954323},)"
                 R"("data":{)"
                 R"("errs":{)"
                 R"("label":"AUTHENTICATION_FAILED",)"
                 R"("message":"Client ip not in account ip white list")"
                 R"(})"
                 R"(},)"
                 R"("request_id":"1")"
                 R"(})"sv;
  auto helper = [](value_type const &obj) {
    CHECK(obj.header.response_time == 1757642539954ms);
    CHECK(obj.header.status == 400);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
