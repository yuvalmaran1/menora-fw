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
#include "buzzer.h"
#include "buzzer_notes.h"
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

    uint32_t lit_led_count;     // number of menorah LEDs currently lit, in range [0, LEDSTRIP_NUM_DEVICES]

    MS_SCHEDULER_SLOT_t* btn_readout_slot;
} MENORA_st;

/******************************************************************************
 * Static variables
 *****************************************************************************/
static MENORA_st s_menora = {0};

/* opening phrase of "Ode to Joy" - short and recognizable buzzer demo song */
static const BUZZER_NOTE_st s_menora_song_notes[] =
{
    { NOTE_E4, 300 }, { NOTE_E4, 300 }, { NOTE_F4, 300 }, { NOTE_G4, 300 },
    { NOTE_G4, 300 }, { NOTE_F4, 300 }, { NOTE_E4, 300 }, { NOTE_D4, 300 },
    { NOTE_C4, 300 }, { NOTE_C4, 300 }, { NOTE_D4, 300 }, { NOTE_E4, 300 },
    { NOTE_E4, 450 }, { NOTE_D4, 300 }, { NOTE_D4, 600 },
};

static const BUZZER_SONG_st s_menora_song =
{
    .notes = s_menora_song_notes,
    .num_notes = sizeof(s_menora_song_notes) / sizeof(s_menora_song_notes[0])
};

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
/* turn all menorah LEDs off and reset the lit count */
static void MENORA_leds_all_off(void)
{
    for (uint32_t led_num = 0; led_num < LEDSTRIP_NUM_DEVICES; led_num++)
    {
        LEDSTRIP_set_led(led_num, LEDSTRIP_COLOR_OFF, LEDSTRIP_BLINK_0HZ);
    }

    s_menora.lit_led_count = 0;
}

//_____________________________________________________________________________
/* light button short press: light up one more LED (gaussian-pulsing yellow)
 * on each press; once all are lit, the next press turns them all off */
static void MENORA_light_btn_short_press_cb(void* ctx)
{
    (void)ctx;

    if (s_menora.lit_led_count >= LEDSTRIP_NUM_DEVICES)
    {
        RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Light button - short press, all LEDs lit - turning off");

        MENORA_leds_all_off();
    }
    else
    {
        RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Light button - short press, lighting LED %lu", (unsigned long)s_menora.lit_led_count);

        LEDSTRIP_set_led(s_menora.lit_led_count, LEDSTRIP_COLOR_YELLOW, LEDSTRIP_BLINK_GAUSSIAN_1HZ);
        s_menora.lit_led_count++;
    }
}

//_____________________________________________________________________________
/* light button long press: turn all LEDs off, regardless of current state */
static void MENORA_light_btn_long_press_cb(void* ctx)
{
    (void)ctx;

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Light button - long press, turning all LEDs off");

    MENORA_leds_all_off();
}

//_____________________________________________________________________________
/* music button short press toggles song playback - start it if idle, stop it if playing */
static void MENORA_music_btn_short_press_cb(void* ctx)
{
    (void)ctx;

    if (BUZZER_get_state(&s_menora.buzzer) == BUZZER_STATE_PLAYING)
    {
        RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Music button - short press, stopping song");
        BUZZER_stop(&s_menora.buzzer);
    }
    else
    {
        RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Music button - short press, playing song");
        BUZZER_play(&s_menora.buzzer, &s_menora_song);
    }
}

//_____________________________________________________________________________
static void MENORA_btn_long_press_cb(void* ctx)
{
    const char* btn_name = (const char*)ctx;

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "%s button - long press", btn_name);
}

//_____________________________________________________________________________
static void MENORA_btn_readout_task(void* ctx)
{
    (void)ctx;

    CAPTOUCH_BTN_process(&s_menora.light_btn);
    CAPTOUCH_BTN_process(&s_menora.music_btn);
    CAPTOUCH_BTN_process(&s_menora.mode_btn);

    /* drive the led strip refresh from the same cadence (it self-throttles to LEDSTRIP_REFRESH_TIME_MS) */
    LEDSTRIP_process();
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
        .tim_ch = p_init_config->led_tim_ch,
    };

    LEDSTRIP_init(&ledstrip_cfg);

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Initializing capacitive touch buttons");

    MENORA_captouch_btn_init(&s_menora.light_btn, "Light", p_init_config, p_init_config->tsc_light_btn_channel_io);
    MENORA_captouch_btn_init(&s_menora.music_btn, "Music", p_init_config, p_init_config->tsc_music_btn_channel_io);
    MENORA_captouch_btn_init(&s_menora.mode_btn, "Mode", p_init_config, p_init_config->tsc_mode_btn_channel_io);

    /* music button short press toggles song playback instead of just logging */
    CAPTOUCH_BTN_register_short_press_cb(&s_menora.music_btn, MENORA_music_btn_short_press_cb, NULL);

    /* light button short/long press drives the menorah LED lighting sequence instead of just logging */
    CAPTOUCH_BTN_register_short_press_cb(&s_menora.light_btn, MENORA_light_btn_short_press_cb, NULL);
    CAPTOUCH_BTN_register_long_press_cb(&s_menora.light_btn, MENORA_light_btn_long_press_cb, NULL);

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Initializing scheduler");

    MS_SCHEDULER_INIT_CONFIG_st scheduler_cfg = {
        .timer = p_init_config->scheduler_tim
    };

    MS_SCHEDULER_init(&scheduler_cfg);

    s_menora.btn_readout_slot = MS_SCHEDULER_allocate_slot();
    MS_SCHEDULER_schedule(s_menora.btn_readout_slot, MENORA_btn_readout_task, NULL, MENORA_BTN_READOUT_INTERVAL_MS, true);

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Initializing buzzer");

    BUZZER_INIT_CONFIG_st buzzer_cfg = {
        .tim = p_init_config->buzzer_tim,
        .tim_ch = TIM_CHANNEL_4
    };

    BUZZER_init(&s_menora.buzzer, &buzzer_cfg);

    RTT_LOG_log(RTT_INFO, MENORA_LOG_SOURCE, "Menora initialization complete");
}

//_____________________________________________________________________________
void MENORA_process(void)
{
    TASK_HANDLER_handle();
}