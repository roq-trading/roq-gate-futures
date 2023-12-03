/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/gate_futures/api.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

// === IMPLEMENTATION ===

API API::create(Settings const &settings) {
  auto api = settings.app.api;
  if (api.compare("btc"sv) == 0) {
    return {
        .get_contracts = "/futures/btc/contracts"sv,
        .get_order_book = "/futures/btc/order_book"sv,
    };
  }
  if (api.compare("usdt"sv) == 0) {
    return {
        .get_contracts = "/futures/usdt/contracts"sv,
        .get_order_book = "/futures/usdt/order_book"sv,
    };
  }
  throw RuntimeError{R"(Unknown api="{}")"sv, api};
}

}  // namespace gate_futures
}  // namespace roq
