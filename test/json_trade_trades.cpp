/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/trade_parser.hpp"
#include "roq/gate_futures/json/user_trades.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_trades_update", "[json_trades]") {
  auto message = R"([)"
                 R"({)"
                 R"("price":"0.3397",)"
                 R"("text":"web",)"
                 R"("fee":"-0.0254775",)"
                 R"("create_time":1714215684.3627,)"
                 R"("point_fee":"0",)"
                 R"("trade_id":"10668304",)"
                 R"("contract":"BLZ_USDT",)"
                 R"("role":"maker",)"
                 R"("order_id":"460776906546",)"
                 R"("size":50,)"
                 R"("biz_info":"-",)"
                 R"("amend_text":"-")"
                 R"(},{)"
                 R"("price":"0.3399",)"
                 R"("text":"web",)"
                 R"("fee":"-0.00866745",)"
                 R"("create_time":1714215674.2741,)"
                 R"("point_fee":"0",)"
                 R"("trade_id":"10668303",)"
                 R"("contract":"BLZ_USDT",)"
                 R"("role":"maker",)"
                 R"("order_id":"460776972807",)"
                 R"("size":17,)"
                 R"("biz_info":"-",)"
                 R"("amend_text":"-")"
                 R"(})"
                 R"(])"sv;
  std::vector<std::byte> buffer(8192);
  [[maybe_unused]] json::UserTrades obj{message, buffer};
}

TEST_CASE("json_trades_update_2", "[json_trades]") {
  auto message = R"({)"
                 R"("channel":"futures.usertrades",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("price":"0.3397",)"
                 R"("text":"web",)"
                 R"("fee":"-0.0254775",)"
                 R"("create_time":1714215684.3627,)"
                 R"("point_fee":"0",)"
                 R"("trade_id":"10668304",)"
                 R"("contract":"BLZ_USDT",)"
                 R"("role":"maker",)"
                 R"("order_id":"460776906546",)"
                 R"("size":50,)"
                 R"("biz_info":"-",)"
                 R"("amend_text":"-")"
                 R"(},{)"
                 R"("price":"0.3399",)"
                 R"("text":"web",)"
                 R"("fee":"-0.00866745",)"
                 R"("create_time":1714215674.2741,)"
                 R"("point_fee":"0",)"
                 R"("trade_id":"10668303",)"
                 R"("contract":"BLZ_USDT",)"
                 R"("role":"maker",)"
                 R"("order_id":"460776972807",)"
                 R"("size":17,)"
                 R"("biz_info":"-",)"
                 R"("amend_text":"-")"
                 R"(})"
                 R"(],)"
                 R"("time":1727094610,)"
                 R"("time_ms":1727094610077)"
                 R"(})"sv;
  struct MyHandler final : public json::TradeParser::Handler {
    bool found = false;

   protected:
    void operator()(Trace<json::TradeLogin> const &) override { FAIL(); }
    void operator()(Trace<json::TradeSubscribe> const &) override { FAIL(); }
    void operator()(Trace<json::TradeBalances> const &) override { FAIL(); }
    void operator()(Trace<json::TradePositions> const &) override { FAIL(); }
    void operator()(Trace<json::TradeOrders> const &) override { FAIL(); }
    void operator()(Trace<json::TradeTrades> const &event) override {
      found = true;
      auto &result = event.value.result;
      CHECK(std::size(result) == 2);
    }
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
}
