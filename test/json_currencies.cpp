/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/gate_futures/json/currencies.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

// note! reduced
TEST_CASE("json_currencies_item", "[json_currencies]") {
  auto message = R"([{)"
                 R"("currency":"AGLD",)"
                 R"("delisted":false,)"
                 R"("withdraw_disabled":false,)"
                 R"("withdraw_delayed":false,)"
                 R"("deposit_disabled":false,)"
                 R"("trade_disabled":false)"
                 R"(},{)"
                 R"("currency":"QANX",)"
                 R"("delisted":false,)"
                 R"("withdraw_disabled":false,)"
                 R"("withdraw_delayed":false,)"
                 R"("deposit_disabled":false,)"
                 R"("trade_disabled":false)"
                 R"(})"
                 R"(])";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Currencies>(message, buffer_);
  auto &data = obj.data;
  REQUIRE(std::size(data) == 2);
}
