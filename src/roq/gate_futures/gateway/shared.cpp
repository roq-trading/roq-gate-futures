/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/gate_futures/gateway/shared.hpp"

namespace roq {
namespace gate_futures {
namespace gateway {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : api{API::create(settings)}, dispatcher_{dispatcher}, settings{settings}, rate_limiter{settings.misc.request_limit, settings.misc.request_limit_interval},
      symbols{settings.ws.max_subscriptions_per_stream}, depth_request_queue{settings.ws.mbp_request_delay} {
}

}  // namespace gateway
}  // namespace gate_futures
}  // namespace roq
