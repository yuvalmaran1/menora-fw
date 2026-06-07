/******************************************************************************
 * @file      task_handler.c
 * @brief     task handler implementation
 *
 * @author    ymaran
 * @date      Jun 7, 2026
 *****************************************************************************/
//*****************************************************************************
// Dependencies                                                              
//*****************************************************************************
#include "assert.h"
#include "task_handler.h"
#include "queue.h"

//*****************************************************************************
// Constants & Macros                                                        
//*****************************************************************************

//*****************************************************************************
// Data types                                                                  
//*****************************************************************************
/*
 * Task struct
 */
typedef struct 
{
    TASK_f task;
    void *context;
} TASK_HANDLER_TASK_st;

//*****************************************************************************
// Static Variables                                                          
//*****************************************************************************
QUEUE_DECLARE(task_queue, TASK_HANDLER_QUEUE_LENGTH, TASK_HANDLER_TASK_st);

//*****************************************************************************
// Static Functions                                                          
//*****************************************************************************
//_____________________________________________________________________________

//*****************************************************************************
// Public Functions                                                          
//*****************************************************************************
//_____________________________________________________________________________
void TASK_HANDLER_init(void) 
{
    QUEUE_INIT(task_queue);
}

//_____________________________________________________________________________
bool TASK_HANDLER_handle(void) 
{
    TASK_HANDLER_TASK_st task = {0};

    if (!QUEUE_EMPTY(task_queue)) {
        // dequeue task
        QUEUE_DEQUEUE(task_queue, task);

        // check for null pointer
        CTOSAL_ASSERT(task.task, "NULL task");

        // execute
        task.task(task.context);
    }

    return (!QUEUE_EMPTY(task_queue));
}

//_____________________________________________________________________________
void TASK_HANDLER_enqueue(TASK_f task, void *context) 
{
    TASK_HANDLER_TASK_st tmp_task;

    // assign to local
    tmp_task.task = task;
    tmp_task.context = context;

    // enqueue
    QUEUE_ENQUEUE(task_queue, tmp_task);
}
