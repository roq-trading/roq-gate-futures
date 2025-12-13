/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/utils/container.hpp"

#include "roq/core/symbols.hpp"
#include "roq/core/timer_queue.hpp"

#include "roq/core/limit/rate_limiter.hpp"

#include "roq/market/mbp/sequencer.hpp"

#include "roq/gate_futures/api.hpp"
#include "roq/gate_futures/settings.hpp"

namespace roq {
namespace gate_futures {

struct Shared final {
  Shared(server::Dispatcher &, Settings const &);

  Shared(Shared const &) = delete;

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }

  template <typename... Args>
  auto update_order(Args &&...args) {
    return dispatcher_.update_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

  template <typename Callback>
  bool get_all_order_symbols(Callback callback, std::string_view const &account) const {
    return dispatcher_.get_all_order_symbols(callback, account);
  }

  template <typename... Args>
  auto get_ref_data(Args &&...args) {
    return dispatcher_.get_ref_data(std::forward<Args>(args)...);
  }

 public:
  API const api;

 private:
  struct {
    std::vector<MBPUpdate> bids, asks;
    auto &clear() {
      bids.clear();
      asks.clear();
      return *this;
    }
    bool empty() const { return std::empty(bids) && std::empty(asks); }
  } mbp;
  std::vector<Trade> trades;

 public:
  auto &get_mbp() { return mbp.clear(); }

  auto &get_trades() {
    trades.clear();
    return trades;
  }

  utils::unordered_map<std::string, market::mbp::Sequencer> mbp_sequencer;

 private:
  server::Dispatcher &dispatcher_;

 public:
  Settings const &settings;
  core::limit::RateLimiter rate_limiter;
  core::Symbols symbols;
  core::TimerQueue<std::string> depth_request_queue;
  core::TimerQueue<std::string> time_series_request_queue;

  std::vector<Bar> bars;
};

}  // namespace gate_futures
}  // namespace roq
