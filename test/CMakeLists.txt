set(TARGET_NAME "${PROJECT_NAME}-test")

set(SOURCES
    json_book_ticker.cpp
    json_currencies.cpp
    json_contracts.cpp
    json_message.cpp
    json_subscribe.cpp
    json_tickers.cpp
    main.cpp)

add_executable("${TARGET_NAME}" ${SOURCES})

target_link_libraries("${TARGET_NAME}" "${PROJECT_NAME}-json" roq-logging::roq-logging gtest_main
                      ${RT_LIBRARIES})

set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS_RELEASE -s)

# run

add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")
