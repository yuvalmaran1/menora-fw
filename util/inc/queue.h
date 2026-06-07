/**************************************************************************//**
 * @file      queue.h
 * @brief     generic queue 
 *
 * @author    ymaran
 * @date      Oct 30, 2021
 * @copyright 2023  Ltd
 *****************************************************************************/

#ifndef UTIL_QUEUE_H_
#define UTIL_QUEUE_H_

//***************************************************************************//
// Dependencies                                                              //
//***************************************************************************//
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ctosal.h"

//***************************************************************************//
// Constants & Macros                                                        //
//***************************************************************************//
//_____________________________________________________________________________
#define QUEUE_TYPEDEF(qName, qSize, qType)                                  \
  typedef struct {                                                          \
    uint16_t head;                                                          \
    uint16_t tail;                                                          \
    uint16_t used;                                                          \
    uint16_t size;                                                          \
    qType fifo[qSize];                                                      \
  } _##qName##_t

//_____________________________________________________________________________
#define QUEUE_INSTANTIATE(qName) _##qName##_t qName

//_____________________________________________________________________________
#define QUEUE_DECLARE(qName, qSize, qType)                                  \
	QUEUE_TYPEDEF(qName, qSize, qType);                                     \
	static QUEUE_INSTANTIATE(qName)

//_____________________________________________________________________________
#define QUEUE_INIT(qName)                                                   \
    do                                                                      \
    {                                                                       \
        CTOSAL_critical_section_enter();                                    \
        memset(qName.fifo, 0, sizeof(qName.fifo));                          \
        qName.head = 0,                                                     \
        qName.tail = 0,                                                     \
        qName.used = 0,                                                     \
        qName.size = sizeof(qName.fifo)/                                    \
                     sizeof(qName.fifo[0]);                                 \
        CTOSAL_critical_section_exit();                                     \
    } while(0);

//_____________________________________________________________________________
#define QUEUE_ENQUEUE(qName, item)                                          \
    do                                                                      \
    {                                                                       \
        CTOSAL_critical_section_enter();                                    \
        if (qName.used < qName.size)                                        \
        {                                                                   \
            memcpy((void*)&qName.fifo[qName.head],                          \
                 (void*)&item,                                              \
                 sizeof(qName.fifo[0]));                                    \
            qName.head = (qName.head + 1) % qName.size;                     \
            qName.used ++;                                                  \
        }                                                                   \
        else                                                                \
        {                                                                   \
            CTOSAL_ASSERT(0, "Queue overflow");         	                \
        }                                                                   \
        CTOSAL_critical_section_exit();                                     \
    }while(0);

//_____________________________________________________________________________
#define QUEUE_ENQUEUE_MULTI(qName, items, amount)                           \
    do                                                                      \
    {                                                                       \
        CTOSAL_critical_section_enter();                                    \
        if (qName.used + amount <= qName.size)                              \
        {                                                                   \
            uint32_t items_left = amount;                                   \
            uint32_t iter_items = 0;                                        \
            if (qName.head + amount >= qName.size)                          \
            {                                                               \
                iter_items = qName.size - qName.head;                       \
                memcpy((void*)&qName.fifo[qName.head],                      \
                       (void*)&items,                                       \
                       iter_items * sizeof(qName.fifo[0]));                 \
                qName.head = 0;                                             \
                qName.used += iter_items;                                   \
                items_left -= iter_items;                                   \
            }                                                               \
            if (items_left > 0)                                             \
            {                                                               \
              memcpy((void*)&qName.fifo[qName.head],                        \
                     (void*)&items + iter_items * sizeof(qName.fifo[0]),    \
                     items_left * sizeof(qName.fifo[0]));                   \
              qName.head += items_left;                                     \
              qName.used += items_left;                                     \
            }                                                               \
        }                                                                   \
        else                                                                \
        {                                                                   \
          CTOSAL_ASSERT(0, "Queue overflow");                               \
        }                                                                   \
        CTOSAL_critical_section_exit();                                     \
    }while(0);


//_____________________________________________________________________________
#define QUEUE_DEQUEUE(qName, item)                                          \
    do                                                                      \
    {                                                                       \
        CTOSAL_critical_section_enter();                                    \
        if (qName.used > 0)                                                 \
        {                                                                   \
            memcpy((void*)&item,                                            \
                 (void*)&qName.fifo[qName.tail],                            \
                 sizeof(qName.fifo[0]));                                    \
            qName.tail = (qName.tail + 1) % qName.size;                     \
            qName.used --;                                                  \
        }                                                                   \
        else                                                                \
        {                                                                   \
            CTOSAL_ASSERT(0, "Queue underflow");        	                \
        }                                                                   \
        CTOSAL_critical_section_exit();                                     \
    } while(0);

//_____________________________________________________________________________
#define QUEUE_DEQUEUE_MULTI(qName, items, amount)                           \
    do                                                                      \
    {                                                                       \
        CTOSAL_critical_section_enter();                                    \
        if ((qName.used > 0) && (qName.used >= amount))                     \
        {                                                                   \
            uint32_t items_left = amount;                                   \
            uint32_t iter_items = 0;                                        \
            if (qName.tail + items_left >= qName.size)                      \
            {                                                               \
                iter_items = qName.size - qName.tail;                       \
                memcpy((void*)&items,                                       \
                       (void*)&qName.fifo[qName.tail],                      \
                       iter_items * sizeof(qName.fifo[0]));                 \
                qName.tail = 0;                                             \
                qName.used -= iter_items;                                   \
                items_left -= iter_items;                                   \
            }                                                               \
            if (items_left > 0)                                             \
            {                                                               \
              memcpy(((void*)&items) + iter_items * sizeof(qName.fifo[0]),  \
                     (void*)&qName.fifo[qName.tail],                        \
                     items_left * sizeof(qName.fifo[0]));                   \
              qName.tail += items_left;                                     \
              qName.used -= items_left;                                     \
            }                                                               \
        }                                                                   \
        else                                                                \
        {                                                                   \
          CTOSAL_ASSERT(0, "Queue underflow");                              \
        }                                                                   \
        CTOSAL_critical_section_exit();                                     \
    } while(0);


//_____________________________________________________________________________
#define QUEUE_ITEMS(qName)  (qName.used)

//_____________________________________________________________________________
#define QUEUE_AVAILABLE(qName)  ((uint32_t)(qName.size - QUEUE_ITEMS(qName)))

//_____________________________________________________________________________
#define QUEUE_EMPTY(qName)  (QUEUE_ITEMS(qName) == 0)

//_____________________________________________________________________________
#define QUEUE_FULL(qName)   (QUEUE_ITEMS(qName) == qName.size)

#endif /* UTIL_QUEUE_H_ */
