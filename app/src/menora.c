/**************************************************************************//**
 * @file      menora.c
 * @brief     Application implementation
 *
 * @author    ymaran
 * @date      25.7.2024
 *****************************************************************************/
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "menora.h"
#include "ledstrip.h"
#include "task_handler.h"
#include "ms_scheduler.h"
#include "captouch_btn.h"
#include "rtt_log.h"
#include "version.h"

/******************************************************************************
 * Constants
 *****************************************************************************/
#define MENORA_LOG_SOURCE   "MENORA"

#define MENORA_TSC_TOUCH_THRESHOLD  (1200)  // acquisition count above which an electrode is considered touched

/******************************************************************************
 * Data types
 *****************************************************************************/
typedef struct
{
    CAPTOUCH_BTN_st light_btn;
    CAPTOUCH_BTN_st music_btn;
    CAPTOUCH_BTN_st mode_btn;
} MENORA_st;

/******************************************************************************
 * Static variables
 *****************************************************************************/
static MENORA_st s_menora = {0};

/******************************************************************************
 * Static functions
 *****************************************************************************/
//_____________________________________________________________________________
static void MENORA_btn_short_press_cb(void* ctx)
{
    const char* btn_name = (const char*)ctx;

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "%s button - short press", btn_name);
}

//_____________________________________________________________________________
static void MENORA_btn_long_press_cb(void* ctx)
{
    const char* btn_name = (const char*)ctx;

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "%s button - long press", btn_name);
}

//_____________________________________________________________________________
static void MENORA_captouch_btn_init(CAPTOUCH_BTN_st* p_btn, const char* name, MENORA_INIT_CONFIG_st* p_init_config, uint32_t channel_io)
{
    CAPTOUCH_BTN_INIT_CONFIG_st btn_cfg = {
        .tsc = p_init_config->tsc,
        .group_index = p_init_config->tsc_btn_group_index,
        .channel_io = channel_io,
        .sampling_io = p_init_config->tsc_btn_sampling_io,
        .touch_threshold = MENORA_TSC_TOUCH_THRESHOLD,
    };

    CAPTOUCH_BTN_init(p_btn, &btn_cfg);
    CAPTOUCH_BTN_register_short_press_cb(p_btn, MENORA_btn_short_press_cb, (void*)name);
    CAPTOUCH_BTN_register_long_press_cb(p_btn, MENORA_btn_long_press_cb, (void*)name);
}

//_____________________________________________________________________________
/******************************************************************************
 * Public functions
 *****************************************************************************/
//_____________________________________________________________________________
void MENORA_init(MENORA_INIT_CONFIG_st* p_init_config)
{
    /* bring up logging first so the rest of the init sequence is observable */
    RTT_LOG_init();

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Menora starting up - version %s", PROJECT_VERSION);
    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Initializing task handler");

    TASK_HANDLER_init();

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Initializing led strip");

    LEDSTRIP_INIT_CONFIG_st ledstrip_cfg = {
        .gpio_led_data = p_init_config->led_data_port,
        .gpio_led_data_pin = p_init_config->led_data_pin,
        .gpio_led_en = p_init_config->led_en_port,
        .gpio_led_en_pin = p_init_config->led_en_pin,
        .tim = p_init_config->led_tim,
        .tim_ch = TIM_CHANNEL_1
    };

    LEDSTRIP_init(&ledstrip_cfg);

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Initializing capacitive touch buttons");

    MENORA_captouch_btn_init(&s_menora.light_btn, "Light", p_init_config, p_init_config->tsc_light_btn_channel_io);
    MENORA_captouch_btn_init(&s_menora.music_btn, "Music", p_init_config, p_init_config->tsc_music_btn_channel_io);
    MENORA_captouch_btn_init(&s_menora.mode_btn, "Mode", p_init_config, p_init_config->tsc_mode_btn_channel_io);

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Menora initialization complete");
}

//_____________________________________________________________________________
void MENORA_process(void)
{
    TASK_HANDLER_handle();

    CAPTOUCH_BTN_process(&s_menora.light_btn);
    CAPTOUCH_BTN_process(&s_menora.music_btn);
    CAPTOUCH_BTN_process(&s_menora.mode_btn);
}