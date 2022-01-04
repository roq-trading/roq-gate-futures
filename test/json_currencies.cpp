/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/gate_io_futures/json/currencies.h"

using namespace roq;
using namespace roq::gate_io_futures;

using namespace std::literals;
using namespace std::chrono_literals;

// note! reduced
TEST(json_currencies, item) {
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
  ASSERT_EQ(std::size(data), 2);
}
