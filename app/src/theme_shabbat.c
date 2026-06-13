/**************************************************************************//**
 * @file      theme_shabbat.c
 * @brief     Shabbat theme implementation
 *
 * @author    ymaran
 * @date      11.6.2026
 *****************************************************************************/
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "theme_shabbat.h"
#include "buzzer_notes.h"

/******************************************************************************
 * Static variables
 *****************************************************************************/
/* Shalom Aleichem - traditional Friday night melody, G major */
static const BUZZER_NOTE_st s_shalom_aleichem_notes[] =
{
    { NOTE_G4, 300 }, { NOTE_A4, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 300 }, { NOTE_G4, 300 }, { NOTE_E4, 300 },
    { NOTE_G4, 300 }, { NOTE_A4, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 300 }, { NOTE_G4, 600 }, { BUZZER_NOTE_REST, 300 },

    { NOTE_G4, 300 }, { NOTE_A4, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 300 }, { NOTE_G4, 300 }, { NOTE_E4, 300 },
    { NOTE_G4, 300 }, { NOTE_A4, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 300 }, { NOTE_G4, 600 }, { BUZZER_NOTE_REST, 300 },

    { NOTE_C5, 300 }, { NOTE_C5, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 300 }, { NOTE_B4, 300 }, { NOTE_C5, 300 },
    { NOTE_D5, 600 }, { NOTE_C5, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 600 }, { BUZZER_NOTE_REST, 300 },

    { NOTE_G4, 300 }, { NOTE_A4, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 300 }, { NOTE_G4, 300 }, { NOTE_E4, 300 },
    { NOTE_G4, 300 }, { NOTE_A4, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 300 }, { NOTE_G4, 1200 },
};
static const BUZZER_SONG_st s_shalom_aleichem =
{
    .name = "Shalom Aleichem",
    .notes = s_shalom_aleichem_notes,
    .num_notes = sizeof(s_shalom_aleichem_notes) / sizeof(s_shalom_aleichem_notes[0])
};

/* Hinei Ma Tov - A minor, repeated round-style with a faster second pass */
static const BUZZER_NOTE_st s_hinei_ma_tov_notes[] =
{
    { NOTE_A4, 300 }, { NOTE_A4, 300 }, { NOTE_B4, 300 }, { NOTE_C5, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 300 },
    { NOTE_G4, 300 }, { NOTE_A4, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 600 }, { BUZZER_NOTE_REST, 300 },

    { NOTE_A4, 300 }, { NOTE_A4, 300 }, { NOTE_B4, 300 }, { NOTE_C5, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 300 },
    { NOTE_G4, 300 }, { NOTE_F4, 300 }, { NOTE_E4, 1200 }, { BUZZER_NOTE_REST, 300 },

    { NOTE_A4, 200 }, { NOTE_A4, 200 }, { NOTE_B4, 200 }, { NOTE_C5, 200 }, { NOTE_B4, 200 }, { NOTE_A4, 200 },
    { NOTE_G4, 200 }, { NOTE_A4, 200 }, { NOTE_B4, 200 }, { NOTE_A4, 400 }, { BUZZER_NOTE_REST, 200 },

    { NOTE_A4, 200 }, { NOTE_A4, 200 }, { NOTE_B4, 200 }, { NOTE_C5, 200 }, { NOTE_B4, 200 }, { NOTE_A4, 200 },
    { NOTE_G4, 200 }, { NOTE_F4, 200 }, { NOTE_E4, 800 },
};
static const BUZZER_SONG_st s_hinei_ma_tov =
{
    .name = "Hinei Ma Tov",
    .notes = s_hinei_ma_tov_notes,
    .num_notes = sizeof(s_hinei_ma_tov_notes) / sizeof(s_hinei_ma_tov_notes[0])
};

/* ordered playlist for the Shabbat theme */
static const BUZZER_SONG_st * const s_shabbat_songs[] =
{
    &s_shalom_aleichem,
    &s_hinei_ma_tov,
};

static const THEME_st s_theme_shabbat =
{
    .id = THEME_SHABBAT_ID,
    .name = "Shabbat",
    .num_leds = 2,
    .led_color = LEDSTRIP_COLOR_YELLOW,
    .led_pattern = LEDSTRIP_BLINK_GAUSSIAN_1HZ,
    .led_phase = LEDSTRIP_PHASE_RANDOM,
    .songs = s_shabbat_songs,
    .num_songs = sizeof(s_shabbat_songs) / sizeof(s_shabbat_songs[0]),
};

/******************************************************************************
 * Public functions
 *****************************************************************************/
//_____________________________________________________________________________
void THEME_SHABBAT_register(void)
{
    THEME_register(&s_theme_shabbat);
}
