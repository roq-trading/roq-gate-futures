/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/gate_futures/market_data.hpp"

#include <algorithm>

#include "roq/mask.hpp"
#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"
#include "roq/core/charconv.hpp"

#include "roq/core/tools/exception.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/socket/client_factory.hpp"

#include "roq/gate_futures/flags.hpp"

#include "roq/gate_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace gate_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "md"sv;

auto const SUPPORTS = Mask{
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::ws_uri();
  web::socket::Client::Config config{
      .always_reconnect = true,
      .connection_timeout = server::Flags::net_connection_timeout(),
      .disconnect_on_idle_timeout = server::Flags::net_disconnect_on_idle_timeout(),
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
      .uris = {&uri, 1},
      .query = {},
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return web::socket::ClientFactory::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto const &group, auto const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

// === IMPLEMENTATION ===

MarketData::MarketData(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared, size_t index)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, index_{index},
      connection_{create_connection(*this, context)}, decode_buffer_{Flags::decode_buffer_size()},
      request_id_{static_cast<uint64_t>(stream_id_) * 1000000},  // scale (debugging)
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
      shared_{shared} {
}

void MarketData::operator()(Event<Start> const &) {
  (*connection_).start();
}

void MarketData::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void MarketData::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
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

void MarketData::operator()(web::socket::Client::Connected const &) {
}

void MarketData::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
}

void MarketData::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::READY);
  subscribe();
}

void MarketData::operator()(web::socket::Client::Close const &) {
}

void MarketData::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  const ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void MarketData::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    const StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void MarketData::subscribe(std::span<Symbol const> const &symbols) {
  if (std::empty(symbols))
    return;
  subscribe("futures.tickers"sv, symbols);
  subscribe("futures.trades"sv, symbols);
  subscribe("futures.book_ticker"sv, symbols);
  subscribe(
      "futures.order_book_update"sv, symbols, utils::safe_cast(Flags::order_book_freq()), Flags::order_book_depth());
}

void MarketData::subscribe(std::string_view const &channel, std::span<Symbol const> const &symbols) {
  assert(!std::empty(symbols));
  if (true) {
    std::chrono::seconds now = utils::safe_cast(clock::get_realtime());
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
    (*connection_).send_text(message);
  } else {
    for (auto &symbol : symbols) {
      std::chrono::seconds now = utils::safe_cast(clock::get_realtime());
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
      (*connection_).send_text(message);
    }
  }
}

void MarketData::subscribe(
    std::string_view const &channel,
    std::span<Symbol const> const &symbols,
    const std::chrono::milliseconds frequency,
    const uint32_t depth) {
  assert(!std::empty(symbols));
  for (auto &symbol : symbols) {
    std::chrono::seconds now = utils::safe_cast(clock::get_realtime());
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
    (*connection_).send_text(message);
  }
}

void MarketData::parse(std::string_view const &message) {
  profile_.parse([&]() {
    try {
      TraceInfo trace_info;
      core::json::Buffer buffer{decode_buffer_};
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

void MarketData::operator()(Trace<json::Subscribe> const &event) {
  profile_.subscribe([&]() {
    auto &[trace_info, subscribe] = event;
    log::info<3>("trace_info={}, subscribe={}"sv, trace_info, subscribe);
    log::debug("subscribe={}"sv, subscribe);
  });
}

void MarketData::operator()(Trace<json::Tickers> const &event) {
  profile_.tickers([&]() {
    auto &[trace_info, tickers] = event;
    log::info<3>("trace_info={}, tickers={}"sv, trace_info, tickers);
    (*connection_).touch(trace_info.source_receive_time);
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
          {
              .type = StatisticsType::HIGHEST_TRADED_PRICE,
              .value = item.high_24h,
              .begin_time_utc = {},
              .end_time_utc = {},
          },
          {
              .type = StatisticsType::LOWEST_TRADED_PRICE,
              .value = item.low_24h,
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
      create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
    }
  });
}

void MarketData::operator()(Trace<json::Trades> const &event) {
  profile_.trades([&]() {
    auto &[trace_info, trades] = event;
    log::info<3>("trace_info={}, trades={}"sv, trace_info, trades);
    (*connection_).touch(trace_info.source_receive_time);
    auto &result = trades.result;
    auto create_trade = []<typename T>(T &result, auto const &value) {
      auto const side = utils::compare(value.size, 0.0) == std::strong_ordering::less ? Side::SELL : Side::BUY;
      new (&result) Trade{
          .side = side,
          .price = value.price,
          .quantity = std::fabs(value.size),
          .trade_id = {},
          .taker_order_id = {},
          .maker_order_id = {},
      };
      core::charconv::to_string(std::back_inserter(result.trade_id), value.id);
    };
    core::back_emplacer trades_{shared_.trades};
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
              .exchange_sequence = {},
          };
          create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
        }
        contract = item.contract;
        timestamp = {};
      }
      trades_.emplace_back([&](auto &result) { create_trade(result, item); });
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
      create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
    }
  });
}

void MarketData::operator()(Trace<json::BookTicker> const &event) {
  profile_.book_ticker([&]() {
    auto &[trace_info, book_ticker] = event;
    log::info<3>("trace_info={}, book_ticker={}"sv, trace_info, book_ticker);
    (*connection_).touch(trace_info.source_receive_time);
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
        .exchange_sequence = {},
    };
    create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
  });
}

void MarketData::operator()(Trace<json::OrderBookUpdate> const &event) {
  profile_.order_book_update([&]() {
    auto &trace_info = event.trace_info;
    auto &order_book_update = event.value;
    log::info<3>("trace_info={}, order_book_update={}"sv, trace_info, order_book_update);
    (*connection_).touch(trace_info.source_receive_time);
    auto &result = order_book_update.result;
    auto &symbol = result.symbol;
    auto first_sequence = result.first_update_id;
    auto last_sequence = result.last_update_id;
    auto &collector = shared_.mbp_collector[symbol];
    auto create_mbp_update = []<typename T>(T &result, auto const &item) {
      new (&result) T{
          .price = item.price,
          .quantity = item.size,
          .implied_quantity = NaN,
          .number_of_orders = {},
          .update_action = {},
          .price_level = {},
      };
    };
    core::back_emplacer bids{shared_.bids}, asks{shared_.asks};
    for (auto &item : result.bids)
      bids.emplace_back([&](auto &result) { create_mbp_update(result, item); });
    for (auto &item : result.asks)
      asks.emplace_back([&](auto &result) { create_mbp_update(result, item); });
    auto exchange_time_utc = result.timestamp;
    try {
      auto create_update = [&](auto &bids, auto &asks, auto update_type, auto exchange_sequence) {
        return MarketByPriceUpdate{
            .stream_id = stream_id_,
            .exchange = Flags::exchange(),
            .symbol = symbol,
            .bids = bids,
            .asks = asks,
            .update_type = update_type,
            .exchange_time_utc = exchange_time_utc,
            .exchange_sequence = exchange_sequence,
            .price_decimals = {},
            .quantity_decimals = {},
            .checksum = {},
        };
      };
      auto publish_update = [&](auto &bids, auto &asks) {
        // log::debug(R"(PUBLISH UPDATE symbol="{}")"sv, symbol);
        auto market_by_price_update = create_update(bids, asks, UpdateType::INCREMENTAL, last_sequence);
        create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true, false);
      };
      auto publish_snapshot = [&](auto &bids, auto &asks, auto sequence) {
        log::debug(R"(PUBLISH SNAPSHOT symbol="{}", sequence={})"sv, symbol, sequence);
        auto market_by_price_update = create_update(bids, asks, UpdateType::SNAPSHOT, collector.last_sequence());
        Trace event{trace_info, market_by_price_update};
        shared_(event, true, [&](auto &market_by_price) { collector.apply(market_by_price, sequence, true); });
      };
      auto request_snapshot = [&](auto retries) {
        log::debug(R"(REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
        if (Flags::ws_mbp_request_max_retries() && Flags::ws_mbp_request_max_retries() < retries) {
          log::fatal(R"(Unexpected: symbol="{}", retries={})"sv, symbol, retries);
        }
        shared_.depth_request_queue.emplace_back(symbol);
      };
      collector(
          bids,
          asks,
          first_sequence,
          last_sequence,
          first_sequence - 1,
          publish_update,
          publish_snapshot,
          request_snapshot);
    } catch (BadState &) {
      log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
      collector.clear();
      shared_.depth_request_queue.emplace_back(symbol);
    }
  });
}

}  // namespace gate_futures
}  // namespace roq
