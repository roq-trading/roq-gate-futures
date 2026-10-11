/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <span>

#include "roq/string_types.hpp"

namespace roq {
namespace gate_futures {
namespace gateway {

struct SymbolsUpdate final {
  std::span<Symbol const> symbols;
};

}  // namespace gateway
}  // namespace gate_futures
}  // namespace roq
