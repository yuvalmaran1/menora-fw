/**************************************************************************//**
 * @file      ledstrip.c
 * @brief     Addressable LED strip implementation
 *
 * @author    ymaran
 * @date      25.7.2024
 *****************************************************************************/
/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "ledstrip.h"
#include "stm32l0xx.h"


/******************************************************************************
 * Constants
 *****************************************************************************/
#define BITS_PER_BYTE           (8)
#define COLORS_PER_LED          (3)       // red, green, blue
#define LEDSTRIP_GREEN_OFFSET   (0)
#define LEDSTRIP_RED_OFFSET     (1)
#define LEDSTRIP_BLUE_OFFSET    (2)       
#define LEDSTRIP_BUF_LEN        (LEDSTRIP_NUM_DEVICES * COLORS_PER_LED * BITS_PER_BYTE)
#define LEDSTRIP_REFRESH_RATE_HZ (100)
#define LEDSTRIP_REFRESH_TIME_MS (1000/LEDSTRIP_REFRESH_RATE_HZ)
#define LEDSTRIP_COUNTER_ROLLOVER (2*LEDSTRIP_REFRESH_RATE_HZ) // vectors are 2 sec long
#define LEDSTRIP_COUNTER_BEAT_LEN (LEDSTRIP_COUNTER_ROLLOVER/12)
/******************************************************************************
 * Macros
 *****************************************************************************/
/* list expansion for predefined color array */
#define LEDSTRIP_COLOR_VAL(name, red, green, blue) \
    {.r = red, .g = green, .b = blue},

/******************************************************************************
 * Data types
 *****************************************************************************/
#pragma pack(push,1)
/* led color */
typedef struct 
{
    uint32_t g[BITS_PER_BYTE];  // green
    uint32_t r[BITS_PER_BYTE];  // red
    uint32_t b[BITS_PER_BYTE];  // blue
} LEDSTRIP_LED_CODE_st;

#pragma pack(pop)

/* led color for predefined colors */
typedef struct 
{
    uint8_t r;  // red
    uint8_t g;  // green
    uint8_t b;  // blue
} LEDSTRIP_LED_COLOR_st;

typedef struct
{
    LEDSTRIP_LED_COLOR_st color;
    LEDSTRIP_BLINK_en pattern;
} LEDSTRIP_LED_CONFIG_st;

/* context struct */
typedef struct
{
    
    GPIO_TypeDef* gpio_led_data;
    uint16_t gpio_led_data_pin;
    GPIO_TypeDef* gpio_led_en;
    uint16_t gpio_led_en_pin;
    TIM_HandleTypeDef*  tim;
    uint32_t tim_ch;
    bool first_update;
    uint32_t last_update;
    uint32_t counter;
    LEDSTRIP_BRIGHTNESS_en brightness;
    LEDSTRIP_LED_CONFIG_st led_config[LEDSTRIP_NUM_DEVICES];
    uint32_t dummy[100];
    LEDSTRIP_LED_CODE_st led_vals[LEDSTRIP_NUM_DEVICES];
} LEDSTRIP_st;

/******************************************************************************
 * Static variables
 *****************************************************************************/
/* predefined colors */
static const LEDSTRIP_LED_COLOR_st s_predefined_color[] = {LEDSTRIP_COLOR_LIST(LEDSTRIP_COLOR_VAL)};

/* gaussian vector - 200 entries long */
static const uint8_t s_gaussian[] = {
3,3,3,4,4,4,5,5,6,6,7,7,8,8,9,10,11,12,12,13,14,15,17,18,19,20,22,23,25,26,28,
30,32,34,36,38,41,43,45,48,51,53,56,59,62,66,69,72,76,79,83,87,91,95,99,103,107,
111,116,120,125,129,134,138,143,148,152,157,161,166,171,175,180,184,189,193,198,
202,206,210,214,218,221,225,228,231,234,237,240,242,245,247,249,250,252,253,254,
255,255,255,255,255,255,255,254,253,252,250,249,247,245,242,240,237,234,231,228,
225,221,218,214,210,206,202,198,193,189,184,180,175,171,166,161,157,152,148,143,
138,134,129,125,120,116,111,107,103,99,95,91,87,83,79,76,72,69,66,62,59,56,53,
51,48,45,43,41,38,36,34,32,30,28,26,25,23,22,20,19,18,17,15,14,13,12,12,11,10,
9,8,8,7,7,6,6,5,5,4,4,4,3,3 };

/*!< context struct instance */
static LEDSTRIP_st s_ledstrip = {0};

/******************************************************************************
 * Static function prototypes
 *****************************************************************************/

/******************************************************************************
 * Static functions
 *****************************************************************************/
//_____________________________________________________________________________
void LEDSTRIP_timer_cb(TIM_HandleTypeDef *htim)
{
    (void)htim;

    CTOSAL_semaphore_give(s_ledstrip.sem);
}

//_____________________________________________________________________________
static void LEDSTRIP_encode_led(uint32_t led_num, uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t bit;

    for (bit=0; bit<BITS_PER_BYTE; bit++)
    {
        s_ledstrip.led_vals[led_num].r[BITS_PER_BYTE - 1 - bit] = ((red   >> bit) & 1) ? (LEDSTRIP_1H) : (LEDSTRIP_0H);
        s_ledstrip.led_vals[led_num].g[BITS_PER_BYTE - 1 - bit] = ((green >> bit) & 1) ? (LEDSTRIP_1H) : (LEDSTRIP_0H);
        s_ledstrip.led_vals[led_num].b[BITS_PER_BYTE - 1 - bit] = ((blue  >> bit) & 1) ? (LEDSTRIP_1H) : (LEDSTRIP_0H);
    }
}

//_____________________________________________________________________________
static uint8_t LEDSTRIP_get_intensity(LEDSTRIP_BLINK_en pattern, uint32_t counter)
{
    uint8_t intensity = 0;

    switch(pattern)
    {
        case LEDSTRIP_BLINK_0HZ:
            intensity = UINT8_MAX;
            break;

        case LEDSTRIP_BLINK_1HZ:
            intensity = (((counter*2) % LEDSTRIP_COUNTER_ROLLOVER) < (LEDSTRIP_COUNTER_ROLLOVER>>1)) ? (0) : (UINT8_MAX);
            break;

        case LEDSTRIP_BLINK_5HZ:
            intensity = (((counter*10) % LEDSTRIP_COUNTER_ROLLOVER) < (LEDSTRIP_COUNTER_ROLLOVER>>1)) ? (0) : (UINT8_MAX);
            break;

        case LEDSTRIP_BLINK_10HZ:
            intensity = (((counter*20) % LEDSTRIP_COUNTER_ROLLOVER) < (LEDSTRIP_COUNTER_ROLLOVER>>1)) ? (0) : (UINT8_MAX);
            break;
        
        case LEDSTRIP_BLINK_GAUSSIAN_0_5HZ:
            intensity = s_gaussian[(counter) % LEDSTRIP_COUNTER_ROLLOVER];
            break;

        case LEDSTRIP_BLINK_GAUSSIAN_1HZ:
            intensity = s_gaussian[(counter*2) % LEDSTRIP_COUNTER_ROLLOVER];
            break;

        case LEDSTRIP_BLINK_GAUSSIAN_5HZ:
            intensity = s_gaussian[(counter*10) % LEDSTRIP_COUNTER_ROLLOVER];
            break;

        case LEDSTRIP_BLINK_HEARTBEAT_0_5HZ:
        {
            uint32_t val = counter % LEDSTRIP_COUNTER_ROLLOVER;
            intensity = ((val <= LEDSTRIP_COUNTER_BEAT_LEN) || ((val >= 2*LEDSTRIP_COUNTER_BEAT_LEN) && (val <= 5*LEDSTRIP_COUNTER_BEAT_LEN))) ? (UINT8_MAX) : (0);
        }
        break;

        default:
            break;
    }

    return intensity;
}

//_____________________________________________________________________________
static void LEDSTRIP_encode_all(uint32_t counter)
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t intensity;

    LEDSTRIP_lock();

    for (uint32_t led=0; led<LEDSTRIP_NUM_DEVICES; led++)
    {
        intensity = LEDSTRIP_get_intensity(s_ledstrip.led_config[led].pattern, counter);
        red   = (((((uint16_t)s_ledstrip.led_config[led].color.r) * (uint16_t)intensity) >> 8) >> s_ledstrip.brightness);
        green = (((((uint16_t)s_ledstrip.led_config[led].color.g) * (uint16_t)intensity) >> 8) >> s_ledstrip.brightness);
        blue  = (((((uint16_t)s_ledstrip.led_config[led].color.b) * (uint16_t)intensity) >> 8) >> s_ledstrip.brightness);

        LEDSTRIP_encode_led(led, red, green, blue);
    }

    LEDSTRIP_unlock();
}

/******************************************************************************
 * Public functions
 *****************************************************************************/
//_____________________________________________________________________________
LEDSTRIP_STATUS_t LEDSTRIP_init(LEDSTRIP_INIT_CONFIG_st* p_init_config)
{
    LEDSTRIP_STATUS_t status = LEDSTRIP_STATUS_OK;
    CTOSAL_SEM_CONFIG_t sem_cfg = {
        .initial_count = 0,
        .max_count = 1,
    };

    /* initialize context struct */
    s_ledstrip.gpio_led_data = p_init_config->gpio_led_data;
    s_ledstrip.gpio_led_en = p_init_config->gpio_led_en;
    s_ledstrip.tim = p_init_config->tim;
    s_ledstrip.tim_ch = p_init_config->tim_ch;
    s_ledstrip.brightness = p_init_config->brightness;
    s_ledstrip.first_update = true;
    s_ledstrip.last_update = CTOSAL_time_get_ms();
    s_ledstrip.counter = 0;
        
    HAL_TIM_RegisterCallback(s_ledstrip.tim->handle, HAL_TIM_PWM_PULSE_FINISHED_CB_ID, LEDSTRIP_timer_cb);
    
    CTHAL_GPIO_write(s_ledstrip.gpio_led_data, CTHAL_GPIO_VAL_RESET);

    return status;
}

//_____________________________________________________________________________
LEDSTRIP_STATUS_t LEDSTRIP_set_brightness(LEDSTRIP_BRIGHTNESS_en brightness)
{
    LEDSTRIP_STATUS_t status = LEDSTRIP_STATUS_OK;

    if (brightness >= LEDSTRIP_BRIGHTNESS_LENGTH)
    {
        status = LEDSTRIP_STATUS_INVALID_ARG;
    }
    else
    {
        LEDSTRIP_lock();
        
        s_ledstrip.brightness = brightness;

        LEDSTRIP_unlock();
    }

    return status;
}

//_____________________________________________________________________________
LEDSTRIP_STATUS_t LEDSTRIP_set_led(uint32_t led_num, LEDSTRIP_COLOR_en color, LEDSTRIP_BLINK_en pattern)
{
    LEDSTRIP_STATUS_t status = LEDSTRIP_STATUS_OK;

    if ((led_num >= LEDSTRIP_NUM_DEVICES) || (color >= LEDSTRIP_COLOR_LEN) || (pattern >= LEDSTRIP_BLINK_LENGTH))
    {
        status = LEDSTRIP_STATUS_INVALID_ARG;
    }
    else
    {
        LEDSTRIP_lock();
        
        s_ledstrip.led_config[led_num].color.r = s_predefined_color[color].r;
        s_ledstrip.led_config[led_num].color.g = s_predefined_color[color].g;
        s_ledstrip.led_config[led_num].color.b = s_predefined_color[color].b;
        s_ledstrip.led_config[led_num].pattern = pattern;

        LEDSTRIP_unlock();
    }

    return status;
}

//_____________________________________________________________________________
void LEDSTRIP_process(void)
{
    uint32_t timetag;

    timetag = CTOSAL_time_get_ms();

    if (s_ledstrip.first_update)
    {
        s_ledstrip.last_update = timetag;
        s_ledstrip.first_update = false; 
    }

    if ((timetag - s_ledstrip.last_update) >= LEDSTRIP_REFRESH_TIME_MS)
    {
        s_ledstrip.last_update = timetag; 

        s_ledstrip.counter = (timetag / (LEDSTRIP_REFRESH_TIME_MS)) % LEDSTRIP_COUNTER_ROLLOVER;

        /* encode leds */
        LEDSTRIP_encode_all(s_ledstrip.counter);

        
        CTHAL_GPIO_write(s_ledstrip.gpio_led_en, CTHAL_GPIO_VAL_SET);

        /* initiate transmission */
        if (HAL_OK == HAL_TIM_PWM_Start_DMA(s_ledstrip.tim->handle, s_ledstrip.tim_ch, (uint32_t*)s_ledstrip.dummy, sizeof(uint32_t)*(100+LEDSTRIP_BUF_LEN)))
        {
            /* wait for completion */
            CTOSAL_semaphore_take(s_ledstrip.sem, CTOSAL_TIMEOUT_FOREVER);
            // CTOSAL_time_delay_ms(1);
             /* stop transmission */
            HAL_TIM_PWM_Stop_DMA(s_ledstrip.tim->handle, s_ledstrip.tim_ch);           
        }

        CTHAL_GPIO_write(s_ledstrip.gpio_led_en, CTHAL_GPIO_VAL_RESET);
    }
}

