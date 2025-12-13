/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/gate_futures/json/trade_parser.hpp"

namespace roq {
namespace gate_futures {

template <typename T>
struct TradeParserTester final : public json::TradeParser::Handler {
  using value_type = std::remove_cvref_t<T>;
  using callback_type = std::function<void(value_type const &)>;

  static void dispatch(callback_type const &callback, std::string_view const &message, size_t buffer_size, size_t max_depth) {
    core::json::BufferStack buffers{buffer_size, max_depth};
    // simple
    // XXX FIXME TODO catch2 block ???
    T obj{message, buffers};
    callback(obj);
    // parser
    // XXX FIXME TODO catch2 block ???
    TradeParserTester handler{callback};
    auto res = json::TradeParser::dispatch(handler, message, buffers, {}, false);
    CHECK(res == true);
    CHECK(handler.found_ == true);
  }

 protected:
  explicit TradeParserTester(callback_type const &callback) : callback_{callback} {}

  void operator()(Trace<json::TradeLogin> const &event) override { dispatch(event); }
  void operator()(Trace<json::TradeSubscribe> const &event) override { dispatch(event); }
  void operator()(Trace<json::TradeBalances> const &event) override { dispatch(event); }
  void operator()(Trace<json::TradePositions> const &event) override { dispatch(event); }
  void operator()(Trace<json::TradeOrders> const &event) override { dispatch(event); }
  void operator()(Trace<json::TradeTrades> const &event) override { dispatch(event); }
  void operator()(Trace<json::TradeOrderPlace> const &event) override { dispatch(event); }
  void operator()(Trace<json::TradeOrderAmend> const &event) override { dispatch(event); }
  void operator()(Trace<json::TradeOrderCancel> const &event) override { dispatch(event); }
  void operator()(Trace<json::TradeOrderCancelCP> const &event) override { dispatch(event); }
  void operator()(Trace<json::TradeOrderList> const &event) override { dispatch(event); }

  template <typename U>
  void dispatch(Trace<U> const &event) {
    if constexpr (std::is_invocable_v<callback_type, U>) {
      found_ = true;
      callback_(event);
    } else {
      FAIL();
    }
  }

 private:
  callback_type const callback_;
  bool found_ = false;
};

}  // namespace gate_futures
}  // namespace roq
