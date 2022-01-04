/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <chrono>
#include <string>
#include <utility>

#include "roq/api.h"
#include "roq/server.h"

#include "roq/core/memory.h"
#include "roq/core/symbols.h"

#include "roq/core/limit/rate_limiter.h"

#include "roq/gateio_futures/api.h"

namespace roq {
namespace gateio_futures {

struct Shared final {
  explicit Shared(server::Dispatcher &);

  Shared(Shared &&) = default;
  Shared(const Shared &) = delete;

  auto discard_symbol(const std::string_view &name) const {
    return dispatcher_.discard_symbol(name);
  }

  template <typename... Args>
  auto update_order(Args &&...args) {
    return dispatcher_.update_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto create_order(Args &&...args) {
    return dispatcher_.create_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

 public:
  const API api;
  core::page_aligned_vector<MBPUpdate> bids, asks, final_bids, final_asks;

 private:
  server::Dispatcher &dispatcher_;

 public:
  core::limit::RateLimiter rate_limiter;
  core::Symbols symbols;
};

}  // namespace gateio_futures
}  // namespace roq
