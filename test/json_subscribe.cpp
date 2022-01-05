/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/gateio_futures/json/subscribe.h"

using namespace roq;
using namespace roq::gateio_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_subscribe, success) {
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
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Subscribe>(message, buffer_);
}
