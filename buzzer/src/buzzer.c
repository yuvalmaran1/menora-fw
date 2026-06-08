/**************************************************************************//**
 * @file      buzzer.c
 * @brief     PWM buzzer music playback implementation
 *
 * @author    ymaran
 * @date      8.6.2026
 *****************************************************************************/
/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stddef.h>
#include "buzzer.h"
#include "assert.h"

/******************************************************************************
 * Constants
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Data types
 *****************************************************************************/

/******************************************************************************
 * Static variables
 *****************************************************************************/

/******************************************************************************
 * Static functions
 *****************************************************************************/
//_____________________________________________________________________________
/* mute the output, if currently sounding */
static void BUZZER_silence(BUZZER_st* p_buzzer)
{
    if (p_buzzer->sounding)
    {
        HAL_TIM_PWM_Stop(p_buzzer->tim, p_buzzer->tim_ch);
        p_buzzer->sounding = false;
    }
}

//_____________________________________________________________________________
/* drive the output to reproduce the given note (or silence it, for a rest) */
static void BUZZER_play_note(BUZZER_st* p_buzzer, const BUZZER_NOTE_st* p_note)
{
    if (p_note->frequency_hz == BUZZER_NOTE_REST)
    {
        BUZZER_silence(p_buzzer);
        return;
    }

    uint32_t arr = (BUZZER_COUNTER_CLK_HZ / p_note->frequency_hz) - 1;
    uint32_t ccr = (arr + 1) / 2; // 50% duty cycle - symmetric square wave for max loudness

    __HAL_TIM_SET_AUTORELOAD(p_buzzer->tim, arr);
    __HAL_TIM_SET_COMPARE(p_buzzer->tim, p_buzzer->tim_ch, ccr);

    if (p_buzzer->sounding)
    {
        /* restart the counter so the new period takes effect immediately,
         * rather than only once the previous (longer) period elapses */
        __HAL_TIM_SET_COUNTER(p_buzzer->tim, 0);
    }
    else
    {
        HAL_TIM_PWM_Start(p_buzzer->tim, p_buzzer->tim_ch);
        p_buzzer->sounding = true;
    }
}

//_____________________________________________________________________________
/* abort the pending note-end wakeup, if one is scheduled */
static void BUZZER_cancel_schedule(BUZZER_st* p_buzzer)
{
    if (MS_SCHEDULER_is_slot_active(p_buzzer->slot))
    {
        MS_SCHEDULER_abort(p_buzzer->slot);
    }
}

//_____________________________________________________________________________
static void BUZZER_note_timer_task(void* ctx);

/* drive the given note and arrange to be woken up exactly when it ends */
static void BUZZER_arm_note(BUZZER_st* p_buzzer, const BUZZER_NOTE_st* p_note)
{
    BUZZER_play_note(p_buzzer, p_note);
    MS_SCHEDULER_schedule(p_buzzer->slot, BUZZER_note_timer_task, p_buzzer, p_note->duration_ms, false);
}

//_____________________________________________________________________________
/* move on to the next note, or finish playback once the song is exhausted */
static void BUZZER_advance(BUZZER_st* p_buzzer)
{
    p_buzzer->note_index++;

    if (p_buzzer->note_index >= p_buzzer->song->num_notes)
    {
        BUZZER_callback_t on_done = p_buzzer->on_done;
        void* done_ctx = p_buzzer->done_ctx;

        BUZZER_silence(p_buzzer);
        p_buzzer->state = BUZZER_STATE_IDLE;
        p_buzzer->song = NULL;

        if (on_done != NULL)
        {
            on_done(done_ctx);
        }

        return;
    }

    BUZZER_arm_note(p_buzzer, &p_buzzer->song->notes[p_buzzer->note_index]);
}

//_____________________________________________________________________________
/* scheduler callback - fires exactly when the current note ends */
static void BUZZER_note_timer_task(void* ctx)
{
    BUZZER_advance((BUZZER_st*)ctx);
}

//_____________________________________________________________________________
/******************************************************************************
 * Public functions
 *****************************************************************************/
//_____________________________________________________________________________
BUZZER_STATUS_t BUZZER_init(BUZZER_st* p_buzzer, BUZZER_INIT_CONFIG_st* p_init_config)
{
    if ((p_buzzer == NULL) || (p_init_config == NULL) || (p_init_config->tim == NULL))
    {
        return BUZZER_STATUS_INVALID_ARG;
    }

    p_buzzer->tim = p_init_config->tim;
    p_buzzer->tim_ch = p_init_config->tim_ch;

    p_buzzer->state = BUZZER_STATE_IDLE;
    p_buzzer->song = NULL;
    p_buzzer->note_index = 0;
    p_buzzer->sounding = false;

    p_buzzer->on_done = NULL;
    p_buzzer->done_ctx = NULL;

    /* configure the prescaler so the counter clock yields fine frequency
     * resolution across the musical note range (see BUZZER_COUNTER_CLK_HZ) */
    __HAL_TIM_SET_PRESCALER(p_buzzer->tim, BUZZER_TIM_PRESCALER);

    /* slot used to wake up exactly when the current note ends - no periodic
     * polling required */
    p_buzzer->slot = MS_SCHEDULER_allocate_slot();

    return BUZZER_STATUS_OK;
}

//_____________________________________________________________________________
BUZZER_STATUS_t BUZZER_register_done_cb(BUZZER_st* p_buzzer, BUZZER_callback_t cb, void* ctx)
{
    if (p_buzzer == NULL)
    {
        return BUZZER_STATUS_INVALID_ARG;
    }

    p_buzzer->on_done = cb;
    p_buzzer->done_ctx = ctx;

    return BUZZER_STATUS_OK;
}

//_____________________________________________________________________________
BUZZER_STATUS_t BUZZER_play(BUZZER_st* p_buzzer, const BUZZER_SONG_st* p_song)
{
    if ((p_buzzer == NULL) || (p_song == NULL) || (p_song->notes == NULL) || (p_song->num_notes == 0))
    {
        return BUZZER_STATUS_INVALID_ARG;
    }

    BUZZER_cancel_schedule(p_buzzer);
    BUZZER_silence(p_buzzer);

    p_buzzer->song = p_song;
    p_buzzer->note_index = 0;
    p_buzzer->state = BUZZER_STATE_PLAYING;

    BUZZER_arm_note(p_buzzer, &p_song->notes[0]);

    return BUZZER_STATUS_OK;
}

//_____________________________________________________________________________
void BUZZER_stop(BUZZER_st* p_buzzer)
{
    ASSERT(p_buzzer != NULL, "null buzzer instance");

    BUZZER_cancel_schedule(p_buzzer);
    BUZZER_silence(p_buzzer);

    p_buzzer->state = BUZZER_STATE_IDLE;
    p_buzzer->song = NULL;
}

//_____________________________________________________________________________
BUZZER_STATE_en BUZZER_get_state(BUZZER_st* p_buzzer)
{
    ASSERT(p_buzzer != NULL, "null buzzer instance");

    return p_buzzer->state;
}
