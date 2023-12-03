/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/gate_futures/shared.hpp"

namespace roq {
namespace gate_futures {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : api{API::create(settings)}, dispatcher_{dispatcher}, settings{settings},
      rate_limiter{settings.common.request_limit, settings.common.request_limit_interval},
      symbols{settings.ws.max_subscriptions_per_stream}, depth_request_queue{settings.ws.mbp_request_delay} {
}

}  // namespace gate_futures
}  // namespace roq
