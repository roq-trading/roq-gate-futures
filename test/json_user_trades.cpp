/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/gate_futures/json/user_trades.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_user_trades_update", "[json_user_trades]") {
  auto message = R"([{)"
                 R"("price":"151.35",)"
                 R"("text":"t-jQICOTizYBsCAQAAAAAA",)"
                 R"("fee":"0.0227025",)"
                 R"("create_time":1727233346.9494,)"
                 R"("point_fee":"0",)"
                 R"("trade_id":"60178895",)"
                 R"("contract":"SOL_USDT",)"
                 R"("role":"taker",)"
                 R"("order_id":"533640269918",)"
                 R"("size":-1,)"
                 R"("close_size":-1,)"
                 R"("biz_info":"-",)"
                 R"("amend_text":"-")"
                 R"(},{)"
                 R"("price":"151.62",)"
                 R"("text":"t-OgICmpo4XRsCAQAAAAAA",)"
                 R"("fee":"0.022743",)"
                 R"("create_time":1727232753.841,)"
                 R"("point_fee":"0",)"
                 R"("trade_id":"60178445",)"
                 R"("contract":"SOL_USDT",)"
                 R"("role":"taker",)"
                 R"("order_id":"533638125442",)"
                 R"("size":1,)"
                 R"("close_size":0,)"
                 R"("biz_info":"-",)"
                 R"("amend_text":"-")"
                 R"(},{)"
                 R"("price":"146.5",)"
                 R"("text":"t-OAICmO8AERoCAQAAAAAA",)"
                 R"("fee":"-0.007325",)"
                 R"("create_time":1727177730.7286,)"
                 R"("point_fee":"0",)"
                 R"("trade_id":"60121891",)"
                 R"("contract":"SOL_USDT",)"
                 R"("role":"maker",)"
                 R"("order_id":"533355978269",)"
                 R"("size":-1,)"
                 R"("close_size":-1,)"
                 R"("biz_info":"-",)"
                 R"("amend_text":"-")"
                 R"(},{)"
                 R"("price":"147.6",)"
                 R"("text":"t-KAICtn9Y5BkCAQAAAAAA",)"
                 R"("fee":"-0.00738",)"
                 R"("create_time":1727169518.4131,)"
                 R"("point_fee":"0",)"
                 R"("trade_id":"60117765",)"
                 R"("contract":"SOL_USDT",)"
                 R"("role":"maker",)"
                 R"("order_id":"533324294866",)"
                 R"("size":-1,)"
                 R"("close_size":-1,)"
                 R"("biz_info":"-",)"
                 R"("amend_text":"-")"
                 R"(},{)"
                 R"("price":"147.76",)"
                 R"("text":"t-DAIC8k8O4hkCAQAAAAAA",)"
                 R"("fee":"0.022164",)"
                 R"("create_time":1727169120.0124,)"
                 R"("point_fee":"0",)"
                 R"("trade_id":"60117508",)"
                 R"("contract":"SOL_USDT",)"
                 R"("role":"taker",)"
                 R"("order_id":"533322588362",)"
                 R"("size":1,)"
                 R"("close_size":0,)"
                 R"("biz_info":"-",)"
                 R"("amend_text":"-")"
                 R"(},{)"
                 R"("price":"148",)"
                 R"("text":"t-sAIC5kyY3xkCAQAAAAAA",)"
                 R"("fee":"-0.0074",)"
                 R"("create_time":1727168891.6293,)"
                 R"("point_fee":"0",)"
                 R"("trade_id":"60117338",)"
                 R"("contract":"SOL_USDT",)"
                 R"("role":"maker",)"
                 R"("order_id":"533321246654",)"
                 R"("size":1,)"
                 R"("close_size":0,)"
                 R"("biz_info":"-",)"
                 R"("amend_text":"-")"
                 R"(})"
                 R"(])"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::UserTrades user_trades{message, buffer};
  REQUIRE(std::size(user_trades.data) == 6);
  auto &result_0 = user_trades.data[0];
  CHECK(result_0.create_time == 1727233346949400us);
}
