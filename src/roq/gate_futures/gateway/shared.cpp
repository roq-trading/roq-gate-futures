/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/gate_futures/gateway/shared.hpp"

namespace roq {
namespace gate_futures {
namespace gateway {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : dispatcher{dispatcher}, settings{settings}, api{API::create(settings)}, throttle{settings},
      rate_limiter{settings.misc.request_limit, settings.misc.request_limit_interval}, depth_request_queue{rate_limiter, settings.ws.mbp_request_delay},
      time_series_request_queue{rate_limiter}, symbols{settings.ws.max_subscriptions_per_stream} {
}

}  // namespace gateway
}  // namespace gate_futures
}  // namespace roq
