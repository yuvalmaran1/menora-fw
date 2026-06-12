/**************************************************************************//**
 * @file      theme.h
 * @brief     Application theme management
 *
 * @author    ymaran
 * @date      11.6.2026
 *****************************************************************************/
#ifndef THEME_H_
#define THEME_H_

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx_hal.h"
#include "ledstrip.h"
#include "buzzer.h"

/******************************************************************************
 * Constants
 *****************************************************************************/
/*!< theme ID denoting "no theme active" - always registered, can't be overridden */
#define THEME_NULL_ID (0)

/*!< number of selectable theme IDs (3-bit ID read from ID2:ID1:ID0) */
#define THEME_NUM_IDS (8)

/*!< default definitions */
#if !defined(THEME_DETECT_INTERVAL_MS)
    #define THEME_DETECT_INTERVAL_MS (100) // how often the ID GPIOs are sampled for theme changes
#endif

/******************************************************************************
 * Status
 *****************************************************************************/
/*! @typedef THEME_STATUS_t
 * return status. 0 success. <0 error.
 */
typedef int32_t THEME_STATUS_t;

#define THEME_STATUS_OK                            ((THEME_STATUS_t)( 0)) //!< no error
#define THEME_STATUS_INVALID_ARG                   ((THEME_STATUS_t)(-1)) //!< invalid argument

/******************************************************************************
 * Data types
 *****************************************************************************/
/*!< a selectable application theme
 *
 * @note the referenced songs array is not copied - it must remain valid for
 * the lifetime of the application (e.g. a static const array) */
typedef struct
{
    uint8_t id;                            // theme ID in range [1, THEME_NUM_IDS-1], matches ID2:ID1:ID0
    const char* name;                      // human readable theme name, used for logging
    uint32_t num_leds;                     // number of ledstrip LEDs used by this theme
    LEDSTRIP_COLOR_en led_color;           // color used for lit LEDs
    LEDSTRIP_BLINK_en led_pattern;         // blink pattern used for lit LEDs
    const BUZZER_SONG_st * const * songs;  // playlist
    uint32_t num_songs;                    // number of entries in songs[]
} THEME_st;

/*!< initial configuration struct */
typedef struct
{
    /* buzzer instance used to play theme songs - owned and initialized by the
     * application, must remain valid for the lifetime of the theme module */
    BUZZER_st* buzzer;

    /* theme ID GPIOs */
    GPIO_TypeDef* id0_port;
    uint16_t id0_pin;
    GPIO_TypeDef* id1_port;
    uint16_t id1_pin;
    GPIO_TypeDef* id2_port;
    uint16_t id2_pin;
} THEME_INIT_CONFIG_st;

/******************************************************************************
 * Public function prototypes
 *****************************************************************************/
/***************************************************************************//**
 * Initialize the theme module - brings up the LED strip and buzzer, and starts
 * periodically sampling the ID2:ID1:ID0 GPIOs to detect the active theme
 *
 * @note all themes must be registered via @ref THEME_register before calling
 * this function, so that the initial detection can select among them
 *
 * @param[in] p_init_config initial configuration
 *
 * @return procedure result
 ******************************************************************************/
THEME_STATUS_t THEME_init(THEME_INIT_CONFIG_st* p_init_config);

/***************************************************************************//**
 * Register a theme implementation, making it selectable via its ID
 *
 * @param[in] p_theme theme definition - must remain valid for the lifetime of
 *                     the application (e.g. a static const struct)
 *
 * @return procedure result
 ******************************************************************************/
THEME_STATUS_t THEME_register(const THEME_st* p_theme);

/***************************************************************************//**
 * Get the currently active theme
 *
 * @return active theme - never NULL, returns the built-in "no theme" instance
 *         (id @ref THEME_NULL_ID) when ID2:ID1:ID0 selects an unregistered ID
 ******************************************************************************/
const THEME_st* THEME_get_active(void);

/***************************************************************************//**
 * Light up one more LED (active theme's color and pattern); once all of the
 * theme's LEDs are lit, the next call turns them all off
 *
 * @return none
 ******************************************************************************/
void THEME_light_next(void);

/***************************************************************************//**
 * Turn all LEDs off, regardless of current state
 *
 * @return none
 ******************************************************************************/
void THEME_light_reset(void);

/***************************************************************************//**
 * Start playing the next song in the active theme's playlist, replacing any
 * song currently playing
 *
 * @return none
 ******************************************************************************/
void THEME_music_play_next(void);

/***************************************************************************//**
 * Stop playback immediately and silence the output. No-op if idle.
 *
 * @return none
 ******************************************************************************/
void THEME_music_stop(void);

/***************************************************************************//**
 * Check whether a song is currently playing
 *
 * @return true if a song is currently playing, false otherwise
 ******************************************************************************/
bool THEME_music_is_playing(void);

/***************************************************************************//**
 * Process theme module internals - drives the LED strip refresh
 *
 * @note This function should be called periodically (e.g. from the main loop
 *       or a scheduled task) and never blocks
 *
 * @return none
 ******************************************************************************/
void THEME_process(void);

#endif // THEME_H_
