/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/gate_futures/api.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

// === IMPLEMENTATION ===

API API::create(Settings const &settings) {
  auto api = settings.app.api;
  if (api == "btc"sv) {
    return {
        .spot_currencies = "/spot/currencies"sv,
        .futures_contracts = "/futures/btc/contracts"sv,
        .futures_order_book = "/futures/btc/order_book"sv,
        .futures_candlesticks = "/futures/btc/candlesticks"sv,
        .accounts = "/futures/btc/accounts"sv,
        .positions = "/futures/btc/positions"sv,
        .orders = "/futures/btc/orders"sv,
        .trades = "/futures/btc/my_trades_timerange"sv,
    };
  }
  if (api == "usdt"sv) {
    return {
        .spot_currencies = "/spot/currencies"sv,
        .futures_contracts = "/futures/usdt/contracts"sv,
        .futures_order_book = "/futures/usdt/order_book"sv,
        .futures_candlesticks = "/futures/usdt/candlesticks"sv,
        .accounts = "/futures/usdt/accounts"sv,
        .positions = "/futures/usdt/positions"sv,
        .orders = "/futures/usdt/orders"sv,
        .trades = "/futures/usdt/my_trades_timerange"sv,
    };
  }
  throw RuntimeError{R"(Unknown api="{}")"sv, api};
}

}  // namespace gate_futures
}  // namespace roq
