/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/gate_futures/application.hpp"

#include "roq/gate_futures/flags/settings.hpp"

#include "roq/gate_futures/gateway/config.hpp"
#include "roq/gate_futures/gateway/controller.hpp"

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
  auto api = gateway::API::parse_api(settings);
  switch (api) {
    using enum gateway::API::Key;
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
  flags::Settings settings{args};
  auto api = parse_api(settings);
  gateway::Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading2<gateway::Controller>{settings, config, *context, api}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace gate_futures
}  // namespace roq
