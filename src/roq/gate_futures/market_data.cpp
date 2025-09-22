/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/gate_futures/market_data.hpp"

#include <algorithm>

#include "roq/logging.hpp"

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/charconv/to_string.hpp"

#include "roq/utils/exceptions/unhandled.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

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

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.ws.uri;
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = settings.net.disconnect_on_idle_timeout,
      .always_reconnect = true,
      // proxy
      .proxy = {},
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};
}  // namespace

// === IMPLEMENTATION ===

MarketData::MarketData(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared, size_t index)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, index_{index}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{
          shared.settings.misc.decode_buffer_size,
          MAX_DECODE_BUFFER_DEPTH,
      },
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .subscribe = create_metrics(shared.settings, name_, "subscribe"sv),
          .tickers = create_metrics(shared.settings, name_, "tickers"sv),
          .trades = create_metrics(shared.settings, name_, "trades"sv),
          .book_ticker = create_metrics(shared.settings, name_, "book_ticker"sv),
          .order_book_update = create_metrics(shared.settings, name_, "order_book_update"sv),
          .candlesticks = create_metrics(shared.settings, name_, "candlesticks"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
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

void MarketData::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.subscribe, metrics::Type::PROFILE)
      .write(profile_.tickers, metrics::Type::PROFILE)
      .write(profile_.trades, metrics::Type::PROFILE)
      .write(profile_.book_ticker, metrics::Type::PROFILE)
      .write(profile_.order_book_update, metrics::Type::PROFILE)
      .write(profile_.candlesticks, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

void MarketData::subscribe(size_t start_from) {
  if (ready()) {
    subscribe(shared_.symbols.get_slice(index_, start_from));
  }
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
  auto external_latency = ExternalLatency{
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
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
        .interface = (*connection_).get_interface(),
        .authority = (*connection_).get_current_authority(),
        .path = (*connection_).get_current_path(),
        .proxy = (*connection_).get_proxy(),
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void MarketData::subscribe(std::span<Symbol const> const &symbols) {
  if (std::empty(symbols)) {
    return;
  }
  subscribe("futures.tickers"sv, symbols);
  subscribe("futures.trades"sv, symbols);
  subscribe("futures.book_ticker"sv, symbols);
  subscribe("futures.order_book_update"sv, symbols, utils::safe_cast(shared_.settings.misc.order_book_freq), shared_.settings.misc.order_book_depth);
  if (shared_.settings.download.time_series_lookback.count()) {
    subscribe("futures.candlesticks"sv, symbols, "1m"sv);
  }
}

void MarketData::subscribe(std::string_view const &channel, std::span<Symbol const> const &symbols) {
  assert(!std::empty(symbols));
  auto now = clock::get_realtime<std::chrono::seconds>();
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
  (*connection_).send_text(message);
}

// order book
void MarketData::subscribe(std::string_view const &channel, std::span<Symbol const> const &symbols, std::chrono::milliseconds frequency, uint32_t depth) {
  assert(!std::empty(symbols));
  for (auto &symbol : symbols) {
    auto now = clock::get_realtime<std::chrono::seconds>();
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
    (*connection_).send_text(message);
  }
}

// candlesticks
void MarketData::subscribe(std::string_view const &channel, std::span<Symbol const> const &symbols, std::string_view const &interval) {
  assert(!std::empty(symbols));
  for (auto &symbol : symbols) {
    auto now = clock::get_realtime<std::chrono::seconds>();
    auto message = fmt::format(
        R"({{)"
        R"("time":{},)"
        R"("channel":"{}",)"
        R"("event":"subscribe",)"
        R"("payload":["{}","{}"])"
        R"(}})"sv,
        now.count(),
        channel,
        interval,
        symbol);
    (*connection_).send_text(message);
    // request snapshot
    shared_.time_series_request_queue.emplace_back(symbol);
  }
}

void MarketData::parse(std::string_view const &message) {
  profile_.parse([&]() {
    auto log_message = [&]() { log::warn(R"(*** PLEASE REPORT *** message="{}")"sv, message); };
    try {
      TraceInfo trace_info;
      if (!json::Parser::dispatch(*this, message, decode_buffer_, trace_info, shared_.settings.experimental.allow_unknown_event_types)) {
        log_message();
      }
    } catch (...) {
      log_message();
      utils::exceptions::Unhandled::terminate();
    }
  });
}

void MarketData::operator()(Trace<json::Subscribe> const &event) {
  profile_.subscribe([&]() {
    auto &[trace_info, subscribe] = event;
    log::info<3>("subscribe={}"sv, subscribe);
  });
}

void MarketData::operator()(Trace<json::Tickers> const &event) {
  profile_.tickers([&]() {
    auto &[trace_info, tickers] = event;
    log::info<3>("tickers={}"sv, tickers);
    (*connection_).touch(trace_info.source_receive_time);
    for (auto &item : tickers.result) {
      std::array<Statistics, 6> statistics{{
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
      }};
      auto statistics_update = StatisticsUpdate{
          .stream_id = stream_id_,
          .exchange = shared_.settings.exchange,
          .symbol = item.contract,
          .statistics = statistics,
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = {},
          .exchange_sequence = {},
          .sending_time_utc = tickers.time_ms,
      };
      create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
    }
  });
}

void MarketData::operator()(Trace<json::Trades> const &event) {
  profile_.trades([&]() {
    auto &[trace_info, trades] = event;
    log::info<3>("trades={}"sv, trades);
    (*connection_).touch(trace_info.source_receive_time);
    auto &result = trades.result;
    auto &trades_2 = shared_.get_trades();
    auto emplace_back = [](auto &result, auto &value) {
      auto side = utils::compare(value.size, 0.0) == std::strong_ordering::less ? Side::SELL : Side::BUY;
      auto trade = Trade{
          .side = side,
          .price = value.price,
          .quantity = std::fabs(value.size),
          .trade_id = {},
          .taker_order_id = {},
          .maker_order_id = {},
      };
      utils::charconv::to_string(std::back_inserter(trade.trade_id), value.id);
      result.emplace_back(std::move(trade));
    };
    std::string_view contract;
    decltype(json::TradesItem::create_time_ms) timestamp = {};
    for (auto &item : result) {
      if (item.contract != contract) {
        if (!std::empty(contract) && !std::empty(trades_2)) {
          auto trade_summary = TradeSummary{
              .stream_id = stream_id_,
              .exchange = shared_.settings.exchange,
              .symbol = contract,
              .trades = trades_2,
              .exchange_time_utc = timestamp,
              .exchange_sequence = {},
              .sending_time_utc = trades.time_ms,
          };
          create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
          trades_2.clear();
        }
        contract = item.contract;
        timestamp = {};
      }
      emplace_back(trades_2, item);
      utils::update_max(timestamp, item.create_time_ms);
    }
    if (!std::empty(trades_2)) {
      auto trade_summary = TradeSummary{
          .stream_id = stream_id_,
          .exchange = shared_.settings.exchange,
          .symbol = contract,
          .trades = trades_2,
          .exchange_time_utc = timestamp,
          .exchange_sequence = {},
          .sending_time_utc = trades.time_ms,
      };
      create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
    }
  });
}

void MarketData::operator()(Trace<json::BookTicker> const &event) {
  profile_.book_ticker([&]() {
    auto &[trace_info, book_ticker] = event;
    log::info<3>("book_ticker={}"sv, book_ticker);
    (*connection_).touch(trace_info.source_receive_time);
    auto &result = book_ticker.result;
    auto top_of_book = TopOfBook{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = result.contract,
        .layer{
            .bid_price = result.best_bid_price,
            .bid_quantity = result.best_bid_size,
            .ask_price = result.best_ask_price,
            .ask_quantity = result.best_ask_size,
        },
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = result.timestamp,
        .exchange_sequence = {},
        .sending_time_utc = book_ticker.time_ms,
    };
    create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
  });
}

void MarketData::operator()(Trace<json::OrderBookUpdate> const &event) {
  profile_.order_book_update([&]() {
    auto &[trace_info, order_book_update] = event;
    log::info<3>("order_book_update={}"sv, order_book_update);
    (*connection_).touch(trace_info.source_receive_time);
    auto &result = order_book_update.result;
    auto &symbol = result.symbol;
    auto first_sequence = result.first_update_id;
    auto last_sequence = result.last_update_id;
    auto &sequencer = shared_.mbp_sequencer[symbol];
    auto &mbp = shared_.get_mbp();
    auto emplace_back = [](auto &result, auto &item) {
      auto mbp_update = MBPUpdate{
          .price = item.price,
          .quantity = item.size,
          .implied_quantity = NaN,
          .number_of_orders = {},
          .update_action = {},
          .price_level = {},
      };
      result.emplace_back(std::move(mbp_update));
    };
    for (auto &item : result.bids) {
      emplace_back(mbp.bids, item);
    }
    for (auto &item : result.asks) {
      emplace_back(mbp.asks, item);
    }
    try {
      auto create_update = [&](auto &bids, auto &asks, auto update_type, auto exchange_sequence) -> MarketByPriceUpdate {
        return {
            .stream_id = stream_id_,
            .exchange = shared_.settings.exchange,
            .symbol = symbol,
            .bids = bids,
            .asks = asks,
            .update_type = update_type,
            .exchange_time_utc = result.timestamp,
            .exchange_sequence = exchange_sequence,
            .sending_time_utc = order_book_update.time_ms,
            .price_precision = {},
            .quantity_precision = {},
            .checksum = {},
        };
      };
      auto publish_update = [&](auto &bids, auto &asks) {
        auto market_by_price_update = create_update(bids, asks, UpdateType::INCREMENTAL, last_sequence);
        create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true);
      };
      auto publish_snapshot = [&](auto &bids, auto &asks, auto sequence, auto retries, auto delay) {
        log::info(
            R"(DEBUG PUBLISH SNAPSHOT symbol="{}", sequence={}, retries={}, delay={})"sv,
            symbol,
            sequence,
            retries,
            std::chrono::duration_cast<std::chrono::milliseconds>(delay));
        auto market_by_price_update = create_update(bids, asks, UpdateType::SNAPSHOT, sequencer.last_sequence());
        auto apply_updates = [&](auto &market_by_price) { sequencer.apply(market_by_price, sequence, true); };
        Trace event{trace_info, market_by_price_update};
        shared_(event, true, apply_updates);
      };
      auto request_snapshot = [&](auto retries) {
        log::info(R"(DEBUG REQUEST symbol="{}" (retries={}))"sv, symbol, retries);
        if (shared_.settings.ws.mbp_request_max_retries && shared_.settings.ws.mbp_request_max_retries < retries) {
          log::fatal(R"(Unexpected: symbol="{}", retries={})"sv, symbol, retries);
        }
        shared_.depth_request_queue.emplace_back(symbol);
      };
      sequencer(mbp.bids, mbp.asks, first_sequence, last_sequence, first_sequence - 1, publish_update, publish_snapshot, request_snapshot);
    } catch (BadState &) {
      log::warn(R"(RESUBSCRIBE symbol="{}")"sv, symbol);
      sequencer.clear();
      shared_.depth_request_queue.emplace_back(symbol);
    }
  });
}

void MarketData::operator()(Trace<json::Candlesticks> const &event) {
  profile_.candlesticks([&]() {
    auto &[trace_info, candlesticks] = event;
    log::info<3>("candlesticks={}"sv, candlesticks);
    (*connection_).touch(trace_info.source_receive_time);
    std::string_view symbol;
    auto &bars = shared_.bars;
    bars.clear();
    auto helper = [&]() {
      if (std::empty(bars)) {
        return;
      }
      auto time_series_update = TimeSeriesUpdate{
          .stream_id = stream_id_,
          .exchange = shared_.settings.exchange,
          .symbol = symbol,
          .data_source = DataSource::TRADE_SUMMARY,
          .interval = shared_.settings.time_series.interval,
          .origin = Origin::EXCHANGE,
          .bars = bars,
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = candlesticks.time_ms,
      };
      create_trace_and_dispatch(handler_, trace_info, time_series_update, true);
      bars.clear();
    };
    for (auto &item : candlesticks.result) {
      if (item.name != symbol) {
        helper();
        symbol = item.name;
      }
      if (!item.confirmed && !shared_.settings.time_series.realtime) {
        continue;
      }
      auto bar = Bar{
          .begin_time_utc = item.time,
          .confirmed = item.confirmed,
          .open_price = item.open,
          .high_price = item.open,
          .low_price = item.open,
          .close_price = item.open,
          .quantity = item.volume,
          .base_amount = roq::NaN,
          .quote_amount = item.amount,
          .number_of_trades = {},
          .vwap = roq::NaN,
      };
      bars.emplace_back(std::move(bar));
    }
    helper();
  });
}

}  // namespace gate_futures
}  // namespace roq
