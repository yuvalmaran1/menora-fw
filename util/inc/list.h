/**************************************************************************//**
 * @file      list.h
 * @brief     linked list interface
 *
 * @author    ymaran
 * @date      Oct 30, 2021
 * @copyright 2023  Ltd
 *****************************************************************************/
 
#ifndef UTIL_LIST_H_
#define UTIL_LIST_H_

/******************************************************************************
 * Dependencies
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * Constants
 *****************************************************************************/

/******************************************************************************
 * Status
 *****************************************************************************/
/*! @typedef LIST_STATUS_t
 * return status. 0 success. <0 error.
 */
typedef int32_t LIST_STATUS_t;

#define LIST_STATUS_OK        ((LIST_STATUS_t)( 0))  ///< operation was successful
#define LIST_STATUS_FULL      ((LIST_STATUS_t)(-1))  ///< No more space in list array
#define LIST_STATUS_NOT_FOUND ((LIST_STATUS_t)(-2))  ///< Item not found in list

/******************************************************************************
 * Data types
 *****************************************************************************/
typedef struct LIST_ITEM_st
{
    void* item;
    struct LIST_ITEM_st* next;
} LIST_ITEM_st;

typedef struct
{
    LIST_ITEM_st* used_head;
    LIST_ITEM_st* free_head;
    uint32_t num_items;
    uint32_t used;
    LIST_ITEM_st* entry_array;
} LIST_st;

typedef LIST_ITEM_st* LIST_ITERATOR_t;

typedef bool (*LIST_PREDICATE_FUNC_t)(void* context, void* item);

/******************************************************************************
 * Public function prototypes
 *****************************************************************************/
/***************************************************************************//**
 * Initialize list
 *
 * @param[in] list 					pointer to list
 * @param[in] list_entry_array		pre-allocated entry array
 * @param[in] num_items				number of entries in entry array
 *
 * @return None
 ******************************************************************************/
void LIST_init(LIST_st* list, LIST_ITEM_st* list_entry_array, uint16_t num_items);

/***************************************************************************//**
 * Insert item to list end
 *
 * @param[in] list pointer to list
 * @param[in] item pointer to item to add
 *
 * @return operation result
 ******************************************************************************/
LIST_STATUS_t LIST_insert(LIST_st* list, void* item);

/***************************************************************************//**
 * Insert item after specific item
 *
 * @param[in] list	pointer to list
 * @param[in] item  pointer to item to add
 * @param[in] after pointer to item after which the new item shall be added
 *
 * @return operation result
 ******************************************************************************/
LIST_STATUS_t LIST_insert_after(LIST_st* list, void* item, void* after);

/***************************************************************************//**
 * Remove item from list
 *
 * @param[in] list pointer to list
 * @param[in] item pointer to item to remove
 *
 * @return operation result
 ******************************************************************************/
LIST_STATUS_t LIST_remove(LIST_st* list, void* item);

/***************************************************************************//**
 * Get list iterator. used for traversing list
 *
 * @param[in] list pointer to list
 *
 * @return iterator
 ******************************************************************************/
LIST_ITERATOR_t LIST_get_iterator(LIST_st* list);

/***************************************************************************//**
 * List iterator next entry
 *
 * @param[in] iterator list iterator
 *
 * @return next entry. NULL if next item doesn't exist
 ******************************************************************************/
LIST_ITERATOR_t LIST_iterator_next(LIST_ITERATOR_t iterator);

/***************************************************************************//**
 * List iterator item
 *
 * @param[in] iterator list iterator
 *
 * @return entry item.
 ******************************************************************************/
void* LIST_iterator_item(LIST_ITERATOR_t iterator);

/***************************************************************************//**
 * Locate item in list
 *
 * @param[in] list pointer to list
 * @param[in] item pointer to item to find
 *
 * @return pointer to item entry if found. NULL otherwise
 ******************************************************************************/
LIST_ITERATOR_t LIST_contains(LIST_st* list, void* item);

/***************************************************************************//**
 * Locate first item in list which fulfills condition
 *
 * @param[in] list 		pointer to list
 * @param[in] predicate pointer to predicate function
 * @param[in] context	context in which predicate is invoked
 *
 * @return pointer to item if found. NULL otherwise
 ******************************************************************************/
void* LIST_find_first(LIST_st* list, LIST_PREDICATE_FUNC_t predicate, void* context);

#endif /* UTIL_LIST_H_ */
