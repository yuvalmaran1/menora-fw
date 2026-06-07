/**************************************************************************//**
 * @file      osal.c
 * @brief     Minimal OS abstraction implementation - critical section helpers
 *
 * @author    ymaran
 * @date      Jun 7, 2026
 * @copyright 2023  Ltd
 *****************************************************************************/
//***************************************************************************//
// Dependencies                                                              //
//***************************************************************************//
#include "osal.h"
#include "stm32l0xx_hal.h"

//***************************************************************************//
// Static Variables                                                          //
//***************************************************************************//
static volatile uint32_t s_critical_section_nesting = 0;

//***************************************************************************//
// Public Functions                                                          //
//***************************************************************************//
//_____________________________________________________________________________
void OSAL_critical_section_enter(void)
{
    __disable_irq();

    s_critical_section_nesting++;
}

//_____________________________________________________________________________
void OSAL_critical_section_exit(void)
{
    if (s_critical_section_nesting > 0)
    {
        s_critical_section_nesting--;

        if (0 == s_critical_section_nesting)
        {
            __enable_irq();
        }
    }
}
