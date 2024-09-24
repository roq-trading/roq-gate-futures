/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/orders.hpp"
#include "roq/gate_futures/json/trade_parser.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_orders_update", "[json_orders]") {
  auto message = R"([)"
                 R"({)"
                 R"("status":"open",)"
                 R"("size":-1,)"
                 R"("left":-1,)"
                 R"("id":532928247170,)"
                 R"("is_liq":false,)"
                 R"("is_close":false,)"
                 R"("contract":"SOL_USDT",)"
                 R"("text":"web",)"
                 R"("fill_price":"0",)"
                 R"("iceberg":0,)"
                 R"("tif":"gtc",)"
                 R"("is_reduce_only":false,)"
                 R"("create_time":1727084914.392,)"
                 R"("update_time":1727084914.392,)"
                 R"("price":"160",)"
                 R"("mkfr":"-0.00005",)"
                 R"("tkfr":"0.00015",)"
                 R"("refr":"0",)"
                 R"("refu":0,)"
                 R"("user":15564602,)"
                 R"("biz_info":"-",)"
                 R"("amend_text":"-",)"
                 R"("stp_act":"-",)"
                 R"("stp_id":0,)"
                 R"("update_id":1,)"
                 R"("pnl":"0",)"
                 R"("pnl_margin":"0")"
                 R"(})"
                 R"(])"sv;
  std::vector<std::byte> buffer(8192);
  [[maybe_unused]] json::Orders obj{message, buffer};
}

TEST_CASE("json_orders_update_2", "[json_orders]") {
  auto message = R"({)"
                 R"("channel":"futures.orders",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("amend_text":"-",)"
                 R"("biz_info":"-",)"
                 R"("contract":"SOL_USDT",)"
                 R"("create_time":1727084914,)"
                 R"("create_time_ms":1727084914383,)"
                 R"("fill_price":0,)"
                 R"("finish_as":"cancelled",)"
                 R"("finish_time":1727094610,)"
                 R"("finish_time_ms":1727094610074,)"
                 R"("iceberg":0,)"
                 R"("id":532928247170,)"
                 R"("is_close":false,)"
                 R"("is_liq":false,)"
                 R"("is_reduce_only":false,)"
                 R"("left":-1,)"
                 R"("mkfr":-0.00005,)"
                 R"("price":160,)"
                 R"("refr":0,)"
                 R"("refu":0,)"
                 R"("size":-1,)"
                 R"("status":"finished",)"
                 R"("stop_loss_price":"",)"
                 R"("stop_profit_price":"",)"
                 R"("stp_act":"-",)"
                 R"("stp_id":"0",)"
                 R"("text":"web",)"
                 R"("tif":"gtc",)"
                 R"("tkfr":0.00015,)"
                 R"("update_id":2,)"
                 R"("update_time":1727094610074,)"
                 R"("user":"15564602")"
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
    void operator()(Trace<json::TradeOrders> const &event) override {
      found = true;
      auto &result = event.value.result;
      CHECK(std::size(result) == 1);
    }
    void operator()(Trace<json::TradeTrades> const &) override { FAIL(); }
  } handler;
  std::vector<std::byte> buffer(8192);
  TraceInfo trace_info;
  [[maybe_unused]] auto res = json::TradeParser::dispatch(handler, message, buffer, trace_info);
  CHECK(handler.found == true);
}
