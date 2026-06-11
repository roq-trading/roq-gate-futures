/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>

#include "roq/cancel_all_orders.hpp"
#include "roq/cancel_order.hpp"
#include "roq/create_order.hpp"
#include "roq/modify_order.hpp"

#include "roq/server/oms/order.hpp"
#include "roq/server/oms/ref_data.hpp"

namespace roq {
namespace gate_futures {
namespace protocol {
namespace json {

struct Encoder final {
  static std::string_view order_place(
      std::string &buffer,
      roq::Event<CreateOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      uint32_t request_id_2);

  static std::string_view order_amend(
      std::string &buffer,
      roq::Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      uint32_t request_id_2);

  static std::string_view order_cancel(
      std::string &buffer,
      roq::Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id,
      uint32_t request_id_2);

  static std::string_view order_cancel_cp(
      std::string &buffer, roq::Event<CancelAllOrders> const &, std::string_view const &request_id, uint32_t request_id_2, std::string_view const &symbol);
};

}  // namespace json
}  // namespace protocol
}  // namespace gate_futures
}  // namespace roq
