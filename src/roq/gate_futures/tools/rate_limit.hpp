/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>

#include <fmt/format.h>

#include "roq/web/rest/interceptor.hpp"

#include "roq/web/socket/interceptor.hpp"

#include "roq/gate_futures/flags/settings.hpp"

namespace roq {
namespace gate_futures {
namespace tools {

struct RateLimit final : public web::rest::Interceptor, public web::socket::Interceptor {
  explicit RateLimit(flags::Settings const &);

  struct Params {
    int32_t limit = {};
    int32_t requests_remain = {};
    int64_t reset_timestamp = {};  // sec
  };

 protected:
  // web::Interceptor

  operator std::chrono::nanoseconds() const override { return suspend_until_; }

  // web::rest::Interceptor

  void operator()(Trace<web::rest::MessageBegin> const &) override;
  void operator()(Trace<web::rest::MessageHeader> const &) override;
  void operator()(Trace<web::rest::MessageEnd> const &) override;

  // web::socket::Interceptor

 private:
  bool const suspend_on_rate_limit_;

  Params params_;

  std::chrono::nanoseconds suspend_until_ = {};
};

}  // namespace tools
}  // namespace gate_futures
}  // namespace roq

template <>
struct fmt::formatter<roq::gate_futures::tools::RateLimit::Params> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::gate_futures::tools::RateLimit::Params const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(limit={}, )"
        R"(requests_remain={}, )"
        R"(reset_timestamp={})"
        R"(}})"sv,
        value.limit,
        value.requests_remain,
        value.reset_timestamp);
  }
};
