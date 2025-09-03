/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/gate_futures/json/trade_parser.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_order_amend_success_1", "[json_order_amend]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727230008594",)"
                 R"("status":"200",)"
                 R"("channel":"futures.order_amend",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc1e848a780")"
                 R"(},)"
                 R"("data":{)"
                 R"("result":{)"
                 R"("text":"t-9QIC4ZGRSxsCAQAAAAAA",)"
                 R"("price":"101",)"
                 R"("biz_info":"-",)"
                 R"("tif":"gtc",)"
                 R"("amend_text":"9QIC4ZGRSxsCAQAAAAAA",)"
                 R"("status":"open",)"
                 R"("contract":"SOL_USDT",)"
                 R"("stp_act":"-",)"
                 R"("fill_price":"0",)"
                 R"("id":533626886800,)"
                 R"("create_time":1727229980.658,)"
                 R"("size":1,)"
                 R"("update_time":1727230008.594,)"
                 R"("left":1,)"
                 R"("user":15564602)"
                 R"(})"
                 R"(},)"
                 R"("request_id":"9gIC4ZGRSxsCAgAAAAAA")"
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
    void operator()(Trace<json::TradeOrderAmend> const &event) override {
      found = true;
      auto &order_amend = event.value;
      CHECK(order_amend.header.response_time == 1727230008594ms);
      CHECK(order_amend.data.result.create_time == 1727229980658ms);
      CHECK(order_amend.data.result.update_time == 1727230008594ms);
    }
    void operator()(Trace<json::TradeOrderCancel> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  core::json::BufferStack buffer{8192, 1};
  TraceInfo trace_info;
  auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(res == true);
  CHECK(handler.found == true);
}

TEST_CASE("json_order_amend_success_2", "[json_order_amend]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727402744861",)"
                 R"("status":"200",)"
                 R"("channel":"futures.order_amend",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc18b8f0c80")"
                 R"(},)"
                 R"("data":{)"
                 R"("result":{)"
                 R"("text":"t-CwICjgCMUh8CAQAAAAAA",)"
                 R"("price":"155",)"
                 R"("biz_info":"-",)"
                 R"("tif":"gtc",)"
                 R"("amend_text":"CwICjgCMUh8CAQAAAAAA",)"
                 R"("status":"finished",)"
                 R"("contract":"SOL_USDT",)"
                 R"("stp_act":"-",)"
                 R"("finish_as":"filled",)"
                 R"("fill_price":"154.87",)"
                 R"("id":534456091481,)"
                 R"("create_time":1727402735.097,)"
                 R"("size":1,)"
                 R"("finish_time":1727402744.86,)"
                 R"("update_time":1727402744.86,)"
                 R"("left":0,)"
                 R"("user":15564602)"
                 R"(})"
                 R"(},)"
                 R"("request_id":"CAICjgCMUh8CAgAAAAAA")"
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
    void operator()(Trace<json::TradeOrderAmend> const &event) override {
      found = true;
      auto &order_amend = event.value;
      CHECK(order_amend.header.response_time == 1727402744861ms);
      CHECK(order_amend.data.result.create_time == 1727402735097ms);
      CHECK(order_amend.data.result.finish_time == 1727402744860ms);
      CHECK(order_amend.data.result.update_time == 1727402744860ms);
    }
    void operator()(Trace<json::TradeOrderCancel> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  core::json::BufferStack buffer{8192, 1};
  TraceInfo trace_info;
  auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(res == true);
  CHECK(handler.found == true);
}

TEST_CASE("json_order_amend_error_1", "[json_order_amend]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727230686049",)"
                 R"("status":"405",)"
                 R"("channel":"futures.order_amend",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc1bb4c0140")"
                 R"(},)"
                 R"("data":{)"
                 R"("errs":{)"
                 R"("label":"",)"
                 R"("message":"405 Method Not Allowed, ")"
                 R"(})"
                 R"(},)"
                 R"("request_id":"gQICc_oFURsCAgAAAAAA")"
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
    void operator()(Trace<json::TradeOrderAmend> const &event) override {
      found = true;
      auto &order_amend = event.value;
      CHECK(order_amend.header.response_time == 1727230686049ms);
      CHECK(order_amend.data.result.create_time == 0ms);
      CHECK(order_amend.data.result.update_time == 0ms);
    }
    void operator()(Trace<json::TradeOrderCancel> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  core::json::BufferStack buffer{8192, 1};
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
}
