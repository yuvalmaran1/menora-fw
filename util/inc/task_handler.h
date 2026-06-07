/**************************************************************************//**
 * @file      task_handler.h
 * @brief     task handler interface
 *
 * @author    ymaran
 * @date      Oct 30, 2021
 * @copyright 2023  Ltd
 *****************************************************************************/

#ifndef CTUTIL_TASK_HANDLER_H_
#define CTUTIL_TASK_HANDLER_H_

//***************************************************************************//
// Dependencies                                                              //
//***************************************************************************//
#include <stdint.h>
#include <stdbool.h>

//***************************************************************************//
// Constants & Macros                                                        //
//***************************************************************************//
#ifndef TASK_HANDLER_QUEUE_LENGTH
#define TASK_HANDLER_QUEUE_LENGTH    (16)
#endif

//***************************************************************************//
// Public Typedefs                                                           //
//***************************************************************************//
typedef void (*TASK_f)(void* context);

//***************************************************************************//
// Public Function Prototypes                                                //
//***************************************************************************//
/******************************************************************************
* @brief   Initialize task handler
*
*****************************************************************************/
void TASK_HANDLER_init(void);

/******************************************************************************
* @brief   Process tasks
* @return  true if task was handled, false if idle
* @note    call this function from main superloop
*****************************************************************************/
bool TASK_HANDLER_handle(void);

/******************************************************************************
* @brief   Enqueue a task
*
* @param   task       Task to be enqueued
*
* @param   context    Parameter to be passed upon execution
*
* @note    this function is context safe
*****************************************************************************/
void TASK_HANDLER_enqueue(TASK_f task, void* context);


#endif /* CTUTIL_TASK_HANDLER_H_ */
