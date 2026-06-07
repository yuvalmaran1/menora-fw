/**************************************************************************//**
 * @file      list.c
 * @brief     linked list implementation
 *
 * @author    ymaran
 * @date      Oct 30, 2021
 * @copyright 2023  Ltd
 *****************************************************************************/


/******************************************************************************
 * Dependencies
 *****************************************************************************/
#include <string.h>
#include "list.h"
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
void LIST_init(LIST_st* list, LIST_ITEM_st* list_entry_array, uint16_t num_items)
{
    uint32_t i;

    /* init array */
    memset(list_entry_array, 0, num_items*sizeof(LIST_ITEM_st));

    list->used_head = NULL;
    list->free_head = list_entry_array;
    list->num_items = num_items;
    list->used = 0;
    list->entry_array = list_entry_array;

    /* iterate over items and add them to free list */
    for (i=0; i<list->num_items-1; i++)
    {
        list_entry_array[i].next = &list_entry_array[i + 1];
    }
}

//_____________________________________________________________________________
LIST_STATUS_t LIST_insert(LIST_st* list, void* item)
{
    LIST_STATUS_t status = LIST_STATUS_OK;
    LIST_ITEM_st* entry;
    LIST_ITERATOR_t iter;

    OSAL_critical_section_enter();
    
    if (list->used == list->num_items)
    {
        status = LIST_STATUS_FULL;
    }
    else
    {
        /* take free list head and populate it */
        entry = list->free_head;
        list->free_head = entry->next;
        entry->next = NULL;
        entry->item = item;

        /* if no items exist add as head */
        if (NULL == list->used_head)
        {
            list->used_head = entry;
        }
        else
        {
            /* find end of used list */
            iter = list->used_head;
            while (NULL != iter->next)
            {
                iter = iter->next;
            }

            iter->next 		= entry;
        }

        list->used++;
    }
    
    OSAL_critical_section_exit();
    
    return status;
}

//_____________________________________________________________________________
LIST_STATUS_t LIST_insert_after(LIST_st* list, void* item, void* after)
{
    LIST_STATUS_t status = LIST_STATUS_OK;
    LIST_ITEM_st* pred;
    LIST_ITEM_st* entry;

    OSAL_critical_section_enter();

    /* check if there is room in list */
    if (list->used == list->num_items)
    {
        status = LIST_STATUS_FULL;
    }
    else
    {
        /* locate predecessor after which node should be added */
        pred = LIST_contains(list, after);

        if (NULL == pred)
        {
            status = LIST_STATUS_NOT_FOUND;
        }
        else
        {
            /* take free list head and populate it */
            entry = list->free_head;
            list->free_head = entry->next;
            entry->item = item;
            entry->next = pred->next;
            pred->next	= entry;

            list->used++;
        }
    }
    
    OSAL_critical_section_exit();

    return status;
}

//_____________________________________________________________________________
LIST_STATUS_t LIST_remove(LIST_st* list, void* item)
{
    LIST_STATUS_t status = LIST_STATUS_NOT_FOUND;
    LIST_ITERATOR_t iter;
    LIST_ITERATOR_t entry_to_remove;

    OSAL_critical_section_enter();

    if (NULL == list->used_head)
    {
        status = LIST_STATUS_NOT_FOUND;
    }
    else
    {
        /* locate entry */
        iter = list->used_head;

        /* head requires special handling */
        if (list->used_head->item == item)
        {
            /* move entry to free list at head*/
            list->used_head = list->used_head->next;
            iter->next = list->free_head;
            iter->item = NULL;
            list->free_head = iter;

            list->used--;

            status = LIST_STATUS_OK;
        }
        else
        {
            while(NULL != iter->next)
            {
                if (iter->next->item == item)
                {
                    entry_to_remove = iter->next;

                    iter->next = entry_to_remove->next;
                    entry_to_remove->next = list->free_head;
                    entry_to_remove->item = NULL;
                    list->free_head = entry_to_remove;

                    list->used--;

                    status = LIST_STATUS_OK;
                    break;
                }

                iter = LIST_iterator_next(iter);
            }
        }
    }
    
    OSAL_critical_section_exit();
    
    return status;
}

//_____________________________________________________________________________
LIST_ITERATOR_t LIST_get_iterator(LIST_st* list)
{
    LIST_ITERATOR_t it;
    
    OSAL_critical_section_enter();

    it = list->used_head;

    OSAL_critical_section_exit();
    
    return it;
}

//_____________________________________________________________________________
LIST_ITERATOR_t LIST_iterator_next(LIST_ITERATOR_t iterator)
{
    LIST_ITERATOR_t it;
    
    OSAL_critical_section_enter();

    it = iterator->next;

    OSAL_critical_section_exit();
    
    return it;
}

//_____________________________________________________________________________
void* LIST_iterator_item(LIST_ITERATOR_t iterator)
{
    void* item;
    
    OSAL_critical_section_enter();

    item = iterator->item;

    OSAL_critical_section_exit();
    
    return item;
}

//_____________________________________________________________________________
LIST_ITERATOR_t LIST_contains(LIST_st* list, void* item)
{
    LIST_ITERATOR_t iter = NULL;
    
    OSAL_critical_section_enter();

    /* if list is empty - list doesn't contain item */
    if (NULL != list->used_head)
    {
        
        iter = list->used_head;
        while (NULL != iter)
        {
            if (iter->item == item)
            {
                break;
            }

            iter = LIST_iterator_next(iter);
        }
    }
    
    OSAL_critical_section_exit();
    
    return iter;
}

//_____________________________________________________________________________
void* LIST_find_first(LIST_st* list, LIST_PREDICATE_FUNC_t predicate, void* context)
{
    LIST_ITERATOR_t iter = NULL;
    void* item = NULL;

    OSAL_critical_section_enter();

    iter = LIST_get_iterator(list);

    while(NULL != iter)
    {
        item = LIST_iterator_item(iter);

        if (predicate(context, item))
        {
            break;
        }
        else
        {
            item = NULL;
        }

        iter = LIST_iterator_next(iter);
    }

    OSAL_critical_section_exit();
    
    return item;
}
