/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/tickers.hpp"

using namespace roq;
using namespace roq::gate_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_tickers_update_1", "[json_tickers]") {
  auto message = R"({)"
                 R"("id":null,)"
                 R"("time":1641365849,)"
                 R"("channel":"futures.tickers",)"
                 R"("event":"update",)"
                 R"("error":null,)"
                 R"("result":[{)"
                 R"("contract":"ADA_USD",)"
                 R"("last":"1.23821",)"
                 R"("change_percentage":"0",)"
                 R"("funding_rate":"0.0001",)"
                 R"("mark_price":"1.34475",)"
                 R"("index_price":"1.344728",)"
                 R"("total_size":"21",)"
                 R"("volume_24h":"0",)"
                 R"("quanto_base_rate":"2.895e-05",)"
                 R"("volume_24h_usd":"0",)"
                 R"("volume_24h_btc":"0",)"
                 R"("funding_rate_indicative":"0.0001",)"
                 R"("volume_24h_quote":"0",)"
                 R"("volume_24h_settle":"0",)"
                 R"("volume_24h_base":"0")"
                 R"(})"
                 R"(])"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  json::Tickers tickers{message, buffer};
  CHECK(tickers.time == 1641365849s);
}

TEST_CASE("json_tickers_update_2", "[json_tickers]") {
  auto message = R"({)"
                 R"("time":1727407596,)"
                 R"("time_ms":1727407596030,)"
                 R"("channel":"futures.tickers",)"
                 R"("event":"update",)"
                 R"("result":[{)"
                 R"("contract":"BTC_USDT",)"
                 R"("last":"64957",)"
                 R"("change_percentage":"2.3454",)"
                 R"("total_size":"224194099",)"
                 R"("volume_24h":"187379287",)"
                 R"("volume_24h_base":"18737",)"
                 R"("volume_24h_quote":"1216635001",)"
                 R"("volume_24h_settle":"1216635001",)"
                 R"("mark_price":"64954.28",)"
                 R"("funding_rate":"0.000092",)"
                 R"("funding_rate_indicative":"0.000092",)"
                 R"("index_price":"64977.59",)"
                 R"("quanto_base_rate":"",)"
                 R"("low_24h":"63300.1",)"
                 R"("high_24h":"65831.2",)"
                 R"("price_type":"last",)"
                 R"("change_from":"24h")"
                 R"(})"
                 R"(])"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  json::Tickers tickers{message, buffer};
  CHECK(tickers.time == 1727407596s);
  CHECK(tickers.time_ms == 1727407596030ms);
}
