/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/gate_futures/json/currencies_ack.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = json::CurrenciesAck;

// note! reduced
TEST_CASE("simple", "[json_currencies_ack]") {
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
  auto helper = [&](value_type &obj) {
    REQUIRE(std::size(obj.data) == 2);
    REQUIRE(obj.data[0].currency == "AGLD"sv);
  };
  core::json::BufferStack buffers{8192, 1};
  value_type obj{message, buffers};
  helper(obj);
}
