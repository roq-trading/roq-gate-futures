include(RoqAutogen)

set(TARGET_NAME ${PROJECT_NAME}-test)

set(SOURCES
    json_book_ticker.cpp
    json_contracts.cpp
    json_currencies.cpp
    json_message.cpp
    json_order_book.cpp
    json_order_book_update.cpp
    json_positions.cpp
    json_subscribe.cpp
    json_tickers.cpp
    json_trade_login.cpp
    json_trade_orders.cpp
    json_trade_subscribe.cpp
    main.cpp)

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME})

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-json roq-core::roq-core roq-io::roq-io roq-logging::roq-logging Catch2::Catch2)

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
