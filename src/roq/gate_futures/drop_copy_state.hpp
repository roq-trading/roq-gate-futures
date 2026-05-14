/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace gate_futures {

enum class DropCopyState : uint8_t {
  UNDEFINED = 0,
  LOGIN,
  SUBSCRIBE,
  ORDERS,
  DONE,
};

}  // namespace gate_futures
}  // namespace roq
