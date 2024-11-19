/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/trade_parser.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_order_cancel_cp_success_1", "[json_order_cancel_cp]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727227666487",)"
                 R"("status":"200",)"
                 R"("channel":"futures.order_cancel_cp",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc15cfc3900"},)"
                 R"("data":{)"
                 R"("result":[{)"
                 R"("text":"t-JQICBoPDPBsCAQAAAAAA",)"
                 R"("price":"100",)"
                 R"("biz_info":"-",)"
                 R"("tif":"gtc",)"
                 R"("amend_text":"-",)"
                 R"("status":"finished",)"
                 R"("contract":"SOL_USDT",)"
                 R"("stp_act":"-",)"
                 R"("finish_as":"cancelled",)"
                 R"("fill_price":"0",)"
                 R"("id":533610324468,)"
                 R"("create_time":1727227280.916,)"
                 R"("size":1,)"
                 R"("finish_time":1727227666.486,)"
                 R"("update_time":1727227666.486,)"
                 R"("left":1,)"
                 R"("user":15564602)"
                 R"(})"
                 R"(])"
                 R"(},)"
                 R"("request_id":"RwICAAAAAAAAAAAAAAAA")"
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
    void operator()(Trace<json::TradeOrderCancelCP> const &event) override {
      found = true;
      auto &order_cancel_cp = event.value;
      CHECK(order_cancel_cp.header.response_time == 1727227666487ms);
      auto &result_0 = order_cancel_cp.data.result[0];
      CHECK(result_0.create_time == 1727227280916ms);
      CHECK(result_0.finish_time == 1727227666486ms);
      CHECK(result_0.update_time == 1727227666486ms);
    }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(res == true);
  CHECK(handler.found == true);
}

TEST_CASE("json_order_cancel_cp_success_2", "[json_order_cancel_cp]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727229849721",)"
                 R"("status":"200",)"
                 R"("channel":"futures.order_cancel_cp",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc1e848a780")"
                 R"(},)"
                 R"("data":{)"
                 R"("result":[{)"
                 R"("text":"t-ywIC35GRSxsCAQAAAAAA",)"
                 R"("price":"100",)"
                 R"("biz_info":"-",)"
                 R"("tif":"gtc",)"
                 R"("amend_text":"-",)"
                 R"("status":"finished",)"
                 R"("contract":"SOL_USDT",)"
                 R"("stp_act":"-",)"
                 R"("finish_as":"cancelled",)"
                 R"("fill_price":"0",)"
                 R"("id":533626074371,)"
                 R"("create_time":1727229839.129,)"
                 R"("size":1,)"
                 R"("finish_time":1727229849.721,)"
                 R"("update_time":1727229849.721,)"
                 R"("left":1,)"
                 R"("user":15564602)"
                 R"(},{)"
                 R"("text":"t-9AIC4JGRSxsCAQAAAAAA",)"
                 R"("price":"101",)"
                 R"("biz_info":"-",)"
                 R"("tif":"gtc",)"
                 R"("amend_text":"-",)"
                 R"("status":"finished",)"
                 R"("contract":"SOL_USDT",)"
                 R"("stp_act":"-",)"
                 R"("finish_as":"cancelled",)"
                 R"("fill_price":"0",)"
                 R"("id":533626167661,)"
                 R"("create_time":1727229847.193,)"
                 R"("size":1,)"
                 R"("finish_time":1727229849.721,)"
                 R"("update_time":1727229849.721,)"
                 R"("left":1,)"
                 R"("user":15564602)"
                 R"(})"
                 R"(])"
                 R"(},)"
                 R"("request_id":"RwICAAAAAAAAAAAAAAAA")"
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
    void operator()(Trace<json::TradeOrderCancelCP> const &event) override {
      found = true;
      auto &order_cancel_cp = event.value;
      CHECK(order_cancel_cp.header.response_time == 1727229849721ms);
      auto &result_0 = order_cancel_cp.data.result[0];
      CHECK(result_0.create_time == 1727229839129ms);
      CHECK(result_0.finish_time == 1727229849721ms);
      CHECK(result_0.update_time == 1727229849721ms);
      auto &result_1 = order_cancel_cp.data.result[1];
      CHECK(result_1.create_time == 1727229847193ms);
      CHECK(result_1.finish_time == 1727229849721ms);
      CHECK(result_1.update_time == 1727229849721ms);
    }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(res == true);
  CHECK(handler.found == true);
}

TEST_CASE("json_order_cancel_cp_error_1", "[json_order_cancel_cp]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727226815570",)"
                 R"("status":"200",)"
                 R"("channel":"futures.order_cancel_cp",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc07cb1a8c0")"
                 R"(},)"
                 R"("data":{)"
                 R"("result":[])"
                 R"(},)"
                 R"("request_id":"RwICAAAAAAAAAAAAAAAA")"
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
    void operator()(Trace<json::TradeOrderCancelCP> const &event) override {
      found = true;
      auto &order_cancel_cp = event.value;
      CHECK(order_cancel_cp.header.response_time == 1727226815570ms);
    }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(res == true);
  CHECK(handler.found == true);
}

TEST_CASE("json_order_cancel_cp_error_2", "[json_order_cancel_cp]") {
  auto message = R"({)"
                 R"("header":{)"
                 R"("response_time":"1727404856256",)"
                 R"("status":"400",)"
                 R"("channel":"futures.order_cancel_cp",)"
                 R"("event":"api",)"
                 R"("client_id":"94.228.147.34-0xc172c7ba40")"
                 R"(},)"
                 R"("data":{)"
                 R"("errs":{)"
                 R"("label":"CONTRACT_NOT_FOUND",)"
                 R"("message":"label: CONTRACT_NOT_FOUND, message: ")"
                 R"(})"
                 R"(},)"
                 R"("request_id":"RwICAAAAAAAAAAAAAAAA")"
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
    void operator()(Trace<json::TradeOrderCancelCP> const &event) override {
      found = true;
      auto &order_cancel_cp = event.value;
      CHECK(order_cancel_cp.header.response_time == 1727404856256ms);
    }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(res == true);
  CHECK(handler.found == true);
}
