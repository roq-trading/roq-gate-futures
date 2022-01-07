/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/gate_futures/json/message.h"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_message, subscribe_success) {
  auto message = R"({)"
                 R"("id":null,)"
                 R"("time":1641365392,)"
                 R"("channel":"futures.tickers",)"
                 R"("event":"subscribe",)"
                 R"("error":null,)"
                 R"("result":{)"
                 R"("status":"success")"
                 R"(})"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Message>(message, buffer_);
}

TEST(json_message, book_ticker) {
  auto message = R"({)"
                 R"("id":null,)"
                 R"("time":1641365392,)"
                 R"("channel":"futures.book_ticker",)"
                 R"("event":"update",)"
                 R"("error":null,)"
                 R"("result":{)"
                 R"("t":1641365392861,)"
                 R"("u":3353442643,)"
                 R"("s":"BTC_USD",)"
                 R"("b":"46400.3",)"
                 R"("B":1044,)"
                 R"("a":"46409.6",)"
                 R"("A":90229)"
                 R"(})"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Message>(message, buffer_);
}

TEST(json_message, tickers) {
  auto message = R"({)"
                 R"("id":null,)"
                 R"("time":1641365849,)"
                 R"("channel":"futures.tickers",)"
                 R"("event":"update",)"
                 R"("error":null,)"
                 R"("result":[{)"
                 R"("contract":"ADA_USD",)"
                 R"("last":"1.23821",)"
                 R"("change_percentage":"0",)"
                 R"("funding_rate":"0.0001",)"
                 R"("mark_price":"1.34475",)"
                 R"("index_price":"1.344728",)"
                 R"("total_size":"21",)"
                 R"("volume_24h":"0",)"
                 R"("quanto_base_rate":"2.895e-05",)"
                 R"("volume_24h_usd":"0",)"
                 R"("volume_24h_btc":"0",)"
                 R"("funding_rate_indicative":"0.0001",)"
                 R"("volume_24h_quote":"0",)"
                 R"("volume_24h_settle":"0",)"
                 R"("volume_24h_base":"0")"
                 R"(})"
                 R"(])"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Message>(message, buffer_);
}

TEST(json_message, trades) {
  auto message = R"({)"
                 R"("id":null,)"
                 R"("time":1641366055,)"
                 R"("channel":"futures.trades",)"
                 R"("event":"update",)"
                 R"("error":null,)"
                 R"("result":[{)"
                 R"("size":-913,)"
                 R"("id":638643,)"
                 R"("create_time":1641366055,)"
                 R"("create_time_ms":1641366055959,)"
                 R"("price":"98.8",)"
                 R"("contract":"XCH_USDT")"
                 R"(})"
                 R"(])"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Message>(message, buffer_);
}
