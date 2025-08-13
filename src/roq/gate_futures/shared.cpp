/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/gate_futures/shared.hpp"

#include "roq/utils/common.hpp"

namespace roq {
namespace gate_futures {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : api{API::create(settings)}, dispatcher_{dispatcher}, settings{settings}, settings_time_series_interval{utils::to_interval(settings.time_series.interval)},
      rate_limiter{settings.misc.request_limit, settings.misc.request_limit_interval}, symbols{settings.ws.max_subscriptions_per_stream},
      depth_request_queue{settings.ws.mbp_request_delay} {
}

}  // namespace gate_futures
}  // namespace roq
