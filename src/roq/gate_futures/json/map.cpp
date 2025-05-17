/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/gate_futures/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// gate_futures => roq

// gate_futures::json::FinishAs ==> roq::OrderStatus

template <>
template <>
constexpr Helper<gate_futures::json::FinishAs>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum gate_futures::json::FinishAs::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderStatus::UNDEFINED;
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
  return {};
}

static_assert(Helper{gate_futures::json::FinishAs{gate_futures::json::FinishAs::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{gate_futures::json::FinishAs{gate_futures::json::FinishAs::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{gate_futures::json::FinishAs{gate_futures::json::FinishAs::CANCELLED}} == roq::OrderStatus::CANCELED);
static_assert(Helper{gate_futures::json::FinishAs{gate_futures::json::FinishAs::LIQUIDATED}} == roq::OrderStatus::CANCELED);
static_assert(Helper{gate_futures::json::FinishAs{gate_futures::json::FinishAs::IOC}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{gate_futures::json::FinishAs{gate_futures::json::FinishAs::AUTO_DELEVERAGED}} == roq::OrderStatus::CANCELED);
static_assert(Helper{gate_futures::json::FinishAs{gate_futures::json::FinishAs::REDUCE_ONLY}} == roq::OrderStatus::CANCELED);
static_assert(Helper{gate_futures::json::FinishAs{gate_futures::json::FinishAs::POSITION_CLOSE}} == roq::OrderStatus::CANCELED);
static_assert(Helper{gate_futures::json::FinishAs{gate_futures::json::FinishAs::STP}} == roq::OrderStatus::CANCELED);
static_assert(Helper{gate_futures::json::FinishAs{gate_futures::json::FinishAs::NEW}} == roq::OrderStatus::WORKING);
static_assert(Helper{gate_futures::json::FinishAs{gate_futures::json::FinishAs::UPDATE}} == roq::OrderStatus::WORKING);
static_assert(Helper{gate_futures::json::FinishAs{gate_futures::json::FinishAs::REDUCE_OUT}} == roq::OrderStatus::WORKING);

template <>
template <>
std::optional<roq::OrderStatus> Map<gate_futures::json::FinishAs>::helper() const {
  return Helper{args_};
}

// gate_futures::json::Role ==> roq::Liquidity

template <>
template <>
constexpr Helper<gate_futures::json::Role>::operator std::optional<roq::Liquidity>() const {
  switch (std::get<0>(args_)) {
    using enum gate_futures::json::Role::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Liquidity::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::Liquidity::UNDEFINED;
    case MAKER:
      return roq::Liquidity::MAKER;
    case TAKER:
      return roq::Liquidity::TAKER;
  }
  return {};
}

static_assert(Helper{gate_futures::json::Role{gate_futures::json::Role::UNDEFINED_INTERNAL}} == roq::Liquidity::UNDEFINED);
static_assert(Helper{gate_futures::json::Role{gate_futures::json::Role::MAKER}} == roq::Liquidity::MAKER);
static_assert(Helper{gate_futures::json::Role{gate_futures::json::Role::TAKER}} == roq::Liquidity::TAKER);

template <>
template <>
std::optional<roq::Liquidity> Map<gate_futures::json::Role>::helper() const {
  return Helper{args_};
}

// gate_futures::json::TIF ==> roq::TimeInForce

template <>
template <>
constexpr Helper<gate_futures::json::TIF>::operator std::optional<roq::TimeInForce>() const {
  switch (std::get<0>(args_)) {
    using enum gate_futures::json::TIF::type_t;
    case UNDEFINED_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::TimeInForce::UNDEFINED;
    case GTC:
      return roq::TimeInForce::GTC;
    case FOK:
      return roq::TimeInForce::FOK;
    case POC:
      return roq::TimeInForce::GTC;  // note!
    case IOC:
      return roq::TimeInForce::IOC;
  }
  return {};
}

static_assert(Helper{gate_futures::json::TIF{gate_futures::json::TIF::UNDEFINED_INTERNAL}} == roq::TimeInForce::UNDEFINED);
static_assert(Helper{gate_futures::json::TIF{gate_futures::json::TIF::GTC}} == roq::TimeInForce::GTC);
static_assert(Helper{gate_futures::json::TIF{gate_futures::json::TIF::FOK}} == roq::TimeInForce::FOK);
static_assert(Helper{gate_futures::json::TIF{gate_futures::json::TIF::POC}} == roq::TimeInForce::GTC);
static_assert(Helper{gate_futures::json::TIF{gate_futures::json::TIF::IOC}} == roq::TimeInForce::IOC);

template <>
template <>
std::optional<roq::TimeInForce> Map<gate_futures::json::TIF>::helper() const {
  return Helper{args_};
}

}  // namespace roq
