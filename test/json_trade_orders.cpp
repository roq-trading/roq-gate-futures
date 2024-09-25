/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/trade_parser.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_orders_update_1", "[json_orders]") {
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

TEST_CASE("json_orders_update_2", "[json_orders]") {
  auto message = R"({)"
                 R"("channel":"futures.orders",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("amend_text":"-",)"
                 R"("biz_info":"-",)"
                 R"("contract":"SOL_USDT",)"
                 R"("create_time":1727164283,)"
                 R"("create_time_ms":1727164283948,)"
                 R"("fill_price":0,)"
                 R"("finish_as":"cancelled",)"
                 R"("finish_time":1727164365,)"
                 R"("finish_time_ms":1727164365250,)"
                 R"("iceberg":0,)"
                 R"("id":533298725911,)"
                 R"("is_close":false,)"
                 R"("is_liq":false,)"
                 R"("is_reduce_only":false,)"
                 R"("left":1,)"
                 R"("mkfr":-0.00005,)"
                 R"("price":100,)"
                 R"("refr":0,)"
                 R"("refu":0,)"
                 R"("size":1,)"
                 R"("status":"finished",)"
                 R"("stop_loss_price":"",)"
                 R"("stop_profit_price":"",)"
                 R"("stp_act":"-",)"
                 R"("stp_id":"0",)"
                 R"("text":"t-FQIC5CBJxRkCAQAAAAAA",)"
                 R"("tif":"gtc",)"
                 R"("tkfr":0.00015,)"
                 R"("update_id":2,)"
                 R"("update_time":1727164365250,)"
                 R"("user":"15564602")"
                 R"(})"
                 R"(],)"
                 R"("time":1727164365,)"
                 R"("time_ms":1727164365252)"
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

// _new
TEST_CASE("json_orders_update_3", "[json_orders]") {
  auto message = R"({)"
                 R"("channel":"futures.orders",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("amend_text":"-",)"
                 R"("biz_info":"-",)"
                 R"("contract":"SOL_USDT",)"
                 R"("create_time":1727165696,)"
                 R"("create_time_ms":1727165696934,)"
                 R"("fill_price":0,)"
                 R"("finish_as":"_new",)"
                 R"("finish_time":1727165696,)"
                 R"("finish_time_ms":1727165696934,)"
                 R"("iceberg":0,)"
                 R"("id":533305351673,)"
                 R"("is_close":false,)"
                 R"("is_liq":false,)"
                 R"("is_reduce_only":false,)"
                 R"("left":1,)"
                 R"("mkfr":-0.00005,)"
                 R"("price":100,)"
                 R"("refr":0,)"
                 R"("refu":0,)"
                 R"("size":1,)"
                 R"("status":"open",)"
                 R"("stop_loss_price":"",)"
                 R"("stop_profit_price":"",)"
                 R"("stp_act":"-",)"
                 R"("stp_id":"0",)"
                 R"("text":"t-LwICgYu1zRkCAQAAAAAA",)"
                 R"("tif":"gtc",)"
                 R"("tkfr":0.00015,)"
                 R"("update_id":1,)"
                 R"("update_time":1727165696934,)"
                 R"("user":"15564602")"
                 R"(})"
                 R"(],)"
                 R"("time":1727165696,)"
                 R"("time_ms":1727165696936)"
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

// sell 1
TEST_CASE("json_orders_update_4", "[json_orders]") {
  auto message = R"({)"
                 R"("channel":"futures.orders",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("amend_text":"-",)"
                 R"("biz_info":"-",)"
                 R"("contract":"SOL_USDT",)"
                 R"("create_time":1727169518,)"
                 R"("create_time_ms":1727169518087,)"
                 R"("fill_price":147.6,)"
                 R"("finish_as":"filled",)"
                 R"("finish_time":1727169518,)"
                 R"("finish_time_ms":1727169518413,)"
                 R"("iceberg":0,)"
                 R"("id":533324294866,)"
                 R"("is_close":false,)"
                 R"("is_liq":false,)"
                 R"("is_reduce_only":false,)"
                 R"("left":0,)"
                 R"("mkfr":-0.00005,)"
                 R"("price":147.6,)"
                 R"("refr":0,)"
                 R"("refu":0,)"
                 R"("size":-1,)"
                 R"("status":"finished",)"
                 R"("stop_loss_price":"",)"
                 R"("stop_profit_price":"",)"
                 R"("stp_act":"-",)"
                 R"("stp_id":"0",)"
                 R"("text":"t-KAICtn9Y5BkCAQAAAAAA",)"
                 R"("tif":"gtc",)"
                 R"("tkfr":0.00015,)"
                 R"("update_id":2,)"
                 R"("update_time":1727169518413,)"
                 R"("user":"15564602")"
                 R"(})"
                 R"(],)"
                 R"("time":1727169518,)"
                 R"("time_ms":1727169518417)"
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
