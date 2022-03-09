/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gate_futures/api.hpp"

#include "roq/exceptions.hpp"

#include "roq/gate_futures/flags.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

API API::create() {
  auto api = Flags::api();
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
  throw RuntimeError(R"(Unknown api="{}")"sv, api);
}

}  // namespace gate_futures
}  // namespace roq
