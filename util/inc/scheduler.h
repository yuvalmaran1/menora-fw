/**************************************************************************//**
 * @file      scheduler.h
 * @brief     scheduler interface
 *
 * @author    ymaran
 * @date      Oct 30, 2021
 * @copyright 2023  Ltd
 *****************************************************************************/

#ifndef CTUTIL_SCHEDULER_H_
#define CTUTIL_SCHEDULER_H_

//***************************************************************************//
// Dependencies                                                              //
//***************************************************************************//
#include <stdint.h>
#include <stdbool.h>
#include "task_handler.h"
#include "ctlist.h"
#include "ptr_queue.h"
//***************************************************************************//
// Constants & Macros                                                        //
//***************************************************************************//


//***************************************************************************//
// Public Typedefs                                                           //
//***************************************************************************//
/*
 * Scheduler slot struct
 */
typedef struct
{
  bool      active;         ///< indicates if slot is active
  TASK_f    task;           ///< task which is scheduled
  void*     context;        ///< context variable to be passed to task
  uint32_t  interval_ticks; ///< task scheduling interval in ticks
  uint32_t  ticks_left;     ///< number of ticks left until invocation
  bool      recurring;      ///< indicates whether task is recurring
} SCHEDULER_SLOT_st;

/*
 * Context struct
 */
typedef struct
{
  SCHEDULER_SLOT_st*  slot_buffer;          ///< scheduler slot buffer
  LIST_st			  allocated_slot_list;  ///< linked list for allocated slots
  PTR_QUEUE_st		  free_slot_queue;      ///< queue for unallocated slots
  uint32_t            tick;                 ///< tick counter
} SCHEDULER_st;

//***************************************************************************//
// Public Function Prototypes                                                //
//***************************************************************************//
/******************************************************************************
* @brief   Initialize scheduler
*
* @param[in] scheduler    context struct pointer
* @param[in] slot_buffer  external buffer to hold slots. should have @p num_items slots
* @param[in] list_buffer  external buffer to hold linked list data. should have @p num_items items
* @param[in] queue_buffer external buffer to hold queue pointers. should have @p num_items items
* @param[in] num_items    number of slots in scheduler data structures
*
* @param   scheduler  module context struct
*****************************************************************************/
void SCHEDULER_init(SCHEDULER_st* scheduler, SCHEDULER_SLOT_st* slot_buffer, LIST_ITEM_st* list_buffer, void* queue_buffer, uint16_t num_items);

/******************************************************************************
* @brief   Tick handler. Call this function from time-base interrupt service routine
*
* @param   scheduler  module context struct
*****************************************************************************/
void SCHEDULER_tick(SCHEDULER_st* scheduler);

/******************************************************************************
* @brief   allocate a slot
*
* @param[in] scheduler    context struct pointer
*
* @return Pointer to slot object. NULL if no resource is available
*****************************************************************************/
SCHEDULER_SLOT_st* SCHEDULER_allocate_slot(SCHEDULER_st* scheduler);

/******************************************************************************
* @brief  release a slot
*
* @param[in] scheduler    context struct pointer
* @param[in] slot         Pointer to slot object
*
* @return None
*****************************************************************************/
void SCHEDULER_free_slot(SCHEDULER_st* scheduler, SCHEDULER_SLOT_st* slot);

/******************************************************************************
* @brief   schedule a task
*
* @param   scheduler  module context struct
* @param   slot       slot to assign for task (use contextual enum for clarity)
* @param   task       task to be scheduled
* @param   context    context passed to task
* @param   interval   interval in ticks
* @param   recurring  is task recurring
*****************************************************************************/
void SCHEDULER_schedule(SCHEDULER_st* scheduler, SCHEDULER_SLOT_st* slot, TASK_f task, void* context, uint32_t  interval, bool recurring);

/******************************************************************************
* @brief   abort task (deactivate)
*
* @param   scheduler  module context struct
* @param   slot       slot to assign for task (use contextual enum for clarity)
*****************************************************************************/
void SCHEDULER_abort(SCHEDULER_st* scheduler, SCHEDULER_SLOT_st* slot);

/******************************************************************************
* @brief   Slot state indication
*
* @param   scheduler  module context struct
* @param   slot       slot of interest
*
* @return  bool       'true' if slot is active. 'false' otherwise
*****************************************************************************/
bool SCHEDULER_is_slot_active(SCHEDULER_st* scheduler, SCHEDULER_SLOT_st* slot);

/******************************************************************************
* @brief   Ticks left before slot expires
*
* @param   scheduler  module context struct
* @param   slot       slot of interest
*
* @return  uint32_t   number of ticks to expiration
*****************************************************************************/
uint32_t SCHEDULER_slot_ticks_left(SCHEDULER_st* scheduler, SCHEDULER_SLOT_st* slot);

/******************************************************************************
* @brief   Get ticks since startup
*
* @param   scheduler  module context struct
*
* @return  uint32_t   tick
*****************************************************************************/
uint32_t SCHEDULER_get_tick(SCHEDULER_st* scheduler);

#endif /* CTUTIL_SCHEDULER_H_ */
