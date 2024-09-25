/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/trade_parser.hpp"
#include "roq/gate_futures/json/trade_positions.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_positions_update_1", "[json_positions]") {
  auto message = R"({)"
                 R"("time":1727169518,)"
                 R"("time_ms":1727169518423,)"
                 R"("channel":"futures.positions",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("liq_price":0,)"
                 R"("mode":"single",)"
                 R"("realised_pnl":-0.287384,)"
                 R"("contract":"SOL_USDT",)"
                 R"("history_point":0,)"
                 R"("leverage_max":50,)"
                 R"("time":1727169518,)"
                 R"("time_ms":1727169518413,)"
                 R"("leverage":0,)"
                 R"("margin":296.16455822737,)"
                 R"("realised_point":0,)"
                 R"("maintenance_rate":0.01,)"
                 R"("size":1,)"
                 R"("entry_price":147.88,)"
                 R"("history_pnl":0,)"
                 R"("last_close_pnl":0,)"
                 R"("update_id":3,)"
                 R"("cross_leverage_limit":10,)"
                 R"("risk_limit":500000,)"
                 R"("user":"15564602")"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &) override { FAIL(); }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &) override { FAIL(); }
    void operator()(Trace<json::TradePositions> const &event) override {
      found = true;
      auto &positions = event.value;
      CHECK(positions.time == 1727169518s);
      CHECK(positions.time_ms == 1727169518423ms);
      REQUIRE(std::size(positions.result) == 1);
      auto &result_0 = positions.result[0];
      CHECK(result_0.time == 1727169518s);
      CHECK(result_0.time_ms == 1727169518413ms);
    }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &) override {}
    void operator()(Trace<json::TradeOrderPlace> const &) override { FAIL(); }
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
