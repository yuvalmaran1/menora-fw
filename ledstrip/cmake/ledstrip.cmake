set(LEDSTRIP_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

function(add_ledstrip TARGET_NAME)

    set(LEDSTRIP_DIR "${LEDSTRIP_CMAKE_DIR}/..")

    message(STATUS "adding ledstrip to target ${TARGET_NAME}")

    target_compile_definitions(
        ${TARGET_NAME} PRIVATE
    )

    target_include_directories(
        ${TARGET_NAME} PRIVATE
        "${LEDSTRIP_DIR}/inc"
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
        "${LEDSTRIP_DIR}/src/ledstrip.c"
    )

endfunction()