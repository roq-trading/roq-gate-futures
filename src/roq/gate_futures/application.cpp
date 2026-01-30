/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/gate_futures/application.hpp"

#include "roq/gate_futures/config.hpp"
#include "roq/gate_futures/gateway.hpp"
#include "roq/gate_futures/settings.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

// === CONSTANTS ===

namespace {
uint8_t const API_BTC = 0x0;
uint8_t const API_USDT = 0x1;
}  // namespace

// === HELPERS ===

namespace {
auto parse_api(auto &settings) {
  auto api = API::parse_api(settings);
  switch (api) {
    using enum API::Key;
    case BTC:
      return API_BTC;
    case USDT:
      return API_USDT;
  }
  log::fatal(R"(Unexpected: api="{}")"sv, settings.app.api);
}
}  // namespace

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  auto api = parse_api(settings);
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>{settings, config, *context, api}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace gate_futures
}  // namespace roq
