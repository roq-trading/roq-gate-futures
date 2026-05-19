/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/compat.hpp"

#include <fmt/format.h>

#include "roq/server/flags/settings.hpp"

#include "roq/gate_futures/flags/flags.hpp"
#include "roq/gate_futures/flags/misc.hpp"
#include "roq/gate_futures/flags/rest.hpp"
#include "roq/gate_futures/flags/ws.hpp"

namespace roq {
namespace gate_futures {
namespace flags {

struct ROQ_PUBLIC Settings final : public server::flags::Settings {
  explicit Settings(args::Parser const &);

  std::string_view exchange;

  flags::Misc misc;
  flags::REST rest;
  flags::WS ws;

 private:
  Settings(args::Parser const &, flags::Flags const &);
};

}  // namespace flags
}  // namespace gate_futures
}  // namespace roq

template <>
struct fmt::formatter<roq::gate_futures::flags::Settings> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::gate_futures::flags::Settings const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(exchange="{}", )"
        R"(misc={}, )"
        R"(rest={}, )"
        R"(ws={}, )"
        R"(server={})"
        R"(}})"sv,
        value.exchange,
        value.misc,
        value.rest,
        value.ws,
        static_cast<roq::server::Settings const &>(value));
  }
};
