/**************************************************************************//**
 * @file      osal.h
 * @brief     Minimal OS abstraction - critical section helpers
 *
 * @author    ymaran
 * @date      Jun 7, 2026
 * @copyright 2023  Ltd
 *****************************************************************************/

#ifndef UTIL_OSAL_H_
#define UTIL_OSAL_H_

//***************************************************************************//
// Dependencies                                                              //
//***************************************************************************//

//***************************************************************************//
// Public Function Prototypes                                                //
//***************************************************************************//
/******************************************************************************
* @brief   Enter critical section (disables interrupts)
*
* @note    calls may be nested - interrupts are re-enabled once the
*          outermost critical section is exited
*****************************************************************************/
void OSAL_critical_section_enter(void);

/******************************************************************************
* @brief   Exit critical section (re-enables interrupts when not nested)
*****************************************************************************/
void OSAL_critical_section_exit(void);

#endif /* UTIL_OSAL_H_ */
