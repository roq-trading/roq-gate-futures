/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace gate_futures {

enum class OrderEntryState : uint8_t {
  UNDEFINED = 0,
  ACCOUNTS,
  POSITIONS,
  TRADES,
  DONE,
};

}  // namespace gate_futures
}  // namespace roq
