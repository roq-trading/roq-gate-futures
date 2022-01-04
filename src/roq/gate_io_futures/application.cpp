/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gate_io/application.h"

#include "roq/gate_io/config.h"
#include "roq/gate_io/flags.h"
#include "roq/gate_io/gateway.h"

using namespace std::literals;

namespace roq {
namespace gate_io {

int Application::main(int, char **) {
  log::info(R"(Parse config_file="{}")"sv, Flags::config_file());
  Config config(Flags::config_file(), Flags::secrets_file());
  log::info<1>("config={}"sv, config);
  log::info("Starting the gateway"sv);
  roq::server::Trading<Gateway>(ROQ_PACKAGE_NAME, config).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace gate_io
}  // namespace roq
