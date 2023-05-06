/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/gate_futures/settings.hpp"

#include "roq/gate_futures/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

Settings Settings::create(server::Type type) {
  auto api = flags::Flags::api();
  auto settings = server::create_settings(type, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER, api);
  return {settings};
}

}  // namespace gate_futures
}  // namespace roq
