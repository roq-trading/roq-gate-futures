/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/gate_futures/json/encoder.hpp"

#include <fmt/format.h>

#include "roq/decimal.hpp"

#include "roq/utils/common.hpp"

#include "roq/server/oms/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {
namespace json {

std::string_view Encoder::order_place(
    std::string &buffer, roq::Event<CreateOrder> const &event, server::oms::Order const &order, std::string_view const &request_id, uint32_t request_id_2) {
  buffer.clear();
  auto &[message_info, create_order] = event;
  assert(message_info.receive_time_utc.count() > 0);
  auto channel = "futures.order_place"sv;
  auto event_2 = "api"sv;
  auto sign = utils::sign(create_order.side);
  auto quantity = sign * create_order.quantity;
  auto price = [&]() {
    if (create_order.order_type == OrderType::MARKET) {
      return 0.0;
    }
    if (std::isnan(create_order.price)) {
      return 0.0;
    }
    return create_order.price;
  }();
  auto reduce_only = create_order.execution_instructions.has(ExecutionInstruction::DO_NOT_INCREASE);
  auto tif = [&]() -> std::string_view {
    if (create_order.order_type == OrderType::MARKET) {
      return "ioc"sv;
    }
    if (create_order.execution_instructions.has(ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE)) {
      return "poc"sv;
    }
    switch (create_order.time_in_force) {
      using enum TimeInForce;
      case UNDEFINED:
        break;
      case GFD:
        break;
      case GTC:
        return "gtc"sv;
      case OPG:
        break;
      case IOC:
        return "ioc"sv;
      case FOK:
        return "fok"sv;
      case GTX:
        break;
      case GTD:
        break;
      case AT_THE_CLOSE:
        break;
      case GOOD_THROUGH_CROSSING:
        break;
      case AT_CROSSING:
        break;
      case GOOD_FOR_TIME:
        break;
      case GFA:
        break;
      case GFM:
        break;
    }
    throw server::oms::NotSupported{"not supported"sv};
  }();
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":{},)"
      R"("time":{},)"
      R"("channel":"{}",)"
      R"("event":"{}",)"
      R"("payload":{{)"
      R"("req_id":"{}",)"
      R"("req_param":{{)"
      R"("contract":"{}",)"
      R"("size":{},)"
      R"("iceberg":0,)"
      R"("price":"{}",)"
      R"("close":false,)"
      R"("reduce_only":{},)"
      R"("tif":"{}",)"
      R"("text":"t-{}")"
      // R"("stp_act":"cn")"  // XXX make flag
      R"(}})"
      R"(}})"
      R"(}})"sv,
      request_id_2,
      message_info.receive_time_utc.count(),
      channel,
      event_2,
      request_id,
      order.symbol,
      Decimal{quantity, order.quantity_precision.precision},
      Decimal{price, order.price_precision.precision},
      reduce_only,
      tif,
      request_id);
  return buffer;
}

std::string_view Encoder::order_amend(
    std::string &buffer,
    roq::Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    uint32_t request_id_2) {
  buffer.clear();
  auto &[message_info, modify_order] = event;
  assert(message_info.receive_time_utc.count() > 0);
  auto const channel = "futures.order_amend"sv;
  auto const event_2 = "api"sv;
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":{},)"
      R"("time":{},)"
      R"("channel":"{}",)"
      R"("event":"{}",)"
      R"("payload":{{)"
      R"("req_id":"{}",)"
      R"("req_param":{{)"
      R"("order_id":"t-{}",)"sv,
      request_id_2,
      message_info.receive_time_utc.count(),
      channel,
      event_2,
      request_id,
      order.client_order_id);
  if (!std::isnan(modify_order.quantity)) {
    auto sign = utils::sign(order.side);
    auto quantity = sign * modify_order.quantity;
    fmt::format_to(std::back_inserter(buffer), R"("size":{},)", Decimal{quantity, order.quantity_precision.precision});
  }
  if (!std::isnan(modify_order.price)) {
    fmt::format_to(std::back_inserter(buffer), R"("price":"{}",)"sv, Decimal{modify_order.price, order.price_precision.precision});
  }
  fmt::format_to(
      std::back_inserter(buffer),
      R"("amend_text":"{}")"
      R"(}})"
      R"(}})"
      R"(}})"sv,
      order.client_order_id);
  return buffer;
}

std::string_view Encoder::order_cancel(
    std::string &buffer,
    roq::Event<CancelOrder> const &event,
    server::oms::Order const &order,
    std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id,
    uint32_t request_id_2) {
  buffer.clear();
  auto &[message_info, cancel_order] = event;
  assert(message_info.receive_time_utc.count() > 0);
  auto const channel = "futures.order_cancel"sv;
  auto const event_2 = "api"sv;
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":{},)"
      R"("time":{},)"
      R"("channel":"{}",)"
      R"("event":"{}",)"
      R"("payload":{{)"
      R"("req_id":"{}",)"
      R"("req_param":{{)"
      R"("order_id":"t-{}")"
      R"(}})"
      R"(}})"
      R"(}})"sv,
      request_id_2,
      message_info.receive_time_utc.count(),
      channel,
      event_2,
      request_id,
      order.client_order_id);
  return buffer;
}

std::string_view Encoder::order_cancel_cp(
    std::string &buffer, roq::Event<CancelAllOrders> const &event, std::string_view const &request_id, uint32_t request_id_2, std::string_view const &symbol) {
  buffer.clear();
  auto &[message_info, cancel_all_orders] = event;
  assert(message_info.receive_time_utc.count() > 0);
  auto const channel = "futures.order_cancel_cp"sv;
  auto const event_2 = "api"sv;
  fmt::format_to(
      std::back_inserter(buffer),
      R"({{)"
      R"("id":{},)"
      R"("time":{},)"
      R"("channel":"{}",)"
      R"("event":"{}",)"
      R"("payload":{{)"
      R"("req_id":"{}",)"
      R"("req_param":{{)"
      R"("contract":"{}")"
      R"(}})"
      R"(}})"
      R"(}})"sv,
      request_id_2,
      message_info.receive_time_utc.count(),
      channel,
      event_2,
      request_id,
      symbol);
  return buffer;
}

}  // namespace json
}  // namespace gate_futures
}  // namespace roq
