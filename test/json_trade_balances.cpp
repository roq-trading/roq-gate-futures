/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/trade_balances.hpp"
#include "roq/gate_futures/json/trade_parser.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_balances_update_1", "[json_balances]") {
  auto message = R"({)"
                 R"("time":1727169518,)"
                 R"("time_ms":1727169518435,)"
                 R"("channel":"futures.balances",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("text":"SOL_USDT:533324294866",)"
                 R"("time":1727169518,)"
                 R"("time_ms":1727169518413,)"
                 R"("type":"fee",)"
                 R"("user":"15564602",)"
                 R"("currency":"usdt",)"
                 R"("balance":296.16455822737,)"
                 R"("change":0.00738)"
                 R"(})"
                 R"(])"
                 R"(})"sv;
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &) override { FAIL(); }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &event) override {
      found = true;
      auto &positions = event.value;
      CHECK(positions.time == 1727169518s);
      CHECK(positions.time_ms == 1727169518435ms);
      REQUIRE(std::size(positions.result) == 1);
      auto &result_0 = positions.result[0];
      CHECK(result_0.time == 1727169518s);
      CHECK(result_0.time_ms == 1727169518413ms);
    }
    void operator()(Trace<json::TradePositions> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &) override { FAIL(); }
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
