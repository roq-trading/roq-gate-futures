/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/gate_futures/protocol/json/finish_as.hpp"
#include "roq/gate_futures/protocol/json/role.hpp"
#include "roq/gate_futures/protocol/json/tif.hpp"

#include "roq/liquidity.hpp"
#include "roq/order_status.hpp"
#include "roq/time_in_force.hpp"

#include "roq/map.hpp"

namespace roq {

template <>
template <>
std::optional<OrderStatus> Map<gate_futures::protocol::json::FinishAs>::helper() const;

template <>
template <>
std::optional<Liquidity> Map<gate_futures::protocol::json::Role>::helper() const;

template <>
template <>
std::optional<TimeInForce> Map<gate_futures::protocol::json::TIF>::helper() const;

}  // namespace roq
