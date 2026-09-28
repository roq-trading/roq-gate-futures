/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/gate_futures/tools/throttle.hpp"

#include "roq/utils/compare.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/hash/fnv.hpp"

#include "roq/utils/charconv/from_chars.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {
namespace tools {

// === CONSTANTS ===

namespace {
auto const DEFAULT_BACKOFF = 5s;     // note! maybe as low as 1 second
auto const BLOCKED_BACKOFF = 10min;  // note! very serious
}  // namespace

// === HELPERS ===

namespace {
// note! std::tolower is not constexpr gcc16 + clang23
constexpr auto lower(auto value) {
  return utils::detail::ascii_to_lower(value);
}

enum class Header {
  UNKNOWN,
  X_GATE_RATELIMIT_LIMIT,
  X_GATE_RATELIMIT_REQUESTS_REMAIN,
  X_GATE_RATELIMIT_RESET_TIMESTAMP,
};

constexpr auto parse_header(std::string_view const &text) {
  std::string value;
  value.reserve(std::size(text));
  std::transform(std::begin(text), std::end(text), std::back_inserter(value), [](auto c) { return lower(c); });
  auto key = utils::hash::FNV::compute(value);
  switch (key) {
    case utils::hash::FNV::compute("x-gate-ratelimit-limit"sv):
      return Header::X_GATE_RATELIMIT_LIMIT;
    case utils::hash::FNV::compute("x-gate-ratelimit-requests-remain"sv):
      return Header::X_GATE_RATELIMIT_REQUESTS_REMAIN;
    case utils::hash::FNV::compute("x-gate-ratelimit-reset-timestamp"sv):
      return Header::X_GATE_RATELIMIT_RESET_TIMESTAMP;
  }
  return Header::UNKNOWN;
}

static_assert(parse_header("X-Gate-RateLimit-Limit"sv) == Header::X_GATE_RATELIMIT_LIMIT);
static_assert(parse_header("X-Gate-RateLimit-Requests-Remain"sv) == Header::X_GATE_RATELIMIT_REQUESTS_REMAIN);
static_assert(parse_header("X-Gate-RateLimit-Reset-Timestamp"sv) == Header::X_GATE_RATELIMIT_RESET_TIMESTAMP);
}  // namespace

// === IMPLEMENTATION ===

Throttle::Throttle(server::Settings const &settings) : enabled_{settings.experimental.enable_rate_limit} {
}

// web::rest::Interceptor

void Throttle::operator()(Trace<web::rest::MessageBegin> const &) {
}

void Throttle::operator()(Trace<web::rest::MessageHeader> const &event) {
  auto &[trace_info, header] = event;
  auto update_value = [&](auto &result) {
    using value_type = std::remove_cvref_t<decltype(result)>;
    auto value = utils::charconv::from_chars<value_type>(header.value);
    return utils::update(result, value);
  };
  auto update_suspend_until = [&]() {
    if (!enabled_) {
      return;
    }
    if (params_.requests_remain > 0 || params_.reset_timestamp == 0) {
      suspend_until_ = {};
    } else {
      auto now = clock::get_system();
      auto now_utc = clock::get_realtime();
      auto timestamp = std::chrono::seconds{params_.reset_timestamp};
      if (now_utc < timestamp) {
        auto period = timestamp - now_utc;
        suspend_until_ = std::max(suspend_until_, now + period);
      } else {
        suspend_until_ = std::max(suspend_until_, now + DEFAULT_BACKOFF);
      }
    }
  };
  auto key = parse_header(header.name);
  switch (key) {
    using enum Header;
    [[likely]] case UNKNOWN:
      return;
    case X_GATE_RATELIMIT_LIMIT:
      update_value(params_.limit);
      break;
    case X_GATE_RATELIMIT_REQUESTS_REMAIN:
      if (update_value(params_.requests_remain)) {
        update_suspend_until();
      }
      break;
    case X_GATE_RATELIMIT_RESET_TIMESTAMP:
      if (update_value(params_.reset_timestamp)) {
        update_suspend_until();
      }
      break;
  }
}

void Throttle::operator()(Trace<web::rest::MessageEnd> const &event) {
  auto &[trace_info, message_end] = event;
  if (!enabled_) {
    return;
  }
  switch (message_end.status) {
    using enum web::http::Status;
    [[unlikely]] case FORBIDDEN: {  // 403
      auto now = clock::get_system();
      suspend_until_ = std::max(suspend_until_, now + BLOCKED_BACKOFF);
      break;
    }
    [[unlikely]] case TOO_MANY_REQUESTS: {  // 429
      if (suspend_until_.count() == 0) {
        auto now = clock::get_system();
        suspend_until_ = now + DEFAULT_BACKOFF;
      }
      break;
    }
    default:
      break;
  }
}

// web::socket::Interceptor

}  // namespace tools
}  // namespace gate_futures
}  // namespace roq
