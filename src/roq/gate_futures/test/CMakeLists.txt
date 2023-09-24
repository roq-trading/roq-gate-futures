include(RoqAutogen)

set(TARGET_NAME ${PROJECT_NAME}-test)

set(SOURCES
    json_book_ticker.cpp
    json_currencies.cpp
    json_contracts.cpp
    json_message.cpp
    json_order_book.cpp
    json_order_book_update.cpp
    json_subscribe.cpp
    json_tickers.cpp
    main.cpp)

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME})

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(
  ${TARGET_NAME}
  ${PROJECT_NAME}-json
  roq-core::roq-core
  roq-io::roq-io
  roq-logging::roq-logging
  aklomp::base64
  Catch2::Catch2
  ${RT_LIBRARIES})

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
