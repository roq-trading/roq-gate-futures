/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gate_io_futures/api.h"

#include "roq/exceptions.h"

#include "roq/gate_io_futures/flags.h"

using namespace std::literals;

namespace roq {
namespace gate_io_futures {

API API::create() {
  auto api = Flags::api();
  if (api.compare("btc"sv) == 0) {
    return {
        .get_contracts = "/futures/btc/contracts"sv,
    };
  }
  if (api.compare("usdt"sv) == 0) {
    return {
        .get_contracts = "/futures/usdt/contracts"sv,
    };
  }
  throw RuntimeErrorException(R"(Unknown api="{}")"sv, api);
}

}  // namespace gate_io_futures
}  // namespace roq
