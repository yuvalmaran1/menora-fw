/**************************************************************************//**
 * @file      ptr_queue.c
 * @brief     pointer queue implementation
 *
 * @author    ymaran
 * @date      Oct 30, 2021
 * @copyright 2023  Ltd
 *****************************************************************************/

/******************************************************************************
 * Dependencies
 *****************************************************************************/
#include <string.h>
#include "ptr_queue.h"

#include "osal.h"
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

/******************************************************************************
 * Public functions
 *****************************************************************************/
//_____________________________________________________________________________
void PTR_QUEUE_init(PTR_QUEUE_st* queue, void* ptr_array, uint32_t length)
{
	/* clear context struct */
	memset(queue, 0, sizeof(PTR_QUEUE_st));

	/* initialize */
	queue->ptr_array 	= ptr_array;
	queue->array_length	= length;
	queue->used			= 0;
	queue->head			= 0;
	queue->tail			= 0;
}

//_____________________________________________________________________________
void PTR_QUEUE_enqueue(PTR_QUEUE_st* queue, void* item)
{
	OSAL_critical_section_enter();

	/* avoid overflow */
	if (queue->used >= queue->array_length)
	{
		ASSERT(0, "Queue overflow");
	}

	/* enqueue */
	queue->ptr_array[queue->head] = item;
	queue->head = (queue->head + 1) % queue->array_length;
	queue->used++;

	OSAL_critical_section_exit();
}

//_____________________________________________________________________________
void PTR_QUEUE_dequeue(PTR_QUEUE_st* queue, void** item)
{
	OSAL_critical_section_enter();

	/* avoid underflow */
	if (queue->used == 0)
	{
		ASSERT(0, "Queue underflow");
	}

	/* enqueue */
	*item = queue->ptr_array[queue->tail];
	queue->tail = (queue->tail + 1) % queue->array_length;
	queue->used--;

	OSAL_critical_section_exit();
}

//_____________________________________________________________________________
void PTR_QUEUE_flush(PTR_QUEUE_st* queue)
{
	OSAL_critical_section_enter();

	queue->used			= 0;
	queue->head			= 0;
	queue->tail			= 0;

	OSAL_critical_section_exit();
}

//_____________________________________________________________________________
uint32_t PTR_QUEUE_used(PTR_QUEUE_st* queue)
{
	uint32_t used;

	OSAL_critical_section_enter();

	used = queue->used;

	OSAL_critical_section_exit();
	return used;
}

//_____________________________________________________________________________
uint32_t PTR_QUEUE_available(PTR_QUEUE_st* queue)
{
	uint32_t available;

	OSAL_critical_section_enter();

	available = queue->array_length - queue->used;

	OSAL_critical_section_exit();
	return available;
}
