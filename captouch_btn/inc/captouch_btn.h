/**************************************************************************//**
 * @file      captouch_btn.h
 * @brief     Capacitive touch-sense push button interface
 *
 * @author    ymaran
 * @date      7.6.2026
 *****************************************************************************/
#ifndef CAPTOUCH_BTN_H_
#define CAPTOUCH_BTN_H_

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx_hal.h"

/******************************************************************************
 * Constants
 *****************************************************************************/
/*!< default definitions */
#if !defined(CAPTOUCH_BTN_SHORT_PRESS_MAX_MS)
    #define CAPTOUCH_BTN_SHORT_PRESS_MAX_MS     (1000)  // max hold duration recognized as a short press
#endif

#if !defined(CAPTOUCH_BTN_LONG_PRESS_MIN_MS)
    #define CAPTOUCH_BTN_LONG_PRESS_MIN_MS      (2000)  // min hold duration recognized as a long press
#endif

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Status
 *****************************************************************************/
/*! @typedef CAPTOUCH_BTN_STATUS_t
 * return status. 0 success. <0 error.
 */
typedef int32_t CAPTOUCH_BTN_STATUS_t;

#define CAPTOUCH_BTN_STATUS_OK                      ((CAPTOUCH_BTN_STATUS_t)( 0)) //!< no error
#define CAPTOUCH_BTN_STATUS_INVALID_ARG              ((CAPTOUCH_BTN_STATUS_t)(-1)) //!< invalid argument

/******************************************************************************
 * Data types
 *****************************************************************************/
/*!< button activation event callback
 * @param[in] ctx user context, as registered along with the callback
 */
typedef void (*CAPTOUCH_BTN_callback_t)(void* ctx);

/* press state machine */
typedef enum
{
    CAPTOUCH_BTN_STATE_IDLE,
    CAPTOUCH_BTN_STATE_PRESSED,
} CAPTOUCH_BTN_STATE_en;

/*!< initial configuration struct */
typedef struct
{
    TSC_HandleTypeDef* tsc;     // shared TSC peripheral handle
    uint32_t group_index;       // TSC group index this button's channel IO belongs to (e.g. TSC_GROUP1_IDX)
    uint32_t channel_io;        // this button's channel IO mask (e.g. TSC_GROUP1_IO1)
    uint32_t sampling_io;       // sampling IO mask shared by the group (e.g. TSC_GROUP1_IO4)
    uint32_t touch_threshold;   // acquisition count above which the electrode is considered touched
} CAPTOUCH_BTN_INIT_CONFIG_st;

/*!< instance context */
typedef struct CAPTOUCH_BTN_s CAPTOUCH_BTN_st;

struct CAPTOUCH_BTN_s
{
    TSC_HandleTypeDef* tsc;
    uint32_t group_index;
    uint32_t channel_io;
    uint32_t sampling_io;
    uint32_t touch_threshold;

    CAPTOUCH_BTN_STATE_en state;
    uint32_t press_start_tick;
    bool long_press_fired;

    CAPTOUCH_BTN_callback_t on_short_press;
    void* short_press_ctx;

    CAPTOUCH_BTN_callback_t on_long_press;
    void* long_press_ctx;

    /* internal - background TSC acquisition (interrupt-driven, round-robin across instances) */
    volatile bool acq_done;     // true once a fresh acquisition result is available to harvest
    volatile bool acq_touched;  // latest harvested acquisition result
    CAPTOUCH_BTN_st* next;      // intrusive link for round-robin acquisition scheduling
};

/******************************************************************************
 * Public function prototypes
 *****************************************************************************/
/***************************************************************************//**
 * Initialize device
 *
 * @param[in] p_btn button instance
 * @param[in] p_init_config initial configuration
 *
 * @return procedure result
 ******************************************************************************/
CAPTOUCH_BTN_STATUS_t CAPTOUCH_BTN_init(CAPTOUCH_BTN_st* p_btn, CAPTOUCH_BTN_INIT_CONFIG_st* p_init_config);

/***************************************************************************//**
 * Register a callback for the short press activation event (hold duration up
 * to @ref CAPTOUCH_BTN_SHORT_PRESS_MAX_MS), invoked on release
 *
 * @param[in] p_btn button instance
 * @param[in] cb callback function, or NULL to unregister
 * @param[in] ctx user context, passed back to the callback as-is
 *
 * @return procedure result
 ******************************************************************************/
CAPTOUCH_BTN_STATUS_t CAPTOUCH_BTN_register_short_press_cb(CAPTOUCH_BTN_st* p_btn, CAPTOUCH_BTN_callback_t cb, void* ctx);

/***************************************************************************//**
 * Register a callback for the long press activation event (hold duration of
 * at least @ref CAPTOUCH_BTN_LONG_PRESS_MIN_MS), invoked as soon as the
 * threshold is crossed while the button is still held
 *
 * @param[in] p_btn button instance
 * @param[in] cb callback function, or NULL to unregister
 * @param[in] ctx user context, passed back to the callback as-is
 *
 * @return procedure result
 ******************************************************************************/
CAPTOUCH_BTN_STATUS_t CAPTOUCH_BTN_register_long_press_cb(CAPTOUCH_BTN_st* p_btn, CAPTOUCH_BTN_callback_t cb, void* ctx);

/***************************************************************************//**
 * Process button internals - harvests the latest touch sample for this
 * button's electrode and runs the press state machine
 *
 * @note This function should be called periodically (e.g. from the main loop
 *       or a scheduled task) and never blocks. Acquisition itself happens in
 *       the background, driven by TSC interrupts: all button instances created
 *       via @ref CAPTOUCH_BTN_init share the same TSC peripheral and are
 *       sampled one at a time in a round-robin fashion.
 *
 * @param[in] p_btn button instance
 *
 * @return none
 ******************************************************************************/
void CAPTOUCH_BTN_process(CAPTOUCH_BTN_st* p_btn);

#endif // CAPTOUCH_BTN_H_
