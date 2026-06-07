set(CAPTOUCH_BTN_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

function(add_captouch_btn TARGET_NAME)

    set(CAPTOUCH_BTN_DIR "${CAPTOUCH_BTN_CMAKE_DIR}/..")

    message(STATUS "adding captouch_btn to target ${TARGET_NAME}")

    target_compile_definitions(
        ${TARGET_NAME} PRIVATE
    )

    target_include_directories(
        ${TARGET_NAME} PRIVATE
        "${CAPTOUCH_BTN_DIR}/inc"
    )

    target_compile_options(
        ${TARGET_NAME} PRIVATE
    )

    target_link_libraries(
        ${TARGET_NAME} PRIVATE
    )

    target_link_directories(
        ${TARGET_NAME} PRIVATE
    )

    target_link_options(
        ${TARGET_NAME} PRIVATE
    )

    target_sources(
        ${TARGET_NAME} PRIVATE
        "${CAPTOUCH_BTN_DIR}/src/captouch_btn.c"
    )

endfunction()
