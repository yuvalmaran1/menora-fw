/**************************************************************************//**
 * @file      ledstrip.h
 * @brief     Addressable LED strip interface
 *
 * @author    ymaran
 * @date      25.7.2024
 *****************************************************************************/
#ifndef LEDSTRIP_H_
#define LEDSTRIP_H_

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx_hal.h"

/******************************************************************************
 * Constants
 *****************************************************************************/
/*!< default definitions */
#if !defined(LEDSTRIP_NUM_DEVICES)
    #define LEDSTRIP_NUM_DEVICES (9)     //!< number of leds in strip
#endif

#if !defined(LEDSTRIP_0H) || !defined(LEDSTRIP_1H)
    #define LEDSTRIP_0H             (11)       // number of timer high counts for logic 0 
    #define LEDSTRIP_1H             (22)       // number of timer high counts for logic 1
#endif

#define LEDSTRIP_COLOR_LIST(COLOR) \
COLOR(OFF,     0,   0,   0  ) \
COLOR(RED,     255, 0,   0  ) \
COLOR(YELLOW,  255, 255, 0  ) \
COLOR(ORANGE,  255, 128, 0  ) \
COLOR(GREEN,   0,   255, 0  ) \
COLOR(BLUE,    0,   0,   255) \
COLOR(CYAN,    0,   255, 255) \
COLOR(WHITE,   255, 255, 255) \
COLOR(GREY,    128, 128, 128) \

/* list expansion for predefined color enumeration */
#define LEDSTRIP_COLOR_ENUM(name, red, green, blue) LEDSTRIP_COLOR_##name,
/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Status
 *****************************************************************************/
/*! @typedef LEDSTRIP_STATUS_t
 * return status. 0 success. <0 error.
 */
typedef int32_t LEDSTRIP_STATUS_t;

#define LEDSTRIP_STATUS_OK                         ((LEDSTRIP_STATUS_t)( 0)) //!< no error
#define LEDSTRIP_STATUS_INVALID_ARG                ((LEDSTRIP_STATUS_t)(-1)) //!< invalid argument
#define LEDSTRIP_STATUS_TIMEOUT                    ((LEDSTRIP_STATUS_t)(-2)) //!< operation timed out

/******************************************************************************
 * Data types
 *****************************************************************************/
/* led brightness enumeration */
typedef enum
{
    LEDSTRIP_BRIGHTNESS_SUPER_BRIGHT    = 0,
    LEDSTRIP_BRIGHTNESS_BRIGHT          = 1,
    LEDSTRIP_BRIGHTNESS_MEDIUM          = 2,
    LEDSTRIP_BRIGHTNESS_DARK            = 3,
    LEDSTRIP_BRIGHTNESS_LENGTH 
} LEDSTRIP_BRIGHTNESS_en;

/*!< initial configuration struct */
typedef struct
{
    GPIO_TypeDef* gpio_led_data;
    uint16_t gpio_led_data_pin;
    GPIO_TypeDef* gpio_led_en;
    uint16_t gpio_led_en_pin;
    TIM_HandleTypeDef*  tim;
    uint32_t tim_ch;
    LEDSTRIP_BRIGHTNESS_en brightness;
} LEDSTRIP_INIT_CONFIG_st;

/*!< predefined color enumeration */
typedef enum
{
    LEDSTRIP_COLOR_LIST(LEDSTRIP_COLOR_ENUM)
    LEDSTRIP_COLOR_LEN   
} LEDSTRIP_COLOR_en;

/*!< blink pattern enumeration */
typedef enum
{
    LEDSTRIP_BLINK_0HZ,
    LEDSTRIP_BLINK_1HZ,
    LEDSTRIP_BLINK_5HZ,
    LEDSTRIP_BLINK_10HZ,
    LEDSTRIP_BLINK_GAUSSIAN_0_5HZ,
    LEDSTRIP_BLINK_GAUSSIAN_1HZ,
    LEDSTRIP_BLINK_GAUSSIAN_5HZ,
    LEDSTRIP_BLINK_HEARTBEAT_0_5HZ,
    LEDSTRIP_BLINK_LENGTH
} LEDSTRIP_BLINK_en;

/******************************************************************************
 * Public function prototypes
 *****************************************************************************/
/***************************************************************************//**
 * Initialize device
 *
 * @param[in] p_init_config initial configuration
 *
 * @return procedure result
 ******************************************************************************/
LEDSTRIP_STATUS_t LEDSTRIP_init(LEDSTRIP_INIT_CONFIG_st* p_init_config);

/***************************************************************************//**
 * Set predefined LED color setting, along with blink pattern
 * 
 * @brief This function does not update the LED, only the internal value.
 *        To update the strip - call LEDSTRIP_update()
 *
 * @param[in] led_num led number in strip in range [0, LEDSTRIP_NUM_DEVICES-1]
 * @param[in] color color from @ref LEDSTRIP_COLOR_en
 * @param[in] pattern blink pattern from @ref LEDSTRIP_BLINK_en
 *
 * @return procedure result
 ******************************************************************************/
LEDSTRIP_STATUS_t LEDSTRIP_set_led(uint32_t led_num, LEDSTRIP_COLOR_en color, LEDSTRIP_BLINK_en pattern);

/***************************************************************************//**
 * Process LED strip internals
 * 
 * @return none
 ******************************************************************************/
void LEDSTRIP_process(void);

#endif // LEDSTRIP_H_