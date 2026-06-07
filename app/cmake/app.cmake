set(APP_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

function(add_app TARGET_NAME)

    set(APP_DIR "${APP_CMAKE_DIR}/..")

    target_compile_definitions(
        ${TARGET_NAME} PRIVATE
    )

    target_include_directories(
        ${TARGET_NAME} PRIVATE
        "${APP_DIR}/inc"
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
        "${APP_DIR}/src/menora.c"
    )

    configure_file(${APP_CMAKE_DIR}/version.h.in ${APP_DIR}/inc/version.h @ONLY)

endfunction()