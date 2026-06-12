/**************************************************************************//**
 * @file      theme.c
 * @brief     Application theme management
 *
 * @author    ymaran
 * @date      11.6.2026
 *****************************************************************************/
/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stddef.h>
#include "theme.h"
#include "ms_scheduler.h"
#include "rtt_log.h"

/******************************************************************************
 * Constants
 *****************************************************************************/
#define THEME_LOG_SOURCE "THEME"

/******************************************************************************
 * Data types
 *****************************************************************************/
typedef struct
{
    BUZZER_st* buzzer;  // owned and initialized by the application

    const THEME_st* themes[THEME_NUM_IDS];  // registered themes, indexed by id (NULL if unregistered)
    const THEME_st* active;                 // currently active theme, never NULL

    GPIO_TypeDef* id0_port;
    uint16_t id0_pin;
    GPIO_TypeDef* id1_port;
    uint16_t id1_pin;
    GPIO_TypeDef* id2_port;
    uint16_t id2_pin;

    uint32_t lit_led_count;     // number of theme LEDs currently lit, in range [0, active->num_leds]
    uint32_t song_index;        // cycles through active->songs[] on each new playback

    MS_SCHEDULER_SLOT_t* detect_slot;
} THEME_PRIVATE_st;

/******************************************************************************
 * Static variables
 *****************************************************************************/
static THEME_PRIVATE_st s_theme = {0};

/*!< built-in "no theme" instance, always registered at @ref THEME_NULL_ID */
static const THEME_st s_theme_null =
{
    .id = THEME_NULL_ID,
    .name = "NONE",
    .num_leds = 0,
    .led_color = LEDSTRIP_COLOR_OFF,
    .led_pattern = LEDSTRIP_BLINK_0HZ,
    .songs = NULL,
    .num_songs = 0,
};

/******************************************************************************
 * Static functions
 *****************************************************************************/
//_____________________________________________________________________________
/* read the 3-bit theme ID from the ID2:ID1:ID0 GPIOs */
static uint8_t THEME_read_id(void)
{
    uint8_t id0 = (HAL_GPIO_ReadPin(s_theme.id0_port, s_theme.id0_pin) == GPIO_PIN_SET) ? 1u : 0u;
    uint8_t id1 = (HAL_GPIO_ReadPin(s_theme.id1_port, s_theme.id1_pin) == GPIO_PIN_SET) ? 1u : 0u;
    uint8_t id2 = (HAL_GPIO_ReadPin(s_theme.id2_port, s_theme.id2_pin) == GPIO_PIN_SET) ? 1u : 0u;

    return (uint8_t)((id2 << 2) | (id1 << 1) | id0);
}

//_____________________________________________________________________________
/* turn all menorah LEDs off */
static void THEME_leds_all_off(void)
{
    for (uint32_t led_num = 0; led_num < LEDSTRIP_NUM_DEVICES; led_num++)
    {
        LEDSTRIP_set_led(led_num, LEDSTRIP_COLOR_OFF, LEDSTRIP_BLINK_0HZ);
    }
}

//_____________________________________________________________________________
/* switch the active theme - turns off all LEDs, stops music and resets state
 * machines, as required for hot-swapping between themes */
static void THEME_switch(const THEME_st* new_theme)
{
    RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Switching theme: %s -> %s", s_theme.active->name, new_theme->name);

    THEME_leds_all_off();
    BUZZER_stop(s_theme.buzzer);

    s_theme.lit_led_count = 0;
    s_theme.song_index = 0;
    s_theme.active = new_theme;
}

//_____________________________________________________________________________
/* periodically sample the ID GPIOs and hot-swap the active theme on change */
static void THEME_detect_task(void* ctx)
{
    (void)ctx;

    uint8_t id = THEME_read_id();
    const THEME_st* detected = (s_theme.themes[id] != NULL) ? s_theme.themes[id] : &s_theme_null;

    if (detected != s_theme.active)
    {
        THEME_switch(detected);
    }
}

/******************************************************************************
 * Public functions
 *****************************************************************************/
//_____________________________________________________________________________
THEME_STATUS_t THEME_init(THEME_INIT_CONFIG_st* p_init_config)
{
    if (p_init_config == NULL)
    {
        return THEME_STATUS_INVALID_ARG;
    }

    s_theme.buzzer = p_init_config->buzzer;

    s_theme.id0_port = p_init_config->id0_port;
    s_theme.id0_pin = p_init_config->id0_pin;
    s_theme.id1_port = p_init_config->id1_port;
    s_theme.id1_pin = p_init_config->id1_pin;
    s_theme.id2_port = p_init_config->id2_port;
    s_theme.id2_pin = p_init_config->id2_pin;

    s_theme.themes[THEME_NULL_ID] = &s_theme_null;
    s_theme.active = &s_theme_null;

    /* select the initial theme immediately, then keep sampling periodically */
    uint8_t id = THEME_read_id();
    s_theme.active = (s_theme.themes[id] != NULL) ? s_theme.themes[id] : &s_theme_null;
    RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Theme ID %u - active theme: %s", (unsigned int)id, s_theme.active->name);

    s_theme.detect_slot = MS_SCHEDULER_allocate_slot();
    MS_SCHEDULER_schedule(s_theme.detect_slot, THEME_detect_task, NULL, THEME_DETECT_INTERVAL_MS, true);

    return THEME_STATUS_OK;
}

//_____________________________________________________________________________
THEME_STATUS_t THEME_register(const THEME_st* p_theme)
{
    if (p_theme == NULL || p_theme->id == THEME_NULL_ID || p_theme->id >= THEME_NUM_IDS)
    {
        return THEME_STATUS_INVALID_ARG;
    }

    s_theme.themes[p_theme->id] = p_theme;

    return THEME_STATUS_OK;
}

//_____________________________________________________________________________
const THEME_st* THEME_get_active(void)
{
    return s_theme.active;
}

//_____________________________________________________________________________
void THEME_light_next(void)
{
    if (s_theme.lit_led_count >= s_theme.active->num_leds)
    {
        RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Light next - all LEDs lit - turning off");

        THEME_leds_all_off();
        s_theme.lit_led_count = 0;
    }
    else
    {
        RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Light next - lighting LED %lu", (unsigned long)s_theme.lit_led_count);

        LEDSTRIP_set_led(s_theme.lit_led_count, s_theme.active->led_color, s_theme.active->led_pattern);
        s_theme.lit_led_count++;
    }
}

//_____________________________________________________________________________
void THEME_light_reset(void)
{
    RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Light reset - turning all LEDs off");

    THEME_leds_all_off();
    s_theme.lit_led_count = 0;
}

//_____________________________________________________________________________
void THEME_music_play_next(void)
{
    if (s_theme.active->num_songs == 0)
    {
        return;
    }

    const BUZZER_SONG_st* song = s_theme.active->songs[s_theme.song_index % s_theme.active->num_songs];
    s_theme.song_index++;
    RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Music play next - playing song %lu",
                (unsigned long)(s_theme.song_index % s_theme.active->num_songs));
    BUZZER_play(s_theme.buzzer, song);
}

//_____________________________________________________________________________
void THEME_music_stop(void)
{
    RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Music stop");
    BUZZER_stop(s_theme.buzzer);
}

//_____________________________________________________________________________
bool THEME_music_is_playing(void)
{
    return BUZZER_get_state(s_theme.buzzer) == BUZZER_STATE_PLAYING;
}

//_____________________________________________________________________________
void THEME_process(void)
{
    LEDSTRIP_process();
}
