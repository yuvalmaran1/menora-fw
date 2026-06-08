/******************************************************************************
 * @file      menora.h
 * @brief     Application header
 *
 * @author    ymaran
 * @date      25.7.2024
 *****************************************************************************/
#ifndef MENORA_H_
#define MENORA_H_

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "stm32l0xx_hal.h"

/******************************************************************************
 * Constants
 *****************************************************************************/

/******************************************************************************
 * Data types
 *****************************************************************************/
typedef struct
{
    TIM_HandleTypeDef*  scheduler_tim;
    TIM_HandleTypeDef*  led_tim;
    TIM_HandleTypeDef*  buzzer_tim;

    GPIO_TypeDef* led_data_port;
    uint16_t led_data_pin;
    uint32_t led_tim_ch;

    GPIO_TypeDef* led_en_port;
    uint16_t led_en_pin;

    GPIO_TypeDef* id0_port;
    uint16_t id0_pin;

    GPIO_TypeDef* id1_port;
    uint16_t id1_pin;

    GPIO_TypeDef* id2_port;
    uint16_t id2_pin;

    GPIO_TypeDef* light_btn_port;
    uint16_t light_btn_pin;

    GPIO_TypeDef* music_btn_port;
    uint16_t music_btn_pin;

    GPIO_TypeDef* mode_btn_port;
    uint16_t mode_btn_pin;

    TSC_HandleTypeDef* tsc;

    uint32_t tsc_light_btn_channel_io;
    uint32_t tsc_music_btn_channel_io;
    uint32_t tsc_mode_btn_channel_io;
    uint32_t tsc_btn_group_index;
    uint32_t tsc_btn_sampling_io;
} MENORA_INIT_CONFIG_st;

/******************************************************************************
 * Public function prototypes
 *****************************************************************************/
/******************************************************************************
 * Initialize application
 *
 * @param[in] p_init_config initial configuration
 *
 * @return procedure result
 ******************************************************************************/
void MENORA_init(MENORA_INIT_CONFIG_st* p_init_config);

/******************************************************************************
 * Process application internals
 * @note This function should be called in an infinite loop
 ******************************************************************************/
 void MENORA_process(void);

#endif // MENORA_H_