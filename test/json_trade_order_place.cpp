/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/trade_parser.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

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
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &) override { FAIL(); }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &) override { FAIL(); }
    void operator()(Trace<json::TradePositions> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderPlace> const &event) override {
      found = true;
      auto &order_place = event.value;
      CHECK(order_place.header.response_time == 1727163050403ms);
      CHECK(order_place.data.result.create_time == 0ms);
      CHECK(order_place.data.result.finish_time == 0ms);
      CHECK(order_place.data.result.update_time == 0ms);
    }
    void operator()(Trace<json::TradeOrderAmend> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancel> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
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
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &) override { FAIL(); }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &) override { FAIL(); }
    void operator()(Trace<json::TradePositions> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderPlace> const &event) override {
      found = true;
      auto &order_place = event.value;
      CHECK(order_place.header.response_time == 1727227280918ms);
      CHECK(order_place.data.result.create_time == 1727227280916ms);
      CHECK(order_place.data.result.finish_time == 0ms);
      CHECK(order_place.data.result.update_time == 1727227280916ms);
    }
    void operator()(Trace<json::TradeOrderAmend> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancel> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
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
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &) override { FAIL(); }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &) override { FAIL(); }
    void operator()(Trace<json::TradePositions> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderPlace> const &event) override {
      found = true;
      auto &order_place = event.value;
      CHECK(order_place.header.response_time == 1727232753841ms);
      CHECK(order_place.data.result.create_time == 1727232753840ms);
      CHECK(order_place.data.result.finish_time == 1727232753840ms);
      CHECK(order_place.data.result.update_time == 1727232753840ms);
    }
    void operator()(Trace<json::TradeOrderAmend> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancel> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
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
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &) override { FAIL(); }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &) override { FAIL(); }
    void operator()(Trace<json::TradePositions> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderPlace> const &event) override {
      found = true;
      auto &order_place = event.value;
      CHECK(order_place.header.response_time == 1727164011854ms);
      CHECK(order_place.data.result.create_time == 0ms);
      CHECK(order_place.data.result.finish_time == 0ms);
      CHECK(order_place.data.result.update_time == 0ms);
    }
    void operator()(Trace<json::TradeOrderAmend> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancel> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
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
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &) override { FAIL(); }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &) override { FAIL(); }
    void operator()(Trace<json::TradePositions> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderPlace> const &event) override {
      found = true;
      auto &order_place = event.value;
      CHECK(order_place.header.response_time == 1727184373784ms);
      CHECK(order_place.data.result.create_time == 0ms);
      CHECK(order_place.data.result.finish_time == 0ms);
      CHECK(order_place.data.result.update_time == 0ms);
    }
    void operator()(Trace<json::TradeOrderAmend> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancel> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderCancelCP> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrderList> const &) override { FAIL(); }
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
}
