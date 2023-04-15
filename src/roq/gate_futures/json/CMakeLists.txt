set(TARGET_NAME "${PROJECT_NAME}-json")

set(SOURCES parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    book_ticker.json
    book_ticker_result.json
    channel.json
    contracts_item.json
    contracts.json
    contract_type.json
    currencies_item.json
    currencies.json
    event.json
    mark_type.json
    message.json
    order_book.json
    order_book_update.json
    order_book_update_result.json
    price_size.json
    subscribe.json
    subscribe_result.json
    tickers_item.json
    tickers.json
    trades_item.json
    trades.json)

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  "roq/gate_futures/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "hpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen(
  OUTPUT
  AUTOGEN_SOURCES
  NAMESPACE
  "roq/gate_futures/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "cpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

roq_gitignore(OUTPUT ".gitignore" SOURCES "${TARGET_NAME}" ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library("${TARGET_NAME}" OBJECT ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies("${TARGET_NAME}" autogen-json-headers)

target_link_libraries("${TARGET_NAME}" roq-core::roq-core roq-logging::roq-logging fmt::fmt)
