/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/gate_futures/json/map.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {
namespace json {

// === HELPERS ===

namespace {
// note! constexpr helper for static testing
template <typename... Args>
struct Helper final {
  explicit constexpr Helper(std::tuple<Args...> const &args) : args_{args} {}
  explicit constexpr Helper(Args &&...args_) : args_{std::forward<Args>(args_)...} {}

  template <typename R>
  constexpr operator R();

 private:
  std::tuple<Args...> const args_;
};

// ==> roq

// roq ==>
}  // namespace

// === IMPLEMENTATION ===

// ==> roq

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
