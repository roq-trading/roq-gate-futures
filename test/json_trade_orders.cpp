/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "trade_parser_tester.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::TradeOrders;

TEST_CASE("update_1", "[json_orders]") {
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.time == 1727094610s);
    CHECK(obj.time_ms == 1727094610077ms);
    REQUIRE(std::size(obj.result) == 1);
    auto &result_0 = obj.result[0];
    CHECK(result_0.create_time == 1727084914s);
    CHECK(result_0.create_time_ms == 1727084914383ms);
    CHECK(result_0.finish_time == 1727094610s);
    CHECK(result_0.finish_time_ms == 1727094610074ms);
    CHECK(result_0.update_time == 1727094610074ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

TEST_CASE("update_2", "[json_orders]") {
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.time == 1727164365s);
    CHECK(obj.time_ms == 1727164365252ms);
    REQUIRE(std::size(obj.result) == 1);
    auto &result_0 = obj.result[0];
    CHECK(result_0.create_time == 1727164283s);
    CHECK(result_0.create_time_ms == 1727164283948ms);
    CHECK(result_0.finish_time == 1727164365s);
    CHECK(result_0.finish_time_ms == 1727164365250ms);
    CHECK(result_0.update_time == 1727164365250ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

// _new
TEST_CASE("update_3", "[json_orders]") {
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.time == 1727165696s);
    CHECK(obj.time_ms == 1727165696936ms);
    REQUIRE(std::size(obj.result) == 1);
    auto &result_0 = obj.result[0];
    CHECK(result_0.create_time == 1727165696s);
    CHECK(result_0.create_time_ms == 1727165696934ms);
    CHECK(result_0.finish_time == 1727165696s);
    CHECK(result_0.finish_time_ms == 1727165696934ms);
    CHECK(result_0.update_time == 1727165696934ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}

// sell 1
TEST_CASE("update_4", "[json_orders]") {
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
  auto helper = [](value_type const &obj) {
    CHECK(obj.time == 1727169518s);
    CHECK(obj.time_ms == 1727169518417ms);
    REQUIRE(std::size(obj.result) == 1);
    auto &result_0 = obj.result[0];
    CHECK(result_0.create_time == 1727169518s);
    CHECK(result_0.create_time_ms == 1727169518087ms);
    CHECK(result_0.finish_time == 1727169518s);
    CHECK(result_0.finish_time_ms == 1727169518413ms);
    CHECK(result_0.update_time == 1727169518413ms);
  };
  TradeParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
