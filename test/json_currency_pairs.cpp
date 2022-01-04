/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/gate_io/json/currency_pairs.h"

using namespace roq;
using namespace roq::gate_io;

using namespace std::literals;
using namespace std::chrono_literals;

// note! reduced
TEST(json_currency_pairs, item) {
  auto message = R"([{)"
                 R"("id":"IHT_ETH",)"
                 R"("base":"IHT",)"
                 R"("quote":"ETH",)"
                 R"("fee":"0.2",)"
                 R"("min_quote_amount":"0.001",)"
                 R"("amount_precision":1,)"
                 R"("precision":9,)"
                 R"("trade_status":"tradable",)"
                 R"("sell_start":0,)"
                 R"("buy_start":0)"
                 R"(},{)"
                 R"("id":"ALPHR_USDT",)"
                 R"("base":"ALPHR",)"
                 R"("quote":"USDT",)"
                 R"("fee":"0.2",)"
                 R"("min_quote_amount":"1",)"
                 R"("amount_precision":4,)"
                 R"("precision":4,)"
                 R"("trade_status":"tradable",)"
                 R"("sell_start":0,)"
                 R"("buy_start":0)"
                 R"(})"
                 R"(])";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::CurrencyPairs>(message, buffer_);
  auto &data = obj.data;
  ASSERT_EQ(std::size(data), 2);
}
