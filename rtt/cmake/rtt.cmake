set(RTT_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

function(add_rtt TARGET_NAME)

    set(RTT_DIR "${RTT_CMAKE_DIR}/..")

    message(STATUS "adding rtt to target ${TARGET_NAME}")

    target_compile_definitions(
        ${TARGET_NAME} PRIVATE
    )

    target_include_directories(
        ${TARGET_NAME} PRIVATE
        "${RTT_DIR}/inc"
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
        # "${RTT_DIR}/src/SEGGER_RTT_Syscalls_GCC.c"
        "${RTT_DIR}/src/SEGGER_RTT.c"
        "${RTT_DIR}/src/SEGGER_RTT_printf.c"
        "${RTT_DIR}/src/rtt_log.c"
    )

endfunction()