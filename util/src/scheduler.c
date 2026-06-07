/**************************************************************************//**
 * @file      scheduler.c
 * @brief     scheduler implementation
 *
 * @author    ymaran
 * @date      Oct 30, 2021
 * @copyright 2023  Ltd
 *****************************************************************************/
 
//***************************************************************************//
// Dependencies                                                              //
//***************************************************************************//
#include <string.h>
#include "scheduler.h"
#include "osal.h"
#include "assert.h"

//***************************************************************************//
// Constants & Macros                                                        //
//***************************************************************************//

//***************************************************************************//
// Typedefs                                                                  //
//***************************************************************************//

//***************************************************************************//
// Global Variables                                                          //
//***************************************************************************//

//***************************************************************************//
// Static Functions                                                          //
//***************************************************************************//
//_____________________________________________________________________________

//***************************************************************************//
// Public Functions                                                          //
//***************************************************************************//
//_____________________________________________________________________________
void SCHEDULER_init(SCHEDULER_st* scheduler, SCHEDULER_SLOT_st* slot_buffer, LIST_ITEM_st* list_buffer, void* queue_buffer, uint16_t num_items)
{
	uint16_t i;

	ASSERT(scheduler, "NULL context pointer");
	ASSERT(slot_buffer, "NULL slot buffer");
	ASSERT(list_buffer, "NULL list buffer");
	ASSERT(queue_buffer, "NULL queue buffer");

	// initialize the data structure
	memset(scheduler, 0, sizeof(SCHEDULER_st));

	LIST_init(&scheduler->allocated_slot_list, list_buffer, num_items);
	PTR_QUEUE_init(&scheduler->free_slot_queue, queue_buffer, num_items);

	/* fill queue */
	for(i=0; i<num_items; i++)
	{
		PTR_QUEUE_enqueue(&scheduler->free_slot_queue, &slot_buffer[i]);
	}
}

//_____________________________________________________________________________
void SCHEDULER_tick(SCHEDULER_st* scheduler)
{
  LIST_ITERATOR_t iter;
  SCHEDULER_SLOT_st* slot;

  OSAL_critical_section_enter();

  scheduler->tick++;

  // iterate over slots, decrementing time left
  iter = LIST_get_iterator(&scheduler->allocated_slot_list);

  while(iter != NULL)
  {
	  slot = (SCHEDULER_SLOT_st*)iter->item;

	  if (slot->active)
	  {
		  slot->ticks_left--;

		  // time to call the task
		  if (0 == slot->ticks_left)
		  {
			  TASK_HANDLER_enqueue(slot->task,
								   slot->context);

			  // reschedule if task is recurring
			  if (slot->recurring)
			  {
				  slot->ticks_left = slot->interval_ticks;
			  }
			  else
			  {
				  slot->active = false;
			  }
		  }
	  }

	  /* next slot */
	  iter = LIST_iterator_next(iter);
  }

  OSAL_critical_section_exit();
}

//_____________________________________________________________________________
SCHEDULER_SLOT_st* SCHEDULER_allocate_slot(SCHEDULER_st* scheduler)
{
	SCHEDULER_SLOT_st* slot;

	OSAL_critical_section_enter();

	ASSERT(scheduler, "NULL context pointer");

	PTR_QUEUE_dequeue(&scheduler->free_slot_queue, (void**)&slot);

	LIST_insert(&scheduler->allocated_slot_list, slot);

	OSAL_critical_section_exit();

	return slot;
}

//_____________________________________________________________________________
void SCHEDULER_free_slot(SCHEDULER_st* scheduler, SCHEDULER_SLOT_st* slot)
{
	OSAL_critical_section_enter();

	ASSERT(scheduler, "NULL context pointer");
	ASSERT(slot, "NULL slot pointer");
	ASSERT(LIST_contains(&scheduler->allocated_slot_list, slot), "Slot not allocated!");

	LIST_remove(&scheduler->allocated_slot_list, slot);

	PTR_QUEUE_enqueue(&scheduler->free_slot_queue, slot);

	OSAL_critical_section_exit();
}

//_____________________________________________________________________________
void SCHEDULER_schedule(SCHEDULER_st* scheduler, SCHEDULER_SLOT_st* slot, TASK_f task, void* context, uint32_t  interval, bool recurring)
{
  OSAL_critical_section_enter();

  ASSERT(scheduler, "NULL context pointer");
  ASSERT(slot, "NULL slot pointer");
  ASSERT(!slot->active, "Slot already active");
  ASSERT(LIST_contains(&scheduler->allocated_slot_list, slot), "Slot not allocated");

  slot->task = task;
  slot->context = context;
  slot->interval_ticks = interval;
  slot->ticks_left = interval;
  slot->recurring = recurring;
  slot->active = true;

  OSAL_critical_section_exit();
}

//_____________________________________________________________________________
void SCHEDULER_abort(SCHEDULER_st* scheduler, SCHEDULER_SLOT_st* slot)
{
  OSAL_critical_section_enter();

  ASSERT(scheduler, "NULL context pointer");
  ASSERT(LIST_contains(&scheduler->allocated_slot_list, slot), "Slot not allocated");

  slot->active = false;

  OSAL_critical_section_exit();
}

//_____________________________________________________________________________
bool SCHEDULER_is_slot_active(SCHEDULER_st* scheduler, SCHEDULER_SLOT_st* slot)
{
  bool active;

  ASSERT(scheduler, "NULL context pointer");
  ASSERT(LIST_contains(&scheduler->allocated_slot_list, slot), "Slot not allocated");

  OSAL_critical_section_enter();

  active = slot->active;

  OSAL_critical_section_exit();

  return active;
}

//_____________________________________________________________________________
uint32_t SCHEDULER_slot_ticks_left(SCHEDULER_st* scheduler, SCHEDULER_SLOT_st* slot)
{
  uint32_t ticks_left;

  ASSERT(scheduler, "NULL context pointer");

  OSAL_critical_section_enter();

  ASSERT(LIST_contains(&scheduler->allocated_slot_list, slot), "Slot not allocated");
  ASSERT(slot->active, "Slot not active");

  ticks_left = slot->ticks_left;

  OSAL_critical_section_exit();

  return ticks_left;
}

//_____________________________________________________________________________
uint32_t SCHEDULER_get_tick(SCHEDULER_st* scheduler)
{
	uint32_t tick;

	OSAL_critical_section_enter();

	tick = scheduler->tick;

	OSAL_critical_section_exit();

	return tick;
}
