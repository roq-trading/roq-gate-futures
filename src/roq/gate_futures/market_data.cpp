/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/gate_futures/market_data.h"

#include <algorithm>

#include "roq/utils/mask.h"
#include "roq/utils/safe_cast.h"
#include "roq/utils/update.h"

#include "roq/core/back_emplacer.h"
#include "roq/core/charconv.h"

#include "roq/core/tools/exception.h"

#include "roq/core/metrics/factory.h"

#include "roq/gate_futures/flags.h"

#include "roq/gate_futures/json/utils.h"

using namespace std::literals;

namespace roq {
namespace gate_futures {

namespace {
const auto NAME = "md"sv;
const auto SUPPORTS = utils::Mask{
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
      .quantity = item.size,
      .implied_quantity = NaN,
      .price_level = {},
      .number_of_orders = {},
  };
}

template <typename T>
void emplace(Trade &result, const T &value) {
  new (&result) Trade{
      .side = {},
      .price = value.price,
      .quantity = value.size,
      .trade_id = {},
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
          .subscribe = create_metrics(name_, "subscribe"sv),
          .tickers = create_metrics(name_, "tickers"sv),
          .trades = create_metrics(name_, "trades"sv),
          .book_ticker = create_metrics(name_, "book_ticker"sv),
          .order_book_update = create_metrics(name_, "order_book_update"sv),
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
      .write(profile_.subscribe, metrics::PROFILE)
      .write(profile_.tickers, metrics::PROFILE)
      .write(profile_.trades, metrics::PROFILE)
      .write(profile_.book_ticker, metrics::PROFILE)
      .write(profile_.order_book_update, metrics::PROFILE)
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
      .account = {},
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

void MarketData::subscribe(const std::span<std::string const> &symbols) {
  if (std::empty(symbols))
    return;
  subscribe("futures.tickers"sv, symbols);
  subscribe("futures.trades"sv, symbols);
  subscribe("futures.book_ticker"sv, symbols);
  subscribe(
      "futures.order_book_update"sv,
      symbols,
      utils::safe_cast(Flags::order_book_freq()),
      Flags::order_book_depth());
}

void MarketData::subscribe(
    const std::string_view &channel, const std::span<std::string const> &symbols) {
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

void MarketData::subscribe(
    const std::string_view &channel,
    const std::span<std::string const> &symbols,
    const std::chrono::milliseconds frequency,
    const uint32_t depth) {
  assert(!std::empty(symbols));
  for (auto &symbol : symbols) {
    std::chrono::seconds now = utils::safe_cast(core::get_realtime_clock());
    auto message = fmt::format(
        R"({{)"
        R"("time":{},)"
        R"("channel":"{}",)"
        R"("event":"subscribe",)"
        R"("payload":["{}","{}ms","{}"])"
        R"(}})"sv,
        now.count(),
        channel,
        symbol,
        frequency.count(),
        depth);
    log::debug("message={}"sv, message);
    connection_.send_text(message);
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

void MarketData::operator()(server::Trace<json::Subscribe> const &event) {
  profile_.subscribe([&]() {
    auto &[trace_info, subscribe] = event;
    log::info<3>("trace_info={}, subscribe={}"sv, trace_info, subscribe);
    log::debug("subscribe={}"sv, subscribe);
  });
}

void MarketData::operator()(server::Trace<json::Tickers> const &event) {
  profile_.tickers([&]() {
    auto &[trace_info, tickers] = event;
    log::info<3>("trace_info={}, tickers={}"sv, trace_info, tickers);
    for (auto &item : tickers.result) {
      Statistics statistics[] = {
          {
              .type = StatisticsType::TRADE_VOLUME,
              .value = item.volume_24h_quote,
              .begin_time_utc = {},
              .end_time_utc = {},
          },
          {
              .type = StatisticsType::INDEX_VALUE,
              .value = item.index_price,
              .begin_time_utc = {},
              .end_time_utc = {},
          },
          {
              .type = StatisticsType::FUNDING_RATE,
              .value = item.funding_rate,
              .begin_time_utc = {},
              .end_time_utc = {},
          },
          {
              .type = StatisticsType::FUNDING_RATE_PREDICTION,
              .value = item.funding_rate_indicative,
              .begin_time_utc = {},
              .end_time_utc = {},
          },
      };
      const StatisticsUpdate statistics_update{
          .stream_id = stream_id_,
          .exchange = Flags::exchange(),
          .symbol = item.contract,
          .statistics = statistics,
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = utils::safe_cast(tickers.time),
      };
      server::create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
    }
  });
}

void MarketData::operator()(server::Trace<json::Trades> const &event) {
  profile_.trades([&]() {
    auto &[trace_info, trades] = event;
    log::info<3>("trace_info={}, trades={}"sv, trace_info, trades);
    auto &result = trades.result;
    core::back_emplacer trades_(shared_.trades);
    std::string_view contract;
    decltype(json::TradesItem::create_time_ms) timestamp = {};
    for (auto &item : result) {
      if (item.contract.compare(contract) != 0) {
        if (!std::empty(contract) && !std::empty(trades_)) {
          const TradeSummary trade_summary{
              .stream_id = stream_id_,
              .exchange = Flags::exchange(),
              .symbol = contract,
              .trades = trades_,
              .exchange_time_utc = utils::safe_cast(timestamp),
          };
          server::create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
        }
        contract = item.contract;
        timestamp = {};
      }
      trades_.emplace_back([&item](auto &result) { emplace(result, item); });
      utils::update_max(timestamp, item.create_time_ms);
    }
    if (!std::empty(trades_)) {
      const TradeSummary trade_summary{
          .stream_id = stream_id_,
          .exchange = Flags::exchange(),
          .symbol = contract,
          .trades = trades_,
          .exchange_time_utc = utils::safe_cast(timestamp),
      };
      server::create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
    }
  });
}

void MarketData::operator()(server::Trace<json::BookTicker> const &event) {
  profile_.book_ticker([&]() {
    auto &[trace_info, book_ticker] = event;
    log::info<3>("trace_info={}, book_ticker={}"sv, trace_info, book_ticker);
    auto &result = book_ticker.result;
    const TopOfBook top_of_book{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = result.contract,
        .layer{
            .bid_price = result.best_bid_price,
            .bid_quantity = result.best_bid_size,
            .ask_price = result.best_ask_price,
            .ask_quantity = result.best_ask_size,
        },
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(result.timestamp),
    };
    server::create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
  });
}

void MarketData::operator()(server::Trace<json::OrderBookUpdate> const &event) {
  profile_.order_book_update([&]() {
    // auto &[trace_info, order_book_update] = event;
    auto &trace_info = event.trace_info;
    auto &order_book_update = event.value;
    log::info<3>("trace_info={}, order_book_update={}"sv, trace_info, order_book_update);
    auto &result = order_book_update.result;
    auto &symbol = result.symbol;
    auto first_sequence = result.first_update_id;
    auto last_sequence = result.last_update_id;
    auto &collector = shared_.mbp_collector[symbol];
    core::back_emplacer bids(shared_.bids), asks(shared_.asks);
    for (auto &item : result.bids)
      bids.emplace_back([&item](auto &result) { emplace(result, item); });
    for (auto &item : result.asks)
      asks.emplace_back([&item](auto &result) { emplace(result, item); });
    auto exchange_time_utc = result.timestamp;
    try {
      collector(
          bids,
          asks,
          first_sequence,
          last_sequence,
          first_sequence - 1,
          [&](auto &bids, auto &asks) {  // update
            // log::debug(R"(PUBLISH UPDATE symbol="{}")"sv, symbol);
            MarketByPriceUpdate market_by_price_update{
                .stream_id = stream_id_,
                .exchange = Flags::exchange(),
                .symbol = symbol,
                .bids = bids,
                .asks = asks,
                .update_type = UpdateType::INCREMENTAL,
                .exchange_time_utc = exchange_time_utc,
                .exchange_sequence = last_sequence,
                .price_decimals = {},
                .quantity_decimals = {},
                .checksum = {},
            };
            server::create_trace_and_dispatch(
                handler_, trace_info, market_by_price_update, true, false);
          },
          [&](auto &bids, auto &asks, auto sequence) {  // snapshot
            log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
            MarketByPriceUpdate market_by_price_update{
                .stream_id = stream_id_,
                .exchange = Flags::exchange(),
                .symbol = symbol,
                .bids = bids,
                .asks = asks,
                .update_type = UpdateType::SNAPSHOT,
                .exchange_time_utc = exchange_time_utc,
                .exchange_sequence = collector.last_sequence(),
                .price_decimals = {},
                .quantity_decimals = {},
                .checksum = {},
            };
            server::Trace event(trace_info, market_by_price_update);
            shared_(event, true, [&](auto &market_by_price) {
              collector.apply(market_by_price, sequence, true);
            });
          },
          [&](auto retries) {  // request
            log::debug(R"(REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
            if (Flags::ws_mbp_request_max_retries() &&
                Flags::ws_mbp_request_max_retries() < retries) {
              log::fatal(R"(Unexpected: symbol="{}", retries={})"sv, symbol, retries);
            }
            shared_.depth_request_queue.emplace_back(symbol);
          });
    } catch (BadState &) {
      log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
      collector.clear();
      shared_.depth_request_queue.emplace_back(symbol);
    }
  });
}

}  // namespace gate_futures
}  // namespace roq
