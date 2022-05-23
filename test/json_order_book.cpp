/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/gate_futures/json/order_book.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_order_book_simple", "[json_order_book]") {
  auto message = R"({)"
                 R"("current":1643191515.447,)"
                 R"("asks":[)"
                 R"({"s":117293,"p":"37705.6"},)"
                 R"({"s":20,"p":"37707.9"},)"
                 R"({"s":79,"p":"37708"},)"
                 R"({"s":2,"p":"37708.4"},)"
                 R"({"s":1325,"p":"37708.6"},)"
                 R"({"s":4536,"p":"37709"},)"
                 R"({"s":1285,"p":"37709.2"},)"
                 R"({"s":9,"p":"37709.5"},)"
                 R"({"s":11109,"p":"37711"},)"
                 R"({"s":2,"p":"37712.2"})"
                 R"(],)"
                 R"("bids":[)"
                 R"({"s":72890,"p":"37705.5"},)"
                 R"({"s":1,"p":"37704.9"},)"
                 R"({"s":7950,"p":"37704.4"},)"
                 R"({"s":517,"p":"37700.5"},)"
                 R"({"s":1744,"p":"37698.9"},)"
                 R"({"s":1325,"p":"37696.5"},)"
                 R"({"s":3073,"p":"37696.4"},)"
                 R"({"s":6,"p":"37696.3"},)"
                 R"({"s":1590,"p":"37695.9"},)"
                 R"({"s":2524,"p":"37695.8"})"
                 R"(],)"
                 R"("id":11144476177,)"
                 R"("update":1643191515.446)"
                 R"(})"sv;
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::OrderBook>(message, buffer_);
  CHECK(obj.current == 1643191515.447_a);
  REQUIRE(std::size(obj.asks) == 10);
  auto &a0 = obj.asks[0];
  CHECK(a0.size == 117293.0_a);
  CHECK(a0.price == 37705.6_a);
  REQUIRE(std::size(obj.bids) == 10);
  auto &b0 = obj.bids[0];
  CHECK(b0.size == 72890.0_a);
  CHECK(b0.price == 37705.5_a);
  CHECK(obj.id == 11144476177);
  CHECK(obj.update == 1643191515.446_a);
}
