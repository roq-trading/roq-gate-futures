/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/gate_futures/protocol/json/positions.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_positions", "[json_positions]") {
  auto message = R"([)"
                 R"({)"
                 R"("value":"0",)"
                 R"("leverage":"0",)"
                 R"("mode":"single",)"
                 R"("realised_point":"0",)"
                 R"("contract":"SOL_USDT",)"
                 R"("entry_price":"0",)"
                 R"("mark_price":"143.39",)"
                 R"("history_point":"0",)"
                 R"("realised_pnl":"0",)"
                 R"("close_order":null,)"
                 R"("size":0,)"
                 R"("cross_leverage_limit":"10",)"
                 R"("pending_orders":1,)"
                 R"("adl_ranking":6,)"
                 R"("maintenance_rate":"0.01",)"
                 R"("unrealised_pnl":"0",)"
                 R"("pnl_pnl":"0",)"
                 R"("pnl_fee":"0",)"
                 R"("pnl_fund":"0",)"
                 R"("user":15564602,)"
                 R"("leverage_max":"50",)"
                 R"("history_pnl":"0",)"
                 R"("risk_limit":"500000",)"
                 R"("margin":"0",)"
                 R"("last_close_pnl":"0",)"
                 R"("liq_price":"0",)"
                 R"("update_time":1727084914,)"
                 R"("update_id":0,)"
                 R"("initial_margin":"0",)"
                 R"("maintenance_margin":"0",)"
                 R"("open_time":0,)"
                 R"("trade_max_size":"0")"
                 R"(})"
                 R"(])"sv;
  core::json::BufferStack buffer{8192, 1};
  protocol::json::Positions positions{message, buffer};
  REQUIRE(std::size(positions.data) == 1);
  auto &result_0 = positions.data[0];
  CHECK(result_0.update_time == 1727084914s);
}
