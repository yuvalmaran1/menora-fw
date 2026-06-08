/**************************************************************************//**
 * @file      buzzer.h
 * @brief     PWM buzzer music playback interface
 *
 * @author    ymaran
 * @date      8.6.2026
 *****************************************************************************/
#ifndef BUZZER_H_
#define BUZZER_H_

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx_hal.h"
#include "ms_scheduler.h"

/******************************************************************************
 * Constants
 *****************************************************************************/
/*!< default definitions */
#if !defined(BUZZER_TIM_CLK_HZ)
    #define BUZZER_TIM_CLK_HZ       (32000000UL) // timer input clock (APB1 timer clock); must match the system clock configuration in main.c
#endif

#if !defined(BUZZER_COUNTER_CLK_HZ)
    #define BUZZER_COUNTER_CLK_HZ   (2000000UL)  // counter clock after prescaling, chosen for fine frequency resolution across the musical note range (~30Hz..5kHz) while keeping the auto-reload value within the timer's 16-bit range
#endif

/*!< prescaler value yielding @ref BUZZER_COUNTER_CLK_HZ from @ref BUZZER_TIM_CLK_HZ */
#define BUZZER_TIM_PRESCALER        ((BUZZER_TIM_CLK_HZ / BUZZER_COUNTER_CLK_HZ) - 1)

/*!< note frequency denoting a rest (silence) */
#define BUZZER_NOTE_REST            (0)

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Status
 *****************************************************************************/
/*! @typedef BUZZER_STATUS_t
 * return status. 0 success. <0 error.
 */
typedef int32_t BUZZER_STATUS_t;

#define BUZZER_STATUS_OK                            ((BUZZER_STATUS_t)( 0)) //!< no error
#define BUZZER_STATUS_INVALID_ARG                   ((BUZZER_STATUS_t)(-1)) //!< invalid argument

/******************************************************************************
 * Data types
 *****************************************************************************/
/* song playback state machine */
typedef enum
{
    BUZZER_STATE_IDLE,      // no song is playing, output is silent
    BUZZER_STATE_PLAYING,   // a song is currently playing
} BUZZER_STATE_en;

/*!< a single note: a tone held for a duration, or a rest (silence) when
 * frequency_hz equals @ref BUZZER_NOTE_REST. See buzzer_notes.h for a table
 * of musical note frequencies to use here. */
typedef struct
{
    uint32_t frequency_hz;  // tone frequency in Hz, or BUZZER_NOTE_REST for silence
    uint32_t duration_ms;   // how long to hold the note (or rest), in milliseconds
} BUZZER_NOTE_st;

/*!< a song: an ordered sequence of notes
 *
 * @note the referenced note array is not copied - it must remain valid for
 * the entire duration of playback (e.g. a static const array) */
typedef struct
{
    const BUZZER_NOTE_st* notes;
    uint32_t num_notes;
} BUZZER_SONG_st;

/*!< song playback completion event callback, invoked once playback reaches
 * the end of the song
 * @param[in] ctx user context, as registered along with the callback
 */
typedef void (*BUZZER_callback_t)(void* ctx);

/*!< initial configuration struct */
typedef struct
{
    TIM_HandleTypeDef* tim;     // timer peripheral driving the buzzer output in PWM mode
    uint32_t tim_ch;            // PWM channel connected to the buzzer (e.g. TIM_CHANNEL_4)
} BUZZER_INIT_CONFIG_st;

/*!< instance context */
typedef struct
{
    TIM_HandleTypeDef* tim;
    uint32_t tim_ch;

    BUZZER_STATE_en state;
    const BUZZER_SONG_st* song;
    uint32_t note_index;
    bool sounding;          // true while the PWM output is actively driving a tone (false during a rest or when idle)

    MS_SCHEDULER_SLOT_t* slot;  // scheduler slot used to wake up exactly when the current note ends

    BUZZER_callback_t on_done;
    void* done_ctx;
} BUZZER_st;

/******************************************************************************
 * Public function prototypes
 *****************************************************************************/
/***************************************************************************//**
 * Initialize device
 *
 * @param[in] p_buzzer buzzer instance
 * @param[in] p_init_config initial configuration
 *
 * @return procedure result
 ******************************************************************************/
BUZZER_STATUS_t BUZZER_init(BUZZER_st* p_buzzer, BUZZER_INIT_CONFIG_st* p_init_config);

/***************************************************************************//**
 * Register a callback for the song playback completion event, invoked once
 * playback reaches the end of the song
 *
 * @param[in] p_buzzer buzzer instance
 * @param[in] cb callback function, or NULL to unregister
 * @param[in] ctx user context, passed back to the callback as-is
 *
 * @return procedure result
 ******************************************************************************/
BUZZER_STATUS_t BUZZER_register_done_cb(BUZZER_st* p_buzzer, BUZZER_callback_t cb, void* ctx);

/***************************************************************************//**
 * Start playing a song asynchronously. Returns immediately - playback is
 * advanced by @ref BUZZER_process. Replaces any song currently playing.
 *
 * @param[in] p_buzzer buzzer instance
 * @param[in] p_song song to play
 *
 * @return procedure result
 ******************************************************************************/
BUZZER_STATUS_t BUZZER_play(BUZZER_st* p_buzzer, const BUZZER_SONG_st* p_song);

/***************************************************************************//**
 * Stop playback immediately and silence the output. No-op if idle. Does not
 * invoke the completion callback registered via @ref BUZZER_register_done_cb.
 *
 * @param[in] p_buzzer buzzer instance
 *
 * @return none
 ******************************************************************************/
void BUZZER_stop(BUZZER_st* p_buzzer);

/***************************************************************************//**
 * Get the current playback state
 *
 * @param[in] p_buzzer buzzer instance
 *
 * @return current state
 ******************************************************************************/
BUZZER_STATE_en BUZZER_get_state(BUZZER_st* p_buzzer);

#endif // BUZZER_H_
