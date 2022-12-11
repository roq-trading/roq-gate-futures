/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/gate_futures/application.hpp"

#include "roq/gate_futures/config.hpp"
#include "roq/gate_futures/flags.hpp"
#include "roq/gate_futures/gateway.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

// === CONSTANTS ===

namespace {
auto get_settings = []() {
  return server::Settings{
      .package_name = ROQ_PACKAGE_NAME,
      .build_number = ROQ_BUILD_NUMBER,
      .api = Flags::api(),
      .type = server::Type::ORDER_MANAGEMENT,
  };
};
}  // namespace

// === IMPLEMENTATION ===

int Application::main(int, char **) {
  Config config;
  auto context = server::create_io_context();
  auto settings = get_settings();
  server::Trading<Gateway>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace gate_futures
}  // namespace roq
