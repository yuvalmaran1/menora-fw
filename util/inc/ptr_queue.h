/**************************************************************************//**
 * @file      ptr_queue.h
 * @brief     pointer queue interface
 *
 * @author    ymaran
 * @date      Oct 30, 2021
 * @copyright 2023  Ltd
 *****************************************************************************/

#ifndef CTUTIL_PTR_QUEUE_H_
#define CTUTIL_PTR_QUEUE_H_

/******************************************************************************
 * Dependencies
 *****************************************************************************/
#include <stdint.h>
/******************************************************************************
 * Constants
 *****************************************************************************/

/******************************************************************************
 * Data types
 *****************************************************************************/
/*!< initial configuration */
typedef struct
{
	void** 		ptr_array;
	uint32_t	array_length;
} PTR_QUEUE_INIT_CONFIG_st;

/*!< context struct */
typedef struct
{
	void** 		ptr_array;
	uint32_t	array_length;
	uint32_t 	used;
	uint32_t	head;
	uint32_t	tail;
} PTR_QUEUE_st;

/******************************************************************************
 * Public function prototypes
 *****************************************************************************/
/***************************************************************************//**
 * Initialize queue
 *
 * @param[in] queue 		queue to be initialized
 * @param[in] ptr_array		array of pointers which serves as buffer
 * @param[in] length		max items in queue (array length)
 *
 * @return None
 ******************************************************************************/
void PTR_QUEUE_init(PTR_QUEUE_st* queue, void* ptr_array, uint32_t length);

/***************************************************************************//**
 * Enqueue an item
 *
 * @param[in] queue 		queue
 * @param[in] item			item to enqueue
 *
 * @return None
 ******************************************************************************/
void PTR_QUEUE_enqueue(PTR_QUEUE_st* queue, void* item);

/***************************************************************************//**
 * Dequeue an item
 *
 * @param[in] queue 		queue
 * @param[in] item			item to enqueue
 *
 * @return None
 ******************************************************************************/
void PTR_QUEUE_dequeue(PTR_QUEUE_st* queue, void** item);

/***************************************************************************//**
 * Flush queue
 *
 * @param[in] queue 		queue to be flushed
 *
 * @return None
 ******************************************************************************/
void PTR_QUEUE_flush(PTR_QUEUE_st* queue);

/***************************************************************************//**
 * Number of used items
 *
 * @param[in] queue 		queue
 *
 * @return number of used items
 ******************************************************************************/
uint32_t PTR_QUEUE_used(PTR_QUEUE_st* queue);

/***************************************************************************//**
 * Number of available items
 *
 * @param[in] queue 		queue
 *
 * @return number of available items
 ******************************************************************************/
uint32_t PTR_QUEUE_available(PTR_QUEUE_st* queue);

#endif /* CTUTIL_PTR_QUEUE_H_ */
