/**************************************************************************//**
 * @file      assert.h
 * @brief     Assertion macro
 *
 * @author    ymaran
 * @date      Jun 7, 2026
 * @copyright 2023  Ltd
 *****************************************************************************/

#ifndef UTIL_ASSERT_H_
#define UTIL_ASSERT_H_

//***************************************************************************//
// Dependencies                                                              //
//***************************************************************************//
#include "rtt_log.h"

//***************************************************************************//
// Constants & Macros                                                        //
//***************************************************************************//
/******************************************************************************
* @brief   Assert a condition. On failure, logs the failure over RTT and
*          halts the system in an infinite loop
*
* @param   cond  condition expected to be true
* @param   msg   message describing the assertion
*****************************************************************************/
#define ASSERT(cond, msg)                                                   \
    do                                                                      \
    {                                                                       \
        if (!(cond))                                                        \
        {                                                                   \
            RTT_LOG_log(RTT_CRITICAL, "ASSERT", "%s:%d - %s", __FILE__, __LINE__, msg); \
            while (1)                                                       \
            {                                                               \
            }                                                               \
        }                                                                   \
    } while (0)

#endif /* UTIL_ASSERT_H_ */
