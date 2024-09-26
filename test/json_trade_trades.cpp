/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/trade_parser.hpp"
#include "roq/gate_futures/json/user_trades.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_trades_update_1", "[json_trades]") {
  auto message = R"({)"
                 R"("channel":"futures.usertrades",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("id":"60117338",)"
                 R"("order_id":"533321246654",)"
                 R"("contract":"SOL_USDT",)"
                 R"("create_time":1727168891,)"
                 R"("create_time_ms":1727168891629,)"
                 R"("size":1,)"
                 R"("role":"maker",)"
                 R"("price":"148",)"
                 R"("text":"t-sAIC5kyY3xkCAQAAAAAA",)"
                 R"("fee":-0.0074,)"
                 R"("point_fee":0,)"
                 R"("amend_text":"-",)"
                 R"("biz_info":"-",)"
                 R"("close_size":"0")"
                 R"(})"
                 R"(],)"
                 R"("time":1727168891,)"
                 R"("time_ms":1727168891630)"
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
      auto &trades = event.value;
      CHECK(trades.time == 1727168891s);
      CHECK(trades.time_ms == 1727168891630ms);
      REQUIRE(std::size(trades.result) == 1);
      auto &result_0 = trades.result[0];
      CHECK(result_0.create_time == 1727168891s);
      CHECK(result_0.create_time_ms == 1727168891629ms);
    }
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

// open 1
TEST_CASE("json_trades_update_2", "[json_trades]") {
  auto message = R"({)"
                 R"("channel":"futures.usertrades",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("id":"60117508",)"
                 R"("order_id":"533322588362",)"
                 R"("contract":"SOL_USDT",)"
                 R"("create_time":1727169120,)"
                 R"("create_time_ms":1727169120012,)"
                 R"("size":1,)"
                 R"("role":"taker",)"
                 R"("price":"147.76",)"
                 R"("text":"t-DAIC8k8O4hkCAQAAAAAA",)"
                 R"("fee":0.022164,)"
                 R"("point_fee":0,)"
                 R"("amend_text":"-",)"
                 R"("biz_info":"-",)"
                 R"("close_size":"0")"
                 R"(})"
                 R"(],)"
                 R"("time":1727169120,)"
                 R"("time_ms":1727169120015)"
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
      auto &trades = event.value;
      CHECK(trades.time == 1727169120s);
      CHECK(trades.time_ms == 1727169120015ms);
      REQUIRE(std::size(trades.result) == 1);
      auto &result_0 = trades.result[0];
      CHECK(result_0.create_time == 1727169120s);
      CHECK(result_0.create_time_ms == 1727169120012ms);
    }
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

// close 1
TEST_CASE("json_trades_update_3", "[json_trades]") {
  auto message = R"({)"
                 R"("channel":"futures.usertrades",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("id":"60117765",)"
                 R"("order_id":"533324294866",)"
                 R"("contract":"SOL_USDT",)"
                 R"("create_time":1727169518,)"
                 R"("create_time_ms":1727169518413,)"
                 R"("size":-1,)"
                 R"("role":"maker",)"
                 R"("price":"147.6",)"
                 R"("text":"t-KAICtn9Y5BkCAQAAAAAA",)"
                 R"("fee":-0.00738,)"
                 R"("point_fee":0,)"
                 R"("amend_text":"-",)"
                 R"("biz_info":"-",)"
                 R"("close_size":"-1")"
                 R"(})"
                 R"(],)"
                 R"("time":1727169518,)"
                 R"("time_ms":1727169518414)"
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
      auto &trades = event.value;
      CHECK(trades.time == 1727169518s);
      CHECK(trades.time_ms == 1727169518414ms);
      REQUIRE(std::size(trades.result) == 1);
      auto &result_0 = trades.result[0];
      CHECK(result_0.create_time == 1727169518s);
      CHECK(result_0.create_time_ms == 1727169518413ms);
    }
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
