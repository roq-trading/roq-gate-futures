/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/gate_futures/gateway/api.hpp"

#include "roq/logging.hpp"

#include "roq/utils/enum.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {
namespace gateway {

// === CONSTANTS ===

namespace {
auto API_BTC = API{
    .spot_currencies = "/spot/currencies"sv,
    .futures_contracts = "/futures/btc/contracts"sv,
    .futures_order_book = "/futures/btc/order_book"sv,
    .futures_candlesticks = "/futures/btc/candlesticks"sv,
    .accounts = "/futures/btc/accounts"sv,
    .positions = "/futures/btc/positions"sv,
    .orders = "/futures/btc/orders"sv,
    .trades = "/futures/btc/my_trades_timerange"sv,
};
auto API_USDT = API{
    .spot_currencies = "/spot/currencies"sv,
    .futures_contracts = "/futures/usdt/contracts"sv,
    .futures_order_book = "/futures/usdt/order_book"sv,
    .futures_candlesticks = "/futures/usdt/candlesticks"sv,
    .accounts = "/futures/usdt/accounts"sv,
    .positions = "/futures/usdt/positions"sv,
    .orders = "/futures/usdt/orders"sv,
    .trades = "/futures/usdt/my_trades_timerange"sv,
};
}  // namespace

// === IMPLEMENTATION ===

API API::create(Settings const &settings) {
  auto api = parse_api(settings);
  switch (api) {
    using enum Key;
    case BTC:
      return API_BTC;
    case USDT:
      return API_USDT;
  }
  log::fatal("Unexpected"sv);
}

API::Key API::parse_api(Settings const &settings) {
  std::string tmp{settings.app.api};
  std::replace(tmp.begin(), tmp.end(), '-', '_');
  return utils::parse_enum<Key>(tmp);
}

}  // namespace gateway
}  // namespace gate_futures
}  // namespace roq
