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
#include "task_handler.h"
#include "ms_scheduler.h"
#include "captouch_btn.h"
#include "ledstrip.h"
#include "buzzer.h"
#include "theme.h"
#include "theme_hanuka.h"
#include "theme_shabbat.h"
#include "rtt_log.h"
#include "version.h"

/******************************************************************************
 * Constants
 *****************************************************************************/
#define MENORA_LOG_SOURCE   "MENORA"

#define MENORA_TSC_TOUCH_THRESHOLD  (1200)  // acquisition count above which an electrode is considered touched

#define MENORA_BTN_READOUT_INTERVAL_MS  (10)

/******************************************************************************
 * Data types
 *****************************************************************************/
typedef struct
{
    CAPTOUCH_BTN_st light_btn;
    CAPTOUCH_BTN_st music_btn;
    CAPTOUCH_BTN_st mode_btn;

    BUZZER_st buzzer;

    MS_SCHEDULER_SLOT_t* btn_readout_slot;
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
/* light button short press: light up the next LED of the active theme, or
 * turn them all off once they're all lit */
static void MENORA_light_btn_short_press_cb(void* ctx)
{
    (void)ctx;

    THEME_light_next();
}

//_____________________________________________________________________________
/* light button long press: turn all LEDs off, regardless of current state */
static void MENORA_light_btn_long_press_cb(void* ctx)
{
    (void)ctx;

    THEME_light_reset();
}

//_____________________________________________________________________________
/* music button short press: toggle song playback - start it if idle, stop it if playing */
static void MENORA_music_btn_short_press_cb(void* ctx)
{
    (void)ctx;

    if (THEME_music_is_playing())
    {
        THEME_music_stop();
    }
    else
    {
        THEME_music_play_next();
    }
}

//_____________________________________________________________________________
static void MENORA_btn_readout_task(void* ctx)
{
    (void)ctx;

    CAPTOUCH_BTN_process(&s_menora.light_btn);
    CAPTOUCH_BTN_process(&s_menora.music_btn);
    CAPTOUCH_BTN_process(&s_menora.mode_btn);

    /* drive the theme module (e.g. led strip refresh) from the same cadence */
    THEME_process();
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

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Initializing scheduler");

    MS_SCHEDULER_INIT_CONFIG_st scheduler_cfg = {
        .timer = p_init_config->scheduler_tim
    };

    MS_SCHEDULER_init(&scheduler_cfg);

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Initializing led strip");

    LEDSTRIP_INIT_CONFIG_st ledstrip_cfg = {
        .gpio_led_data = p_init_config->led_data_port,
        .gpio_led_data_pin = p_init_config->led_data_pin,
        .gpio_led_en = p_init_config->led_en_port,
        .gpio_led_en_pin = p_init_config->led_en_pin,
        .tim = p_init_config->led_tim,
        .tim_ch = p_init_config->led_tim_ch,
    };

    LEDSTRIP_init(&ledstrip_cfg);

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Initializing buzzer");

    BUZZER_INIT_CONFIG_st buzzer_cfg = {
        .tim = p_init_config->buzzer_tim,
        .tim_ch = TIM_CHANNEL_4,
    };

    BUZZER_init(&s_menora.buzzer, &buzzer_cfg);

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Registering themes");

    THEME_HANUKA_register();
    THEME_SHABBAT_register();

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Initializing theme module");

    THEME_INIT_CONFIG_st theme_cfg = {
        .buzzer = &s_menora.buzzer,

        .id0_port = p_init_config->id0_port,
        .id0_pin = p_init_config->id0_pin,
        .id1_port = p_init_config->id1_port,
        .id1_pin = p_init_config->id1_pin,
        .id2_port = p_init_config->id2_port,
        .id2_pin = p_init_config->id2_pin,
    };

    THEME_init(&theme_cfg);

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Initializing capacitive touch buttons");

    MENORA_captouch_btn_init(&s_menora.light_btn, "Light", p_init_config, p_init_config->tsc_light_btn_channel_io);
    MENORA_captouch_btn_init(&s_menora.music_btn, "Music", p_init_config, p_init_config->tsc_music_btn_channel_io);
    MENORA_captouch_btn_init(&s_menora.mode_btn, "Mode", p_init_config, p_init_config->tsc_mode_btn_channel_io);

    /* light/music button presses drive the active theme's LEDs and music playback */
    CAPTOUCH_BTN_register_short_press_cb(&s_menora.light_btn, MENORA_light_btn_short_press_cb, NULL);
    CAPTOUCH_BTN_register_long_press_cb(&s_menora.light_btn, MENORA_light_btn_long_press_cb, NULL);
    CAPTOUCH_BTN_register_short_press_cb(&s_menora.music_btn, MENORA_music_btn_short_press_cb, NULL);

    s_menora.btn_readout_slot = MS_SCHEDULER_allocate_slot();
    MS_SCHEDULER_schedule(s_menora.btn_readout_slot, MENORA_btn_readout_task, NULL, MENORA_BTN_READOUT_INTERVAL_MS, true);

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Menora initialization complete");
}

//_____________________________________________________________________________
void MENORA_process(void)
{
    TASK_HANDLER_handle();
}
