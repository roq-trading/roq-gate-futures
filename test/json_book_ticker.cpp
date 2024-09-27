/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/book_ticker.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_book_ticker_update_1", "[json_book_ticker]") {
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
  std::vector<std::byte> buffer(8192);
  json::BookTicker book_ticker{message, buffer};
  CHECK(book_ticker.time == 1641365392s);
}

TEST_CASE("json_book_ticker_update_2", "[json_book_ticker]") {
  auto message = R"({)"
                 R"("channel":"futures.book_ticker",)"
                 R"("event":"update",)"
                 R"("result":{)"
                 R"("t":1727407594819,)"
                 R"("u":56611084599,)"
                 R"("s":"ETH_USDT",)"
                 R"("b":"2622.7",)"
                 R"("B":255,)"
                 R"("a":"2622.75",)"
                 R"("A":2422)"
                 R"(},)"
                 R"("time":1727407594,)"
                 R"("time_ms":1727407594837)"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  json::BookTicker book_ticker{message, buffer};
  CHECK(book_ticker.time == 1727407594s);
  CHECK(book_ticker.time_ms == 1727407594837ms);
  auto &result = book_ticker.result;
  CHECK(result.timestamp == 1727407594819ms);
}
