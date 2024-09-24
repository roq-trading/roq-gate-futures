/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/trade_login.hpp"
#include "roq/gate_futures/json/trade_parser.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_login_update_2", "[json_login]") {
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
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
}
