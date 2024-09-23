set(TARGET_NAME ${PROJECT_NAME}-json)

set(SOURCES map.cpp parser.cpp trade_parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    accounts_history.json
    accounts.json
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
    orders_item.json
    orders.json
    order_status.json
    positions_item.json
    positions.json
    price_size.json
    status.json
    subscribe.json
    subscribe_result.json
    tickers_item.json
    tickers.json
    tif.json
    trade_error.json
    trade_header.json
    trade_login_data.json
    trade_login_data_result.json
    trade_login.json
    trade_message.json
    trade_result.json
    trades2_item.json
    trades2.json
    trades_item.json
    trades.json)

roq_autogen_hpp(
  OUTPUT
  AUTOGEN_HEADERS
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen_cpp(
  OUTPUT
  AUTOGEN_SOURCES
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME} ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library(${TARGET_NAME} OBJECT ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies(${TARGET_NAME} autogen-json-headers)

target_link_libraries(${TARGET_NAME} PRIVATE roq-core::roq-core roq-utils::roq-utils roq-logging::roq-logging fmt::fmt)
