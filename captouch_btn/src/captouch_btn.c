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

/******************************************************************************
 * Static functions
 *****************************************************************************/
//_____________________________________________________________________________
static bool CAPTOUCH_BTN_acquire_is_touched(CAPTOUCH_BTN_st* p_btn)
{
    TSC_IOConfigTypeDef io_cfg = {
        .ChannelIOs = p_btn->channel_io,
        .ShieldIOs = 0,
        .SamplingIOs = p_btn->sampling_io,
    };

    HAL_TSC_IOConfig(p_btn->tsc, &io_cfg);

    if (HAL_TSC_Start(p_btn->tsc) != HAL_OK)
    {
        return false;
    }

    if (HAL_TSC_PollForAcquisition(p_btn->tsc) != HAL_OK)
    {
        return false;
    }

    if (HAL_TSC_GroupGetStatus(p_btn->tsc, p_btn->group_index) != TSC_GROUP_COMPLETED)
    {
        return false;
    }

    return (HAL_TSC_GroupGetValue(p_btn->tsc, p_btn->group_index) > p_btn->touch_threshold);
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

    CAPTOUCH_BTN_handle_press_state(p_btn, CAPTOUCH_BTN_acquire_is_touched(p_btn));
}
