/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/gate_futures/json/trade_parser.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

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
    void operator()(Trace<json::TradeOrderCancel> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &event) override {
      found = true;
      auto &order_list = event.value;
      CHECK(order_list.header.response_time == 1727250838664ms);
    }
  } handler;
  core::json::BufferStack buffer{8192, 1};
  TraceInfo trace_info;
  auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(res == true);
  CHECK(handler.found == true);
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
    void operator()(Trace<json::TradeOrderCancel> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &event) override {
      found = true;
      auto &order_list = event.value;
      CHECK(order_list.header.response_time == 1727251141654ms);
      auto &result_0 = order_list.data.result[0];
      CHECK(result_0.create_time == 1727251125952ms);
      CHECK(result_0.update_time == 1727251125952ms);
    }
  } handler;
  core::json::BufferStack buffer{8192, 1};
  TraceInfo trace_info;
  auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(res == true);
  CHECK(handler.found == true);
}
