/**************************************************************************//**
 * @file      ms_scheduler.h
 * @brief     milli-second scheduler interface
 *
 * @author    ymaran
 * @date      Oct 30, 2021
 * @copyright 2023  Ltd
 *****************************************************************************/
 
#ifndef CTUTIL_MS_SCHEDULER_H_
#define CTUTIL_MS_SCHEDULER_H_

//***************************************************************************//
// Dependencies                                                              //
//***************************************************************************//
#include <stdint.h>
#include <stdbool.h>
#include "task_handler.h"
#include "stm32l0xx_hal.h"

//***************************************************************************//
// Constants & Macros                                                        //
//***************************************************************************//
/*!< number of preallocated slots */
#ifndef MS_SCHEDULER_NUM_SLOTS
#define MS_SCHEDULER_NUM_SLOTS	(25)
#endif

//***************************************************************************//
// Public Typedefs                                                           //
//***************************************************************************//
/*!< opaque slot type definition */
typedef struct MS_SCHEDULER_SLOT_s MS_SCHEDULER_SLOT_t;

/*!< initial configuration */
typedef struct
{
    TIM_HandleTypeDef* timer;
} MS_SCHEDULER_INIT_CONFIG_st;

//***************************************************************************//
// Public Function Prototypes                                                //
//***************************************************************************//
/******************************************************************************
* @brief   Initialize module
*
* @param[in] p_init_config pointer to initial configuration
*****************************************************************************/
void MS_SCHEDULER_init(MS_SCHEDULER_INIT_CONFIG_st* p_init_config);

/******************************************************************************
* @brief   Pause scheduler
*
* @param[in] pause true to pause, false to resume
*****************************************************************************/
void MS_SCHEDULER_pause(bool pause);

/******************************************************************************
* @brief   allocate a slot
*
* @return Pointer to slot object. NULL if no resource is available
*****************************************************************************/
MS_SCHEDULER_SLOT_t* MS_SCHEDULER_allocate_slot(void);

/******************************************************************************
* @brief  release a slot
*
* @param[in] slot Pointer to slot object
*
* @return None
*****************************************************************************/
void MS_SCHEDULER_free_slot(MS_SCHEDULER_SLOT_t* slot);

/******************************************************************************
* @brief   schedule a task
*
* @param   slot       slot to assign for task
* @param   task       task to be scheduled
* @param   context    context passed to task
* @param   interval   interval in milli-sec
* @param   recurring  is task recurring
*****************************************************************************/
void MS_SCHEDULER_schedule(MS_SCHEDULER_SLOT_t* slot, TASK_f task, void* context, uint32_t  interval_ms, bool recurring);

/******************************************************************************
* @brief   abort task (deactivate)
*
* @param   slot       slot to assign for task (use contextual enum for clarity)
*****************************************************************************/
void MS_SCHEDULER_abort(MS_SCHEDULER_SLOT_t* slot);

/******************************************************************************
* @brief   Slot state indication
*
* @param   slot       slot of interest
*
* @return  bool       'true' if slot is active. 'false' otherwise
*****************************************************************************/
bool MS_SCHEDULER_is_slot_active(MS_SCHEDULER_SLOT_t* slot);

/******************************************************************************
* @brief   milli-secs left before slots expires
*
* @param   slot       slot of interest
*
* @return  uint32_t   number of ms to expiration
*****************************************************************************/
uint32_t MS_SCHEDULER_slot_ms_left(MS_SCHEDULER_SLOT_t* slot);

/******************************************************************************
* @brief   Get ticks since startup
*
* @return  uint32_t   tick
*****************************************************************************/
uint32_t MS_SCHEDULER_get_tick(void);

#endif /* UTIL_MS_SCHEDULER_H_ */
