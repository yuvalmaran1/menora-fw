/**************************************************************************//**
 * @file      captouch_btn.c
 * @brief     Capacitive touch-sense push button implementation
 *
 * @author    ymaran
 * @date      7.6.2026
 *****************************************************************************/
/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stddef.h>
#include "captouch_btn.h"
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
/* round-robin list of all registered button instances, sharing one TSC peripheral */
static CAPTOUCH_BTN_st* s_btn_list_head = NULL;
static CAPTOUCH_BTN_st* s_btn_list_tail = NULL;
static CAPTOUCH_BTN_st* s_next_to_acquire = NULL;

/* button instance currently being sampled by the TSC (NULL if idle) */
static CAPTOUCH_BTN_st* s_acquiring_btn = NULL;

/******************************************************************************
 * Static functions
 *****************************************************************************/
//_____________________________________________________________________________
static void CAPTOUCH_BTN_start_acquisition(CAPTOUCH_BTN_st* p_btn)
{
    TSC_IOConfigTypeDef io_cfg = {
        .ChannelIOs = p_btn->channel_io,
        .ShieldIOs = 0,
        .SamplingIOs = p_btn->sampling_io,
    };

    HAL_TSC_IOConfig(p_btn->tsc, &io_cfg);

    s_acquiring_btn = p_btn;

    if (HAL_TSC_Start_IT(p_btn->tsc) != HAL_OK)
    {
        s_acquiring_btn = NULL;
    }
}

//_____________________________________________________________________________
/* kick off the next pending acquisition (round-robin), if the TSC is idle.
 * safe to call from both task and ISR context - acquisitions are naturally
 * serialized via @ref s_acquiring_btn, since only one can run at a time. */
static void CAPTOUCH_BTN_kick_next_acquisition(void)
{
    CAPTOUCH_BTN_st* p_btn = s_next_to_acquire;

    if ((s_acquiring_btn != NULL) || (p_btn == NULL))
    {
        return;
    }

    s_next_to_acquire = (p_btn->next != NULL) ? p_btn->next : s_btn_list_head;

    CAPTOUCH_BTN_start_acquisition(p_btn);
}

//_____________________________________________________________________________
static void CAPTOUCH_BTN_handle_press_state(CAPTOUCH_BTN_st* p_btn, bool touched)
{
    uint32_t now = HAL_GetTick();

    switch (p_btn->state)
    {
        case CAPTOUCH_BTN_STATE_IDLE:
            if (touched)
            {
                p_btn->state = CAPTOUCH_BTN_STATE_PRESSED;
                p_btn->press_start_tick = now;
                p_btn->long_press_fired = false;
            }
            break;

        case CAPTOUCH_BTN_STATE_PRESSED:
            if (touched)
            {
                if (!p_btn->long_press_fired && ((now - p_btn->press_start_tick) >= CAPTOUCH_BTN_LONG_PRESS_MIN_MS))
                {
                    p_btn->long_press_fired = true;

                    if (p_btn->on_long_press != NULL)
                    {
                        p_btn->on_long_press(p_btn->long_press_ctx);
                    }
                }
            }
            else
            {
                uint32_t hold_duration = now - p_btn->press_start_tick;

                if (!p_btn->long_press_fired && (hold_duration <= CAPTOUCH_BTN_SHORT_PRESS_MAX_MS))
                {
                    if (p_btn->on_short_press != NULL)
                    {
                        p_btn->on_short_press(p_btn->short_press_ctx);
                    }
                }

                p_btn->state = CAPTOUCH_BTN_STATE_IDLE;
            }
            break;

        default:
            break;
    }
}

//_____________________________________________________________________________
/******************************************************************************
 * Public functions
 *****************************************************************************/
//_____________________________________________________________________________
CAPTOUCH_BTN_STATUS_t CAPTOUCH_BTN_init(CAPTOUCH_BTN_st* p_btn, CAPTOUCH_BTN_INIT_CONFIG_st* p_init_config)
{
    if ((p_btn == NULL) || (p_init_config == NULL) || (p_init_config->tsc == NULL))
    {
        return CAPTOUCH_BTN_STATUS_INVALID_ARG;
    }

    p_btn->tsc = p_init_config->tsc;
    p_btn->group_index = p_init_config->group_index;
    p_btn->channel_io = p_init_config->channel_io;
    p_btn->sampling_io = p_init_config->sampling_io;
    p_btn->touch_threshold = p_init_config->touch_threshold;

    p_btn->state = CAPTOUCH_BTN_STATE_IDLE;
    p_btn->press_start_tick = 0;
    p_btn->long_press_fired = false;

    p_btn->on_short_press = NULL;
    p_btn->short_press_ctx = NULL;
    p_btn->on_long_press = NULL;
    p_btn->long_press_ctx = NULL;

    p_btn->acq_done = false;
    p_btn->acq_touched = false;
    p_btn->next = NULL;

    /* append to the round-robin acquisition list */
    if (s_btn_list_tail == NULL)
    {
        s_btn_list_head = p_btn;
    }
    else
    {
        s_btn_list_tail->next = p_btn;
    }

    s_btn_list_tail = p_btn;

    if (s_next_to_acquire == NULL)
    {
        s_next_to_acquire = p_btn;
    }

    return CAPTOUCH_BTN_STATUS_OK;
}

//_____________________________________________________________________________
CAPTOUCH_BTN_STATUS_t CAPTOUCH_BTN_register_short_press_cb(CAPTOUCH_BTN_st* p_btn, CAPTOUCH_BTN_callback_t cb, void* ctx)
{
    if (p_btn == NULL)
    {
        return CAPTOUCH_BTN_STATUS_INVALID_ARG;
    }

    p_btn->on_short_press = cb;
    p_btn->short_press_ctx = ctx;

    return CAPTOUCH_BTN_STATUS_OK;
}

//_____________________________________________________________________________
CAPTOUCH_BTN_STATUS_t CAPTOUCH_BTN_register_long_press_cb(CAPTOUCH_BTN_st* p_btn, CAPTOUCH_BTN_callback_t cb, void* ctx)
{
    if (p_btn == NULL)
    {
        return CAPTOUCH_BTN_STATUS_INVALID_ARG;
    }

    p_btn->on_long_press = cb;
    p_btn->long_press_ctx = ctx;

    return CAPTOUCH_BTN_STATUS_OK;
}

//_____________________________________________________________________________
void CAPTOUCH_BTN_process(CAPTOUCH_BTN_st* p_btn)
{
    ASSERT(p_btn != NULL, "null button instance");

    /* keep the background acquisition chain alive (no-op while one is in flight) */
    CAPTOUCH_BTN_kick_next_acquisition();

    if (p_btn->acq_done)
    {
        p_btn->acq_done = false;

        CAPTOUCH_BTN_handle_press_state(p_btn, p_btn->acq_touched);
    }
}

//_____________________________________________________________________________
/******************************************************************************
 * HAL TSC callbacks (ISR context)
 *****************************************************************************/
//_____________________________________________________________________________
void HAL_TSC_ConvCpltCallback(TSC_HandleTypeDef* htsc)
{
    CAPTOUCH_BTN_st* p_btn = s_acquiring_btn;

    if (p_btn != NULL)
    {
        s_acquiring_btn = NULL;

        p_btn->acq_touched = (HAL_TSC_GroupGetStatus(htsc, p_btn->group_index) == TSC_GROUP_COMPLETED)
                          && (HAL_TSC_GroupGetValue(htsc, p_btn->group_index) > p_btn->touch_threshold);
        p_btn->acq_done = true;
    }

    CAPTOUCH_BTN_kick_next_acquisition();
}

//_____________________________________________________________________________
void HAL_TSC_ErrorCallback(TSC_HandleTypeDef* htsc)
{
    (void)htsc;

    CAPTOUCH_BTN_st* p_btn = s_acquiring_btn;

    if (p_btn != NULL)
    {
        s_acquiring_btn = NULL;

        p_btn->acq_touched = false;
        p_btn->acq_done = true;
    }

    CAPTOUCH_BTN_kick_next_acquisition();
}
