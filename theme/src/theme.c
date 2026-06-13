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
#include "buzzer_notes.h"
#include "ms_scheduler.h"
#include "rtt_log.h"

/******************************************************************************
 * Constants
 *****************************************************************************/
#define THEME_LOG_SOURCE "THEME"

/*!< intro animation played whenever a non-NULL theme becomes active (startup or
 * hot-swap): a rainbow wipes across the strip and then shimmers before clearing */
#define THEME_ANIM_START_DELAY_MS (500)                                          // quiet gap before the animation begins
#define THEME_ANIM_DURATION_MS    (3000)                                         // total visible animation length
#define THEME_ANIM_STEP_MS        (200)                                          // time between animation frames
#define THEME_ANIM_TOTAL_STEPS    (THEME_ANIM_DURATION_MS / THEME_ANIM_STEP_MS)  // number of frames

/*!< pause between a song finishing and the next one auto-starting */
#define THEME_MUSIC_GAP_MS        (3000)

/******************************************************************************
 * Data types
 *****************************************************************************/
/* music playback state machine */
typedef enum
{
    THEME_MUSIC_IDLE,     // nothing playing; the next start begins from the first song
    THEME_MUSIC_INTRO,    // the intro jingle is playing (not part of the playlist)
    THEME_MUSIC_PLAYING,  // a song is currently sounding
    THEME_MUSIC_GAP,      // a song finished; waiting THEME_MUSIC_GAP_MS before auto-advancing
} THEME_MUSIC_STATE_en;

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
    uint32_t song_index;        // index of the currently playing / last played song in active->songs[]
    THEME_MUSIC_STATE_en music_state;

    MS_SCHEDULER_SLOT_t* detect_slot;
    MS_SCHEDULER_SLOT_t* music_slot;  // drives the inter-song gap before an auto-advance

    MS_SCHEDULER_SLOT_t* anim_slot;  // drives the intro animation frames
    uint32_t anim_step;              // current intro animation frame index
    uint32_t anim_epoch;            // bumped on each (re)start so stale queued frames self-cancel
    bool animating;                  // true while the intro animation is running
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

/*!< vibrant rainbow palette cycled across the strip during the intro animation */
static const LEDSTRIP_COLOR_en s_anim_palette[] = {
    LEDSTRIP_COLOR_RED,
    LEDSTRIP_COLOR_ORANGE,
    LEDSTRIP_COLOR_YELLOW,
    LEDSTRIP_COLOR_GREEN,
    LEDSTRIP_COLOR_CYAN,
    LEDSTRIP_COLOR_BLUE,
    LEDSTRIP_COLOR_WHITE,
};
#define THEME_ANIM_PALETTE_LEN (sizeof(s_anim_palette) / sizeof(s_anim_palette[0]))

/*!< short, bright intro jingle played alongside the intro animation: an
 * ascending major arpeggio with a sparkle, landing on the tonic (~1.3s) */
static const BUZZER_NOTE_st s_intro_notes[] =
{
    { NOTE_C5, 100 }, { NOTE_E5, 100 }, { NOTE_G5, 100 }, { NOTE_C6, 200 },
    { BUZZER_NOTE_REST, 40 },
    { NOTE_E6, 100 }, { NOTE_C6, 100 }, { NOTE_G5, 100 }, { NOTE_C6, 400 },
};
static const BUZZER_SONG_st s_theme_intro_song =
{
    .name = "Intro",
    .notes = s_intro_notes,
    .num_notes = sizeof(s_intro_notes) / sizeof(s_intro_notes[0]),
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
        LEDSTRIP_set_led(led_num, LEDSTRIP_COLOR_OFF, LEDSTRIP_BLINK_0HZ, LEDSTRIP_PHASE_SYNC);
    }
}

//_____________________________________________________________________________
/* render one frame of the intro animation and schedule the next one. A rainbow
 * wipes in (one LED revealed per frame) and the whole palette rotates across the
 * strip, so every lit LED shows a different, flowing color. Once all frames have
 * played the strip is cleared and normal operation resumes. */
static void THEME_anim_task(void* ctx)
{
    uint32_t step = s_theme.anim_step;
    uint32_t num = s_theme.active->num_leds;

    /* ignore frames left over from a superseded animation (e.g. a theme switch
     * happened while a frame was still queued) */
    if ((uint32_t)(uintptr_t)ctx == s_theme.anim_epoch)
    {
        if ((step >= THEME_ANIM_TOTAL_STEPS) || (num == 0))
        {
            /* animation done - clear the strip and hand control back to normal play */
            THEME_leds_all_off();
            s_theme.lit_led_count = 0;
            s_theme.animating = false;
        }
        else
        {
            /* LEDs revealed so far - one more per frame until the strip is full */
            uint32_t revealed = ((step + 1) < num) ? (step + 1) : num;

            for (uint32_t i = 0; i < num; i++)
            {
                if (i < revealed)
                {
                    LEDSTRIP_COLOR_en color = s_anim_palette[(i + step) % THEME_ANIM_PALETTE_LEN];
                    LEDSTRIP_set_led(i, color, LEDSTRIP_BLINK_0HZ, LEDSTRIP_PHASE_SYNC);
                }
                else
                {
                    LEDSTRIP_set_led(i, LEDSTRIP_COLOR_OFF, LEDSTRIP_BLINK_0HZ, LEDSTRIP_PHASE_SYNC);
                }
            }

            s_theme.anim_step++;

            MS_SCHEDULER_schedule(s_theme.anim_slot, THEME_anim_task, ctx, THEME_ANIM_STEP_MS, false);
        }
    }
}

//_____________________________________________________________________________
/* kick off the intro animation for a freshly activated theme (no-op for the
 * NULL theme, which has no LEDs). The first frame fires after a short delay. */
static void THEME_anim_start(const THEME_st* theme)
{
    if ((theme != NULL) && (theme->num_leds != 0) && (s_theme.anim_slot != NULL))
    {
        /* cancel any in-flight animation and invalidate its queued frames */
        MS_SCHEDULER_abort(s_theme.anim_slot);
        s_theme.anim_epoch++;
        s_theme.anim_step = 0;
        s_theme.animating = true;

        MS_SCHEDULER_schedule(s_theme.anim_slot, THEME_anim_task, (void*)(uintptr_t)s_theme.anim_epoch, THEME_ANIM_START_DELAY_MS, false);
    }
}

//_____________________________________________________________________________
/* greet a freshly activated theme: play the intro animation and jingle together
 * (no-op for the NULL theme). The jingle plays independently of the playlist -
 * it does not start or advance it. */
static void THEME_intro_start(const THEME_st* theme)
{
    THEME_anim_start(theme);

    if ((theme != NULL) && (theme->num_leds != 0))
    {
        s_theme.song_index = 0;
        s_theme.music_state = THEME_MUSIC_INTRO;

        RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Music - playing intro jingle");
        BUZZER_play(s_theme.buzzer, &s_theme_intro_song);
    }
}

//_____________________________________________________________________________
/* play the song at the given playlist index and enter the PLAYING state */
static void THEME_music_play_index(uint32_t index)
{
    const BUZZER_SONG_st* song = s_theme.active->songs[index];

    s_theme.song_index = index;
    s_theme.music_state = THEME_MUSIC_PLAYING;

    RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Music - playing song '%s'", song->name);
    BUZZER_play(s_theme.buzzer, song);
}

//_____________________________________________________________________________
/* stop playback, cancel any pending inter-song gap and rewind the playlist so
 * the next start begins from the first song */
static void THEME_music_reset(void)
{
    if (s_theme.music_slot != NULL)
    {
        MS_SCHEDULER_abort(s_theme.music_slot);
    }

    BUZZER_stop(s_theme.buzzer);
    s_theme.song_index = 0;
    s_theme.music_state = THEME_MUSIC_IDLE;
}

//_____________________________________________________________________________
/* advance to the next song; once past the last song, stop and rewind so the
 * next manual start begins from the first song again */
static void THEME_music_advance(void)
{
    /* cancel any pending gap (e.g. when a button press skips the pause) */
    if (s_theme.music_slot != NULL)
    {
        MS_SCHEDULER_abort(s_theme.music_slot);
    }

    uint32_t next = s_theme.song_index + 1;

    if (next < s_theme.active->num_songs)
    {
        THEME_music_play_index(next);
    }
    else
    {
        RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Music - end of playlist, resetting");
        THEME_music_reset();
    }
}

//_____________________________________________________________________________
/* inter-song gap elapsed - roll on to the next song */
static void THEME_music_gap_task(void* ctx)
{
    (void)ctx;

    THEME_music_advance();
}

//_____________________________________________________________________________
/* a song finished on its own: auto-advance after a short pause, unless it was
 * the last song - then just rewind so the next press restarts the playlist */
static void THEME_music_done_cb(void* ctx)
{
    (void)ctx;

    if (s_theme.music_state == THEME_MUSIC_INTRO)
    {
        /* intro jingle finished - go quiet; the playlist hasn't started yet */
        s_theme.music_state = THEME_MUSIC_IDLE;
    }
    else if ((s_theme.song_index + 1) < s_theme.active->num_songs)
    {
        s_theme.music_state = THEME_MUSIC_GAP;
        MS_SCHEDULER_schedule(s_theme.music_slot, THEME_music_gap_task, NULL, THEME_MUSIC_GAP_MS, false);
    }
    else
    {
        RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Music - last song finished, resetting");
        THEME_music_reset();
    }
}

//_____________________________________________________________________________
/* switch the active theme - turns off all LEDs, stops music and resets state
 * machines, as required for hot-swapping between themes */
static void THEME_switch(const THEME_st* new_theme)
{
    RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Switching theme: %s -> %s", s_theme.active->name, new_theme->name);

    THEME_leds_all_off();
    THEME_music_reset();

    s_theme.lit_led_count = 0;
    s_theme.active = new_theme;

    THEME_intro_start(new_theme);
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
    s_theme.music_state = THEME_MUSIC_IDLE;

    /* auto-advance the playlist when a song finishes on its own */
    BUZZER_register_done_cb(s_theme.buzzer, THEME_music_done_cb, NULL);

    /* select the initial theme immediately, then keep sampling periodically */
    uint8_t id = THEME_read_id();
    s_theme.active = (s_theme.themes[id] != NULL) ? s_theme.themes[id] : &s_theme_null;
    RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Theme ID %u - active theme: %s", (unsigned int)id, s_theme.active->name);

    s_theme.anim_slot = MS_SCHEDULER_allocate_slot();
    s_theme.music_slot = MS_SCHEDULER_allocate_slot();

    s_theme.detect_slot = MS_SCHEDULER_allocate_slot();
    MS_SCHEDULER_schedule(s_theme.detect_slot, THEME_detect_task, NULL, THEME_DETECT_INTERVAL_MS, true);

    /* greet the initially selected theme with the intro animation and jingle */
    THEME_intro_start(s_theme.active);

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
    /* ignore button-driven lighting while the intro animation owns the strip */
    if (!s_theme.animating)
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

            LEDSTRIP_set_led(s_theme.lit_led_count, s_theme.active->led_color, s_theme.active->led_pattern, s_theme.active->led_phase);
            s_theme.lit_led_count++;
        }
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
void THEME_music_short_press(void)
{
    if (s_theme.active->num_songs != 0)
    {
        if ((s_theme.music_state == THEME_MUSIC_IDLE) || (s_theme.music_state == THEME_MUSIC_INTRO))
        {
            /* nothing from the playlist playing yet (idle, or still on the intro
             * jingle) - start from the first song, replacing the intro */
            THEME_music_play_index(0);
        }
        else
        {
            /* already playing, or waiting in the inter-song gap - skip to next */
            THEME_music_advance();
        }
    }
}

//_____________________________________________________________________________
void THEME_music_long_press(void)
{
    if (s_theme.music_state != THEME_MUSIC_IDLE)
    {
        RTT_LOG_log(RTT_INFO, THEME_LOG_SOURCE, "Music long press - stop and reset");
        THEME_music_reset();
    }
}

//_____________________________________________________________________________
void THEME_process(void)
{
    LEDSTRIP_process();
}
