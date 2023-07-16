/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/gate_futures/settings.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

Settings::Settings(args::Parser const &args) : Settings{args, flags::Flags::create()} {
}

Settings::Settings(args::Parser const &args, flags::Flags const &flags)
    : server::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER, flags.api}, exchange{flags.exchange},
      common{flags::Common::create()}, rest{flags::REST::create()}, ws{flags::WS::create()} {
  log::debug("settings={}"sv, *this);
}

}  // namespace gate_futures
}  // namespace roq
