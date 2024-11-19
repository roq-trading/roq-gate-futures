/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/order_book_update.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_order_book_update_simple_1", "[json_order_book_update]") {
  auto message = R"({)"
                 R"("id":null,)"
                 R"("time":1643180626,)"
                 R"("channel":"futures.order_book_update",)"
                 R"("event":"update",)"
                 R"("error":null,)"
                 R"("result":{)"
                 R"("t":1643180626827,)"
                 R"("s":"BTC_USDT",)"
                 R"("U":11140379005,)"
                 R"("u":11140379319,)"
                 R"("b":[)"
                 R"({"p":"37203.3","s":1613},)"
                 R"({"p":"37205.2","s":0},)"
                 R"({"p":"37207.1","s":0},)"
                 R"({"p":"37207.6","s":66172},)"
                 R"({"p":"37210","s":0},)"
                 R"({"p":"37210.1","s":249181},)"
                 R"({"p":"37206.7","s":1343},)"
                 R"({"p":"37209.7","s":1343},)"
                 R"({"p":"37209.9","s":200})"
                 R"(],)"
                 R"("a":[)"
                 R"({"p":"37210.2","s":206362},)"
                 R"({"p":"37216.2","s":0},)"
                 R"({"p":"37217.5","s":0},)"
                 R"({"p":"37218.8","s":0},)"
                 R"({"p":"37219","s":0},)"
                 R"({"p":"37211.1","s":1343},)"
                 R"({"p":"37211.5","s":2152},)"
                 R"({"p":"37214.1","s":1965},)"
                 R"({"p":"37216.7","s":622})"
                 R"(])"
                 R"(})"
                 R"(})"sv;
  std::vector<std::byte> buffer(8192);
  json::OrderBookUpdate order_book_update{message, buffer};
  CHECK(order_book_update.time == 1643180626s);
  CHECK(order_book_update.channel == json::Channel::ORDER_BOOK_UPDATE);
  CHECK(order_book_update.event == json::Event::UPDATE);
  auto &result = order_book_update.result;
  CHECK(result.timestamp == 1643180626827ms);
  CHECK(result.symbol == "BTC_USDT"sv);
  CHECK(result.first_update_id == 11140379005);
  CHECK(result.last_update_id == 11140379319);
  REQUIRE(std::size(result.bids) == 9);
  auto &b0 = result.bids[0];
  CHECK(b0.price == 37203.3_a);
  CHECK(b0.size == 1613.0_a);
  REQUIRE(std::size(result.asks) == 9);
  auto &a0 = result.asks[0];
  CHECK(a0.price == 37210.2_a);
  CHECK(a0.size == 206362.0_a);
}

TEST_CASE("json_order_book_update_simple_2", "[json_order_book_update]") {
  auto message = R"({)"
                 R"("id":null,)"
                 R"("time":1643180627,)"
                 R"("channel":"futures.order_book_update",)"
                 R"("event":"update",)"
                 R"("error":null,)"
                 R"("result":{)"
                 R"("t":1643180627828,)"
                 R"("s":"BTC_USDT",)"
                 R"("U":11140379320,)"
                 R"("u":11140379509,)"
                 R"("b":[)"
                 R"({"p":"37207.6","s":66016},)"
                 R"({"p":"37209.9","s":0},)"
                 R"({"p":"37210.1","s":164775},)"
                 R"({"p":"37205.9","s":2547})"
                 R"(],)"
                 R"("a":[)"
                 R"({"p":"37210.2","s":227146})"
                 R"(])"
                 R"(})"
                 R"(})"sv;
  std::vector<std::byte> buffer(8192);
  json::OrderBookUpdate order_book_update{message, buffer};
  CHECK(order_book_update.time == 1643180627s);
  CHECK(order_book_update.channel == json::Channel::ORDER_BOOK_UPDATE);
  CHECK(order_book_update.event == json::Event::UPDATE);
  auto &result = order_book_update.result;
  CHECK(result.timestamp == 1643180627828ms);
  CHECK(result.symbol == "BTC_USDT"sv);
  CHECK(result.first_update_id == 11140379320);
  CHECK(result.last_update_id == 11140379509);
  REQUIRE(std::size(result.bids) == 4);
  auto &b0 = result.bids[0];
  CHECK(b0.price == 37207.6_a);
  CHECK(b0.size == 66016.0_a);
  REQUIRE(std::size(result.asks) == 1);
  auto &a0 = result.asks[0];
  CHECK(a0.price == 37210.2_a);
  CHECK(a0.size == 227146.0_a);
}

TEST_CASE("json_order_book_update_simple_3", "[json_order_book_update]") {
  auto message = R"({)"
                 R"("channel":"futures.order_book_update",)"
                 R"("event":"update",)"
                 R"("result":{)"
                 R"("U":23526668009,)"
                 R"("a":[],)"
                 R"("b":[{)"
                 R"("p":"154.39",)"
                 R"("s":1545)"
                 R"(},{)"
                 R"("p":"154.38",)"
                 R"("s":817)"
                 R"(}],)"
                 R"("s":"SOL_USDT",)"
                 R"("t":1727407188004,)"
                 R"("u":23526668010},)"
                 R"("time":1727407188,)"
                 R"("time_ms":1727407188072)"
                 R"(})"sv;
  std::vector<std::byte> buffer(8192);
  json::OrderBookUpdate order_book_update{message, buffer};
  CHECK(order_book_update.time == 1727407188s);
  CHECK(order_book_update.time_ms == 1727407188072ms);
  CHECK(order_book_update.channel == json::Channel::ORDER_BOOK_UPDATE);
  CHECK(order_book_update.event == json::Event::UPDATE);
  auto &result = order_book_update.result;
  CHECK(result.timestamp == 1727407188004ms);
  CHECK(result.symbol == "SOL_USDT"sv);
  CHECK(result.first_update_id == 23526668009);
  CHECK(result.last_update_id == 23526668010);
  REQUIRE(std::size(result.bids) == 2);
  auto &b0 = result.bids[0];
  CHECK(b0.price == 154.39_a);
  CHECK(b0.size == 1545_a);
  auto &b1 = result.bids[1];
  CHECK(b1.price == 154.38_a);
  CHECK(b1.size == 817_a);
  REQUIRE(std::size(result.asks) == 0);
}
