/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gateio_futures/market_data.h"

#include <algorithm>

#include "roq/utils/mask.h"
#include "roq/utils/safe_cast.h"
#include "roq/utils/update.h"

#include "roq/core/back_emplacer.h"
#include "roq/core/charconv.h"

#include "roq/core/tools/exception.h"

#include "roq/core/metrics/factory.h"

#include "roq/gateio_futures/flags.h"

#include "roq/gateio_futures/json/utils.h"

using namespace std::literals;

namespace roq {
namespace gateio_futures {

namespace {
static const auto NAME = "md"sv;
static const auto SUPPORTS = utils::Mask{
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

template <typename T>
void emplace(MBPUpdate &result, const T &item) {
  new (&result) MBPUpdate{
      .price = item.price,
      .quantity = item.quantity,
      .implied_quantity = NaN,
      .price_level = {},
      .number_of_orders = {},
  };
}
}  // namespace

MarketData::MarketData(
    Handler &handler, core::io::Context &context, uint32_t stream_id, Shared &shared, size_t index)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      index_(index), connection_(
                         *this,
                         context,
                         Flags::ws_uri(),
                         {},
                         Flags::ws_ping_freq(),
                         Flags::decode_buffer_size(),
                         Flags::encode_buffer_size(),
                         []() { return std::string(); }),
      decode_buffer_(Flags::decode_buffer_size()),
      request_id_(static_cast<uint64_t>(stream_id_) * 1000000),  // scale (debugging)
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .book_ticker = create_metrics(name_, "book_ticker"sv),
          .depth = create_metrics(name_, "depth"sv),
          .trade = create_metrics(name_, "trade"sv),
          .realtimes = create_metrics(name_, "realtimes"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
      },
      shared_(shared) {
}

void MarketData::operator()(const Event<Start> &) {
  connection_.start();
}

void MarketData::operator()(const Event<Stop> &) {
  connection_.stop();
}

void MarketData::operator()(const Event<Timer> &event) {
  auto now = event.value.now;
  connection_.refresh(now);
}

void MarketData::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.book_ticker, metrics::PROFILE)
      .write(profile_.depth, metrics::PROFILE)
      .write(profile_.trade, metrics::PROFILE)
      .write(profile_.realtimes, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

void MarketData::subscribe(size_t start_from) {
  if (ready())
    subscribe(shared_.symbols.get_slice(index_, start_from));
}

void MarketData::operator()(const core::web::ClientSocket::Connected &) {
}

void MarketData::operator()(const core::web::ClientSocket::Disconnected &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
}

void MarketData::operator()(const core::web::ClientSocket::Ready &) {
  (*this)(ConnectionStatus::READY);
  subscribe();
}

void MarketData::operator()(const core::web::ClientSocket::Close &) {
}

void MarketData::operator()(const core::web::ClientSocket::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(const core::web::ClientSocket::Text &text) {
  parse(text.payload);
}

void MarketData::operator()(const core::web::ClientSocket::Binary &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"sv, stream_status);
    server::create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void MarketData::subscribe(const roq::span<std::string const> &symbols) {
  if (std::empty(symbols))
    return;
  subscribe("futures.tickers"sv, symbols);
  subscribe("futures.trades"sv, symbols);
  subscribe("futures.book_ticker"sv, symbols);
  // subscribe("spot.order_book_update"sv, symbols);  // XXX needs a second argument with the period
}

void MarketData::subscribe(
    const std::string_view &channel, const roq::span<std::string const> &symbols) {
  assert(!std::empty(symbols));
  if (true) {
    std::chrono::seconds now = utils::safe_cast(core::get_realtime_clock());
    auto message = fmt::format(
        R"({{)"
        R"("time":{},)"
        R"("channel":"{}",)"
        R"("event":"subscribe",)"
        R"("payload":["{}"])"
        R"(}})"sv,
        now.count(),
        channel,
        fmt::join(symbols, R"(",")"));
    log::debug("message={}"sv, message);
    connection_.send_text(message);
  } else {
    for (auto &symbol : symbols) {
      std::chrono::seconds now = utils::safe_cast(core::get_realtime_clock());
      auto message = fmt::format(
          R"({{)"
          R"("time":{},)"
          R"("channel":"{}",)"
          R"("event":"subscribe",)"
          R"("payload":["{}"])"
          R"(}})"sv,
          now.count(),
          channel,
          symbol);
      log::debug("message={}"sv, message);
      connection_.send_text(message);
    }
  }
}

void MarketData::parse(const std::string_view &message) {
  profile_.parse([&]() {
    try {
      auto trace_info = server::create_trace_info();
      core::json::Buffer buffer(decode_buffer_);
      if (json::Parser::dispatch(*this, message, buffer, trace_info)) {
      } else {
        log::warn(R"(message="{}")"sv, message);
      }
    } catch (...) {
      log::warn(R"(message="{}")"sv, message);
      core::tools::UnhandledException::terminate();
    }
  });
}

}  // namespace gateio_futures
}  // namespace roq
