/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/subscribe.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_subscribe_success", "[json_subscribe]") {
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
  std::vector<std::byte> buffer(8192);
  [[maybe_unused]] auto obj = json::Subscribe::create(message, buffer);
}
