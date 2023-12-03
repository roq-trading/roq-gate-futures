/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/book_ticker.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_book_ticker_update", "[json_book_ticker]") {
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
  [[maybe_unused]] auto obj = json::BookTicker::create(message, buffer);
}
