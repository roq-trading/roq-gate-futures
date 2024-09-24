/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/trade_parser.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_subscribe_update_2", "[json_subscribe]") {
  auto message = R"({)"
                 R"("time":1727100173,)"
                 R"("time_ms":1727100173023,)"
                 R"("id":2,)"
                 R"("conn_id":"e92ba03dcef376d0",)"
                 R"("trace_id":"24e5ec7f9e211cc83b0d3da46e40e487",)"
                 R"("channel":"futures.balances",)"
                 R"("event":"subscribe",)"
                 R"("payload":["15564602"],)"
                 R"("result":{)"
                 R"("status":"success")"
                 R"(})"
                 R"(})"sv;
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &) override { FAIL(); }
    void operator()(Trace<json::TradeSubscribe> const &) override { found = true; }
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
