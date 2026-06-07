/**************************************************************************//**
 * @file      ms_scheduler.c
 * @brief     milli-second scheduler implementation
 *
 * @author    ymaran
 * @date      Oct 30, 2021
 * @copyright 2023  Ltd
 *****************************************************************************/
//***************************************************************************//
// Dependencies                                                              //
//***************************************************************************//
#include "ms_scheduler.h"
#include "scheduler.h"

//***************************************************************************//
// Constants & Macros                                                        //
//***************************************************************************//

//***************************************************************************//
// Typedefs                                                                  //
//***************************************************************************//
typedef struct
{
    CTHAL_TIM_HANDLE_t* timer;
    SCHEDULER_st        scheduler;
    LIST_ITEM_st        list_buffer[MS_SCHEDULER_NUM_SLOTS];
    void*               queue_buffer[MS_SCHEDULER_NUM_SLOTS];
    SCHEDULER_SLOT_st   slot_array[MS_SCHEDULER_NUM_SLOTS];
} MS_SCHEDULER_st;


//***************************************************************************//
// Static Variables                                                          //
//***************************************************************************//
static MS_SCHEDULER_st s_ms_scheduler = {0};

//***************************************************************************//
// Static Functions                                                          //
//***************************************************************************//
//_____________________________________________________________________________
void MS_SCHEDULER_timer_handler(CTHAL_TIM_HANDLE_t* handle, void* context)
{
    (void)handle;
    (void)context;

	SCHEDULER_tick(&s_ms_scheduler.scheduler);
}

//***************************************************************************//
// Public Functions                                                          //
//***************************************************************************//
//_____________________________________________________________________________
void MS_SCHEDULER_init(MS_SCHEDULER_INIT_CONFIG_st* p_init_config)
{
	/* assign timer */
	s_ms_scheduler.timer = p_init_config->timer;

    /* init scheduler */
    SCHEDULER_init(&s_ms_scheduler.scheduler,
                    s_ms_scheduler.slot_array,
                    s_ms_scheduler.list_buffer,
                    s_ms_scheduler.queue_buffer,
                    MS_SCHEDULER_NUM_SLOTS);

    /* register the callback */
    CTHAL_TIM_register_callback(s_ms_scheduler.timer, MS_SCHEDULER_timer_handler);

    /* start timer */
    CTHAL_TIM_start(s_ms_scheduler.timer);
}

//_____________________________________________________________________________
void MS_SCHEDULER_pause(bool pause)
{
    if (pause)
    {
        CTHAL_TIM_stop(s_ms_scheduler.timer);
    }
    else
    {
        CTHAL_TIM_start(s_ms_scheduler.timer);
    }
}

//_____________________________________________________________________________
MS_SCHEDULER_SLOT_t* MS_SCHEDULER_allocate_slot(void)
{
    return (void*)SCHEDULER_allocate_slot(&s_ms_scheduler.scheduler);
}

//_____________________________________________________________________________
void MS_SCHEDULER_free_slot(MS_SCHEDULER_SLOT_t* slot)
{
    SCHEDULER_free_slot(&s_ms_scheduler.scheduler, (SCHEDULER_SLOT_st*)slot);
}

//_____________________________________________________________________________
void MS_SCHEDULER_schedule(MS_SCHEDULER_SLOT_t* slot, TASK_f task, void* context, uint32_t  interval_ms, bool recurring)
{
    SCHEDULER_schedule(&s_ms_scheduler.scheduler, (SCHEDULER_SLOT_st*)slot, task, context, interval_ms, recurring);
}

//_____________________________________________________________________________
void MS_SCHEDULER_abort(MS_SCHEDULER_SLOT_t* slot)
{
    SCHEDULER_abort(&s_ms_scheduler.scheduler, (SCHEDULER_SLOT_st*)slot);
}

//_____________________________________________________________________________
bool MS_SCHEDULER_is_slot_active(MS_SCHEDULER_SLOT_t* slot)
{
    return SCHEDULER_is_slot_active(&s_ms_scheduler.scheduler, (SCHEDULER_SLOT_st*)slot);
}

//_____________________________________________________________________________
uint32_t MS_SCHEDULER_slot_ms_left(MS_SCHEDULER_SLOT_t* slot)
{
    return SCHEDULER_slot_ticks_left(&s_ms_scheduler.scheduler, (SCHEDULER_SLOT_st*)slot);
}

//_____________________________________________________________________________
uint32_t MS_SCHEDULER_get_tick(void)
{
	return SCHEDULER_get_tick(&s_ms_scheduler.scheduler);
}
