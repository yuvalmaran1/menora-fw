/**************************************************************************//**
 * @file      theme_hanuka.c
 * @brief     Hanuka theme implementation
 *
 * @author    ymaran
 * @date      11.6.2026
 *****************************************************************************/
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "theme_hanuka.h"
#include "buzzer_notes.h"

/******************************************************************************
 * Static variables
 *****************************************************************************/
/* Hanuka, Hanuka */
static const BUZZER_NOTE_st s_hanuka_hanuka_notes[] =
{
    { NOTE_G4, 300 }, { NOTE_E4, 300 }, { NOTE_G4, 300 }, { BUZZER_NOTE_REST, 300 },
    { NOTE_G4, 300 }, { NOTE_E4, 300 }, { NOTE_G4, 300 }, { BUZZER_NOTE_REST, 300 },
    { NOTE_E4, 300 }, { NOTE_G4, 300 }, { NOTE_C5, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 600 }, { BUZZER_NOTE_REST, 400 },
    { NOTE_F4, 300 }, { NOTE_D4, 300 }, { NOTE_F4, 300 }, { BUZZER_NOTE_REST, 300 },
    { NOTE_F4, 300 }, { NOTE_D4, 300 }, { NOTE_F4, 300 }, { BUZZER_NOTE_REST, 300 },
    { NOTE_D4, 300 }, { NOTE_F4, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 300 }, { NOTE_G4, 600 }, { BUZZER_NOTE_REST, 400 },
    { NOTE_G4, 300 }, { NOTE_E4, 300 }, { NOTE_G4, 300 }, { BUZZER_NOTE_REST, 300 },
    { NOTE_G4, 300 }, { NOTE_E4, 300 }, { NOTE_G4, 300 }, { BUZZER_NOTE_REST, 300 },
    { NOTE_E4, 300 }, { NOTE_G4, 300 }, { NOTE_C5, 300 }, { NOTE_B4, 300 }, { NOTE_A4, 600 }, { BUZZER_NOTE_REST, 400 },
    { NOTE_B4, 300 }, { NOTE_B4, 300 }, { NOTE_B4, 300 }, { BUZZER_NOTE_REST, 300 },
    { NOTE_B4, 300 }, { NOTE_B4, 300 }, { NOTE_B4, 300 }, { BUZZER_NOTE_REST, 300 },
    { NOTE_B4, 300 }, { NOTE_G4, 300 }, { NOTE_A4, 300 }, { NOTE_B4, 300 }, { NOTE_C5, 400 }, { BUZZER_NOTE_REST, 200 },
    { NOTE_C5, 400 }
};
static const BUZZER_SONG_st s_hanuka_hanuka_song =
{
    .name = "Hanuka, Hanuka",
    .notes = s_hanuka_hanuka_notes,
    .num_notes = sizeof(s_hanuka_hanuka_notes) / sizeof(s_hanuka_hanuka_notes[0])
};

/* Maoz Tzur */
static const BUZZER_NOTE_st s_maoz_tzur_notes[] =
{
    { NOTE_C5, 400 }, { NOTE_G4, 400 }, { NOTE_C5, 400 }, { NOTE_F5, 400 }, 
    { NOTE_E5, 400 }, { NOTE_D5, 400 }, { NOTE_C5, 400 }, { BUZZER_NOTE_REST, 200 },
    
    { NOTE_G5, 200 }, { NOTE_G5, 400 }, { NOTE_A5, 200 }, { NOTE_D5, 400 }, 
    { NOTE_E5, 200 }, { NOTE_F5, 200 }, { NOTE_E5, 400 }, { NOTE_D5, 400 }, 
    { NOTE_C5, 600 },{ BUZZER_NOTE_REST, 200 },

    { NOTE_C5, 400 }, { NOTE_G4, 400 }, { NOTE_C5, 400 }, { NOTE_F5, 400 }, 
    { NOTE_E5, 400 }, { NOTE_D5, 400 }, { NOTE_C5, 400 }, { BUZZER_NOTE_REST, 200 },
    
    { NOTE_G5, 200 }, { NOTE_G5, 400 }, { NOTE_A5, 200 }, { NOTE_D5, 400 }, 
    { NOTE_E5, 200 }, { NOTE_F5, 200 }, { NOTE_E5, 400 }, { NOTE_D5, 400 }, 
    { NOTE_C5, 600 },{ BUZZER_NOTE_REST, 200 },

    { NOTE_G5, 200 }, { NOTE_G4, 400 }, { NOTE_G5, 200 }, { NOTE_A5, 400 }, 
    { NOTE_B5, 400 }, { NOTE_C6, 800 }, { NOTE_G5, 800 }, { BUZZER_NOTE_REST, 200 },
};
static const BUZZER_SONG_st s_maoz_tzur =
{
    .name     = "Maoz Tzur",
    .notes    = s_maoz_tzur_notes,
    .num_notes = sizeof(s_maoz_tzur_notes) / sizeof(s_maoz_tzur_notes[0])
};

/* Hanukia Li Yesh */
static const BUZZER_NOTE_st s_hanukia_li_yesh_notes[] =
{
    { NOTE_C4, 200 }, { NOTE_F4, 200 }, { NOTE_F4, 200 }, { NOTE_G4, 200 }, { NOTE_A4, 200 }, { NOTE_F4, 200 }, { BUZZER_NOTE_REST, 400 },
    { NOTE_C4, 200 }, { NOTE_F4, 200 }, { NOTE_F4, 200 }, { NOTE_G4, 200 }, { NOTE_A4, 200 }, { NOTE_F4, 200 }, { BUZZER_NOTE_REST, 400 },

    { NOTE_F4, 200 }, { NOTE_AS4, 200 }, { NOTE_A4, 200 }, { NOTE_AS4, 200 }, { NOTE_C5, 200 }, { NOTE_A4, 200 }, { BUZZER_NOTE_REST, 400 },
    { NOTE_A4, 200 }, { NOTE_G4, 200 }, { NOTE_F4, 200 }, { NOTE_G4, 200 }, { NOTE_A4, 200 }, { NOTE_F4, 200 }, { BUZZER_NOTE_REST, 400 },

    { NOTE_F4, 200 }, { NOTE_AS4, 200 }, { NOTE_A4, 200 }, { NOTE_AS4, 200 }, { NOTE_C5, 200 }, { NOTE_A4, 200 }, { BUZZER_NOTE_REST, 400 },
    { NOTE_A4, 200 }, { NOTE_G4, 200 }, { NOTE_F4, 200 }, { NOTE_G4, 200 }, { NOTE_A4, 200 }, { NOTE_F4, 200 }, { BUZZER_NOTE_REST, 400 },

    { NOTE_F4, 200 }, { NOTE_F4, 200 }, { NOTE_E4, 100 }, { NOTE_D4, 100 }, { NOTE_C4, 200 }, { NOTE_F4, 200 }, { NOTE_F4, 200 },{ BUZZER_NOTE_REST, 400 },
    { NOTE_F4, 200 }, { NOTE_F4, 200 }, { NOTE_E4, 100 }, { NOTE_D4, 100 }, { NOTE_C4, 200 }, { NOTE_A4, 200 }, { NOTE_F4, 200 },{ BUZZER_NOTE_REST, 400 },

    { NOTE_F4, 200 }, { NOTE_F4, 200 }, { NOTE_E4, 100 }, { NOTE_D4, 100 }, { NOTE_C4, 200 }, { NOTE_F4, 200 }, { NOTE_F4, 200 },{ BUZZER_NOTE_REST, 400 },
    { NOTE_F4, 200 }, { NOTE_F4, 200 }, { NOTE_E4, 100 }, { NOTE_D4, 100 }, { NOTE_C4, 200 }, { NOTE_A4, 200 }, { NOTE_F4, 200 },{ BUZZER_NOTE_REST, 400 },
};
static const BUZZER_SONG_st s_hanukia_li_yesh =
{
    .name     = "Hanukia Li Yesh",
    .notes    = s_hanukia_li_yesh_notes,
    .num_notes = sizeof(s_hanukia_li_yesh_notes) / sizeof(s_hanukia_li_yesh_notes[0])
};

/* Kad Katan */
static const BUZZER_NOTE_st s_kad_katan_notes[] =
{
    { NOTE_E4, 600 }, { NOTE_D4, 200 }, { NOTE_E4, 400 }, { BUZZER_NOTE_REST, 400 },
    { NOTE_E4, 600 }, { NOTE_D4, 200 }, { NOTE_E4, 400 }, { BUZZER_NOTE_REST, 200 }, { NOTE_E4, 200 },

    { NOTE_E4, 400 },{ NOTE_A4, 400 }, { NOTE_E4, 400 }, { NOTE_FS4, 400 }, 
    { NOTE_G4, 600 }, { NOTE_FS4, 200 }, { NOTE_E4, 400 }, { BUZZER_NOTE_REST, 400 },

    { NOTE_E4, 600 }, { NOTE_D4, 200 }, { NOTE_E4, 400 }, { BUZZER_NOTE_REST, 400 },
    { NOTE_E4, 600 }, { NOTE_D4, 200 }, { NOTE_E4, 400 }, { BUZZER_NOTE_REST, 400 },

    { NOTE_G4, 400 }, { NOTE_E4, 400 },{ NOTE_D4, 400 }, { NOTE_E4, 200 }, { NOTE_FS4, 200 },
    { NOTE_G4, 400 }, { NOTE_FS4, 400 }, { NOTE_E4, 400 }, { BUZZER_NOTE_REST, 400 },

    { NOTE_E4, 400 }, { NOTE_B4, 400 }, { NOTE_A4, 400 }, { NOTE_A4, 400 },
    { NOTE_E4, 400 }, { NOTE_D4, 400 }, { NOTE_G4, 800 }, 

    { NOTE_E4, 400 }, { NOTE_B4, 400 }, { NOTE_A4, 400 }, { NOTE_A4, 400 },
    { NOTE_G4, 400 }, { NOTE_FS4, 400 }, { NOTE_E4, 800 }, 

    { NOTE_A4, 400 }, { NOTE_D4, 400 }, { NOTE_D4, 400 }, { NOTE_D4, 400 },
    { NOTE_CS4, 400 }, { NOTE_D4, 400 }, { NOTE_E4, 800 }, 

    { NOTE_G4, 400 }, { NOTE_E4, 400 }, { NOTE_D4, 400 }, { NOTE_E4, 200 }, { NOTE_FS4, 200 }, 
    { NOTE_G4, 200 }, { BUZZER_NOTE_REST, 200 },{ NOTE_FS4, 200 }, { BUZZER_NOTE_REST, 200 }, { NOTE_E4, 800 },
};
static const BUZZER_SONG_st s_kad_katan =
{
    .name     = "Kad Katan",
    .notes    = s_kad_katan_notes,
    .num_notes = sizeof(s_kad_katan_notes) / sizeof(s_kad_katan_notes[0])
};

/* ordered playlist for the Hanuka theme */
static const BUZZER_SONG_st * const s_hanuka_songs[] =
{
    &s_hanuka_hanuka_song,
    &s_maoz_tzur,
    &s_hanukia_li_yesh,
    &s_kad_katan,
};

static const THEME_st s_theme_hanuka =
{
    .id = THEME_HANUKA_ID,
    .name = "Hanuka",
    .num_leds = 9,
    .led_color = LEDSTRIP_COLOR_CANDLE,
    .led_pattern = LEDSTRIP_BLINK_CANDLE_FLICKER,
    .led_phase = LEDSTRIP_PHASE_RANDOM,
    .songs = s_hanuka_songs,
    .num_songs = sizeof(s_hanuka_songs) / sizeof(s_hanuka_songs[0]),
};

/******************************************************************************
 * Public functions
 *****************************************************************************/
//_____________________________________________________________________________
void THEME_HANUKA_register(void)
{
    THEME_register(&s_theme_hanuka);
}
