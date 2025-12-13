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
uint8_t const API_2 = {};
}

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args};
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>{settings, config, *context, API_2}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace gate_futures
}  // namespace roq
