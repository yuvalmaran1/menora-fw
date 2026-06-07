set(UTIL_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

function(add_util TARGET_NAME)

    set(UTIL_DIR "${UTIL_CMAKE_DIR}/..")

    message(STATUS "adding util to target ${TARGET_NAME}")

    target_compile_definitions(
        ${TARGET_NAME} PRIVATE
    )

    target_include_directories(
        ${TARGET_NAME} PRIVATE
        "${UTIL_DIR}/inc"
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
        "${UTIL_DIR}/src/ms_scheduler.c"
        "${UTIL_DIR}/src/scheduler.c"
        "${UTIL_DIR}/src/task_handler.c"
    )

endfunction()