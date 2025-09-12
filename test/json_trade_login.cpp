/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/gate_futures/json/trade_login.hpp"
#include "roq/gate_futures/json/trade_parser.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

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
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &event) override {
      found = true;
      auto &login = event.value;
      CHECK(login.header.status == 200);
    }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &) override { FAIL(); }
    void operator()(Trace<json::TradePositions> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderPlace> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderAmend> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancel> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  core::json::BufferStack buffer{8192, 1};
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
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
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &event) override {
      found = true;
      auto &login = event.value;
      CHECK(login.header.status == 400);
    }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &) override { FAIL(); }
    void operator()(Trace<json::TradePositions> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderPlace> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderAmend> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancel> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  core::json::BufferStack buffer{8192, 1};
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
}
