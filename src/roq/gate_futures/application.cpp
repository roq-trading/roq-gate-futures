/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/gate_futures/application.hpp"

#include "roq/gate_futures/config.hpp"
#include "roq/gate_futures/gateway.hpp"
#include "roq/gate_futures/settings.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

// === CONSTANTS ===

namespace {
auto const TYPE = server::Type::ORDER_MANAGEMENT;
}  // namespace

// === IMPLEMENTATION ===

int Application::main(int, char **) {
  Settings settings{TYPE};
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace gate_futures
}  // namespace roq
