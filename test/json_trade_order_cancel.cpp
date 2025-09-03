/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/gate_futures/json/trade_parser.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

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
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &) override { FAIL(); }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &) override { FAIL(); }
    void operator()(Trace<json::TradePositions> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderPlace> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderAmend> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancel> const &event) override {
      found = true;
      auto &order_cancel = event.value;
      CHECK(order_cancel.header.response_time == 1727228915694ms);
      CHECK(order_cancel.data.result.create_time == 1727228909079ms);
      CHECK(order_cancel.data.result.finish_time == 1727228915694ms);
      CHECK(order_cancel.data.result.update_time == 1727228915694ms);
    }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  core::json::BufferStack buffer{8192, 1};
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
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
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &) override { FAIL(); }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &) override { FAIL(); }
    void operator()(Trace<json::TradePositions> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderPlace> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderAmend> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancel> const &event) override {
      found = true;
      auto &order_cancel = event.value;
      CHECK(order_cancel.header.response_time == 1727166448270ms);
      CHECK(order_cancel.data.result.create_time == 0ms);
      CHECK(order_cancel.data.result.finish_time == 0ms);
      CHECK(order_cancel.data.result.update_time == 0ms);
    }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  core::json::BufferStack buffer{8192, 1};
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
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
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &) override { FAIL(); }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &) override { FAIL(); }
    void operator()(Trace<json::TradePositions> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderPlace> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderAmend> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancel> const &event) override {
      found = true;
      auto &order_cancel = event.value;
      CHECK(order_cancel.header.response_time == 1727231274435ms);
      CHECK(order_cancel.data.result.create_time == 0ms);
      CHECK(order_cancel.data.result.finish_time == 0ms);
      CHECK(order_cancel.data.result.update_time == 0ms);
    }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  core::json::BufferStack buffer{8192, 1};
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
}
