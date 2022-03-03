/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch.hpp>

#include "roq/core/json/parser.h"

#include "roq/gate_futures/json/book_ticker.h"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_book_ticker_update", "json_book_ticker") {
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
  auto obj = core::json::Parser::create<json::BookTicker>(message, buffer_);
}
