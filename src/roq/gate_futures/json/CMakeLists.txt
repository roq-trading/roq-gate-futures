set(TARGET_NAME ${PROJECT_NAME}-json)

set(SOURCES map.cpp parser.cpp trade_parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    accounts_history.json
    accounts.json
    balances_item.json
    book_ticker.json
    book_ticker_result.json
    channel.json
    contracts_item.json
    contracts.json
    contract_type.json
    currencies_item.json
    currencies.json
    event.json
    finish_as.json
    mark_type.json
    message.json
    order_book.json
    order_book_update.json
    order_book_update_result.json
    orders_item.json
    order_status.json
    positions_item.json
    positions.json
    price_size.json
    role.json
    status.json
    subscribe.json
    subscribe_result.json
    tickers_item.json
    tickers.json
    tif.json
    trade_balances.json
    trade_error.json
    trade_header.json
    trade_login_data.json
    trade_login_data_result.json
    trade_login.json
    trade_message.json
    trade_order_amend_data.json
    trade_order_amend_data_result.json
    trade_order_amend.json
    trade_order_cancel_cp_data.json
    trade_order_cancel_cp_data_result.json
    trade_order_cancel_cp.json
    trade_order_cancel_data.json
    trade_order_cancel_data_result.json
    trade_order_cancel.json
    trade_order_list_data_error.json
    trade_order_list_data.json
    trade_order_list_data_result_item.json
    trade_order_list.json
    trade_order_place_data_errs.json
    trade_order_place_data.json
    trade_order_place_data_result.json
    trade_order_place.json
    trade_orders.json
    trade_positions.json
    trades_item.json
    trades.json
    trade_subscribe.json
    trade_subscribe_result.json
    trade_trades.json
    trade_trades_result_item.json
    user_trades_item.json
    user_trades.json)

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
