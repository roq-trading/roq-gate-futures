/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gate_io_futures/application.h"

#include "roq/gate_io_futures/config.h"
#include "roq/gate_io_futures/flags.h"
#include "roq/gate_io_futures/gateway.h"

using namespace std::literals;

namespace roq {
namespace gate_io_futures {

int Application::main(int, char **) {
  log::info(R"(Parse config_file="{}")"sv, Flags::config_file());
  Config config(Flags::config_file(), Flags::secrets_file());
  log::info<1>("config={}"sv, config);
  log::info("Starting the gateway"sv);
  roq::server::Trading<Gateway>(ROQ_PACKAGE_NAME, config).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace gate_io_futures
}  // namespace roq
