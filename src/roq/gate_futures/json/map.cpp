/* Copyright (c) 2017-2025, Hans Erik Thrane */

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
      return roq::TimeInForce::GTC;  // note!
    case IOC:
      return roq::TimeInForce::IOC;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::TimeInForce>(Helper{TIF{TIF::UNDEFINED__}}) == roq::TimeInForce::UNDEFINED);
static_assert(static_cast<roq::TimeInForce>(Helper{TIF{TIF::GTC}}) == roq::TimeInForce::GTC);
static_assert(static_cast<roq::TimeInForce>(Helper{TIF{TIF::FOK}}) == roq::TimeInForce::FOK);
static_assert(static_cast<roq::TimeInForce>(Helper{TIF{TIF::POC}}) == roq::TimeInForce::GTC);
static_assert(static_cast<roq::TimeInForce>(Helper{TIF{TIF::IOC}}) == roq::TimeInForce::IOC);

// FinishAs ==> roq::OrderStatus

template <>
template <>
constexpr Helper<FinishAs>::operator roq::OrderStatus() {
  switch (std::get<0>(args_)) {
    using enum json::FinishAs::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case FILLED:
      return roq::OrderStatus::COMPLETED;
    case CANCELLED:
      return roq::OrderStatus::CANCELED;
    case LIQUIDATED:
      return roq::OrderStatus::CANCELED;
    case IOC:
      return roq::OrderStatus::COMPLETED;  // XXX ???
    case AUTO_DELEVERAGED:
      return roq::OrderStatus::CANCELED;  // XXX ???
    case REDUCE_ONLY:
      return roq::OrderStatus::CANCELED;
    case POSITION_CLOSE:
      return roq::OrderStatus::CANCELED;
    case STP:
      return roq::OrderStatus::CANCELED;
    case NEW:
      return roq::OrderStatus::WORKING;
    case UPDATE:
      return roq::OrderStatus::WORKING;
    case REDUCE_OUT:
      return roq::OrderStatus::WORKING;  // XXX ???
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::OrderStatus>(Helper{FinishAs{FinishAs::UNDEFINED__}}) == roq::OrderStatus::UNDEFINED);
static_assert(static_cast<roq::OrderStatus>(Helper{FinishAs{FinishAs::FILLED}}) == roq::OrderStatus::COMPLETED);
static_assert(static_cast<roq::OrderStatus>(Helper{FinishAs{FinishAs::CANCELLED}}) == roq::OrderStatus::CANCELED);
static_assert(static_cast<roq::OrderStatus>(Helper{FinishAs{FinishAs::LIQUIDATED}}) == roq::OrderStatus::CANCELED);
static_assert(static_cast<roq::OrderStatus>(Helper{FinishAs{FinishAs::IOC}}) == roq::OrderStatus::COMPLETED);
static_assert(static_cast<roq::OrderStatus>(Helper{FinishAs{FinishAs::AUTO_DELEVERAGED}}) == roq::OrderStatus::CANCELED);
static_assert(static_cast<roq::OrderStatus>(Helper{FinishAs{FinishAs::REDUCE_ONLY}}) == roq::OrderStatus::CANCELED);
static_assert(static_cast<roq::OrderStatus>(Helper{FinishAs{FinishAs::POSITION_CLOSE}}) == roq::OrderStatus::CANCELED);
static_assert(static_cast<roq::OrderStatus>(Helper{FinishAs{FinishAs::STP}}) == roq::OrderStatus::CANCELED);
static_assert(static_cast<roq::OrderStatus>(Helper{FinishAs{FinishAs::NEW}}) == roq::OrderStatus::WORKING);
static_assert(static_cast<roq::OrderStatus>(Helper{FinishAs{FinishAs::UPDATE}}) == roq::OrderStatus::WORKING);
static_assert(static_cast<roq::OrderStatus>(Helper{FinishAs{FinishAs::REDUCE_OUT}}) == roq::OrderStatus::WORKING);

// Role ==> roq::Liquidity

template <>
template <>
constexpr Helper<Role>::operator roq::Liquidity() {
  switch (std::get<0>(args_)) {
    using enum json::Role::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case MAKER:
      return roq::Liquidity::MAKER;
    case TAKER:
      return roq::Liquidity::TAKER;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::Liquidity>(Helper{Role{Role::UNDEFINED__}}) == roq::Liquidity::UNDEFINED);
static_assert(static_cast<roq::Liquidity>(Helper{Role{Role::MAKER}}) == roq::Liquidity::MAKER);
static_assert(static_cast<roq::Liquidity>(Helper{Role{Role::TAKER}}) == roq::Liquidity::TAKER);

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
Map<FinishAs>::operator roq::OrderStatus() {
  return Helper{args_};
}

template <>
template <>
Map<Role>::operator roq::Liquidity() {
  return Helper{args_};
}

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
