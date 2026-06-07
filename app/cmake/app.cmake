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
        "${APP_DIR}/src/irr_node.c"
        "${APP_DIR}/src/irr_scheduler.c"
        "${APP_DIR}/src/irr_lora.c"
        "${APP_DIR}/src/irr_nfc.c"
        "${APP_DIR}/src/irr_cli.c"
        "${APP_DIR}/src/irr_debug.c"
        "${APP_DIR}/src/irr_pwr.c"
        "${APP_DIR}/src/irr_fwu.c"
        "${APP_DIR}/src/irr_lora_msg.c"
        "${APP_DIR}/src/flash_if.c"
        "${APP_DIR}/src/frag_decoder_if.c"
        "${APP_DIR}/src/fw_update_agent.c"
        "${APP_DIR}/../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages/LmhpClockSync.c"
        "${APP_DIR}/../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages/LmhpRemoteMcastSetup.c"
        "${APP_DIR}/../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages/LmhpFragmentation.c"
        "${APP_DIR}/../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages/FragDecoder.c"
        "${APP_DIR}/../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages/LmhpFirmwareManagement.c"
    )

endfunction()