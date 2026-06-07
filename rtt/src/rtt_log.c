/**************************************************************************//**
 * @file      eeprom24.c
 * @brief     I2C EEPROM series 24 implementation
 *
 * @author    ymaran
 * @date      3.4.2023
 * @copyright 2023  Ltd
 *****************************************************************************/
/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "rtt_log.h"
#include "SEGGER_RTT.h"
#include "stm32l0xx_hal.h"

/******************************************************************************
 * Constants
 *****************************************************************************/
/* log text colors */
#define RTT_LOG_COLOR_BLACK           "\033[2;30m"
#define RTT_LOG_COLOR_RED             "\033[2;31m"
#define RTT_LOG_COLOR_GREEN           "\033[2;32m"
#define RTT_LOG_COLOR_YELLOW          "\033[2;33m"
#define RTT_LOG_COLOR_BLUE            "\033[2;34m"
#define RTT_LOG_COLOR_MAGENTA         "\033[2;35m"
#define RTT_LOG_COLOR_CYAN            "\033[2;36m"
#define RTT_LOG_COLOR_WHITE           "\033[2;37m"
#define RTT_LOG_COLOR_BRIGHT_BLACK    "\033[1;30m"
#define RTT_LOG_COLOR_BRIGHT_RED      "\033[1;31m"
#define RTT_LOG_COLOR_BRIGHT_GREEN    "\033[1;32m"
#define RTT_LOG_COLOR_BRIGHT_YELLOW   "\033[1;33m"
#define RTT_LOG_COLOR_BRIGHT_BLUE     "\033[1;34m"
#define RTT_LOG_COLOR_BRIGHT_MAGENTA  "\033[1;35m"
#define RTT_LOG_COLOR_BRIGHT_CYAN     "\033[1;36m"
#define RTT_LOG_COLOR_BRIGHT_WHITE    "\033[1;37m"

/* log background colors */
#define RTT_LOG_BG_NONE               "\033[0m"
#define RTT_LOG_BG_BLACK              "\033[40m"
#define RTT_LOG_BG_RED                "\033[41m"
#define RTT_LOG_BG_GREEN              "\033[42m"
#define RTT_LOG_BG_YELLOW             "\033[43m"
#define RTT_LOG_BG_BLUE               "\033[44m"
#define RTT_LOG_BG_MAGENTA            "\033[45m"
#define RTT_LOG_BG_CYAN               "\033[46m"
#define RTT_LOG_BG_WHITE              "\033[47m"
#define RTT_LOG_BG_BRIGHT_BLACK       "\033[40,1m"
#define RTT_LOG_BG_BRIGHT_RED         "\033[41,1m"
#define RTT_LOG_BG_BRIGHT_GREEN       "\033[42,1m"
#define RTT_LOG_BG_BRIGHT_YELLOW      "\033[43,1m"
#define RTT_LOG_BG_BRIGHT_BLUE        "\033[44,1m"
#define RTT_LOG_BG_BRIGHT_MAGENTA     "\033[45,1m"
#define RTT_LOG_BG_BRIGHT_CYAN        "\033[46,1m"
#define RTT_LOG_BG_BRIGHT_WHITE       "\033[47,1m"

#define RTT_LOG_COLOR_STR_MAX_LEN     (20)
#define RTT_LOG_CTRL_RESET            "\x1B[0m"         // Reset to default colors
#define RTT_LOG_CTRL_CLEAR            "\x1B[2J"         // Clear screen, reposition cursor to top left
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
static void RTT_LOG_log_va(RTT_LOG_LEVEL_en level, const char* source, const char* msg, va_list *param_list)
{
    char color_str[RTT_LOG_COLOR_STR_MAX_LEN] = {0};
    uint32_t timetag = HAL_GetTick();
    uint32_t msec, sec, minute, hour;
    
    /* get color string */
    switch(level)
    {
        case RTT_CRITICAL:
            strcpy(color_str, RTT_LOG_BG_BRIGHT_YELLOW RTT_LOG_COLOR_BRIGHT_RED);
            break;
        case RTT_ERROR:
            strcpy(color_str, RTT_LOG_COLOR_BRIGHT_RED);
            break;
        case RTT_WARNING:
            strcpy(color_str, RTT_LOG_COLOR_BRIGHT_YELLOW);
            break;
        case RTT_INFO:
            strcpy(color_str, RTT_LOG_COLOR_BRIGHT_CYAN);
            break;
        case RTT_DEBUG:
            strcpy(color_str, RTT_LOG_COLOR_BRIGHT_MAGENTA);
            break;
        case RTT_TRACE:
            strcpy(color_str, RTT_LOG_COLOR_BRIGHT_WHITE);
            break;
        default:
            strcpy(color_str, RTT_LOG_COLOR_WHITE);
            break;
    }

    /* calculate time */
    msec = timetag % 1000;
    timetag /= 1000;
    sec = timetag % 60;
    timetag /= 60;
    minute = timetag % 60;
    timetag /= 60;
    hour = timetag;

    SEGGER_RTT_printf(0U, RTT_LOG_CTRL_RESET "%s@ %d:%02d:%02d.%03d - %s - ", color_str, hour, minute, sec, msec, source);

    SEGGER_RTT_vprintf(0U, msg, param_list);

    SEGGER_RTT_PutChar(0U, '\n');
}

/******************************************************************************
 * Public functions
 *****************************************************************************/
//_____________________________________________________________________________
void RTT_LOG_init(void)
{
    SEGGER_RTT_Init();
}

//_____________________________________________________________________________
void RTT_LOG_log(RTT_LOG_LEVEL_en level, const char* source, const char* msg, ...)
{
    va_list param_list;

    va_start(param_list, msg);
    RTT_LOG_log_va(level, source, msg, &param_list);
    va_end(param_list);
}