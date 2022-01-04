/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gateio_futures/shared.h"

#include "roq/gateio_futures/flags.h"

namespace roq {
namespace gateio_futures {

Shared::Shared(server::Dispatcher &dispatcher)
    : api(API::create()), bids(server::Flags::cache_mbp_max_depth()),
      asks(server::Flags::cache_mbp_max_depth()), final_bids(server::Flags::cache_mbp_max_depth()),
      final_asks(server::Flags::cache_mbp_max_depth()), dispatcher_(dispatcher),
      rate_limiter(Flags::request_limit(), Flags::request_limit_interval()),
      symbols(Flags::ws_max_subscriptions_per_stream()) {
}

}  // namespace gateio_futures
}  // namespace roq
