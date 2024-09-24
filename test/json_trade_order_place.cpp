/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/trade_parser.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_order_place_update_1", "[json_order_place]") {
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
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  // CHECK(handler.found == true);
}

TEST_CASE("json_order_place_update_2", "[json_order_place]") {
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
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  // CHECK(handler.found == true);
}
