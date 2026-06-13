/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "parser_tester.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

using value_type = protocol::json::FuturesSystem;

TEST_CASE("simple", "[json_futures_system]") {
  auto message = R"({)"
                 R"("time":1780992412,)"
                 R"("time_ms":1780992412642,)"
                 R"("channel":"futures.system",)"
                 R"("event":"update",)"
                 R"("result":{)"
                 R"("type":"upgrade",)"
                 R"("msg":"The connection will soon be closed for a service upgrade. Please reconnect.")"
                 R"(})"
                 R"(})";
  auto helper = [](value_type const &obj) {
    CHECK(obj.time == 1780992412s);
    CHECK(obj.channel == protocol::json::Channel::FUTURES_SYSTEM);
    CHECK(obj.result.msg == "The connection will soon be closed for a service upgrade. Please reconnect."sv);
  };
  ParserTester<value_type>::dispatch(helper, message, 8192, 1);
}
