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

// TIF ==> roq::TimeInForce

template <>
template <>
constexpr Helper<TIF>::operator roq::TimeInForce() {
  switch (std::get<0>(args_)) {
    using enum json::TIF::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case GTC:
      return roq::TimeInForce::GTC;
    case FOK:
      return roq::TimeInForce::FOK;
    case POC:
      // return roq::TimeInForce::POC;
      break;
    case IOC:
      return roq::TimeInForce::IOC;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::TimeInForce>(Helper{TIF{TIF::UNDEFINED__}}) == roq::TimeInForce::UNDEFINED);
static_assert(static_cast<roq::TimeInForce>(Helper{TIF{TIF::GTC}}) == roq::TimeInForce::GTC);
static_assert(static_cast<roq::TimeInForce>(Helper{TIF{TIF::FOK}}) == roq::TimeInForce::FOK);
// static_assert(static_cast<roq::TimeInForce>(Helper{TIF{TIF::POC}}) == roq::TimeInForce::POC);
static_assert(static_cast<roq::TimeInForce>(Helper{TIF{TIF::IOC}}) == roq::TimeInForce::IOC);

// OrderStatus ==> roq::OrderStatus

template <>
template <>
constexpr Helper<OrderStatus>::operator roq::OrderStatus() {
  switch (std::get<0>(args_)) {
    using enum json::OrderStatus::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case OPEN:
      return roq::OrderStatus::WORKING;
    case FINISHED:
      return roq::OrderStatus::COMPLETED;  // XXX canceled?
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::OrderStatus>(Helper{OrderStatus{OrderStatus::UNDEFINED__}}) == roq::OrderStatus::UNDEFINED);
static_assert(static_cast<roq::OrderStatus>(Helper{OrderStatus{OrderStatus::OPEN}}) == roq::OrderStatus::WORKING);
static_assert(static_cast<roq::OrderStatus>(Helper{OrderStatus{OrderStatus::FINISHED}}) == roq::OrderStatus::COMPLETED);

// roq ==>
}  // namespace

// === IMPLEMENTATION ===

// ==> roq

template <>
template <>
Map<TIF>::operator roq::TimeInForce() {
  return Helper{args_};
}

template <>
template <>
Map<OrderStatus>::operator roq::OrderStatus() {
  return Helper{args_};
}

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
