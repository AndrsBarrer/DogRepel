#include "driver/ledc.h"

#define LEDC_HS_TIMER LEDC_TIMER_0
#define LEDC_HS_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO (18)
#define LEDC_HS_CH0_CHANNEL LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO (19)
#define LEDC_HS_CH1_CHANNEL LEDC_CHANNEL_1
#define LEDC_LS_TIMER LEDC_TIMER_1
#define LEDC_LS_MODE LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH2_GPIO (4)
#define LEDC_LS_CH2_CHANNEL LEDC_CHANNEL_2
#define LEDC_LS_CH3_GPIO (5)
#define LEDC_LS_CH3_CHANNEL LEDC_CHANNEL_3

#define LEDC_TEST_CH_NUM (4)
#define LEDC_TEST_DUTY (4000)
#define LEDC_TEST_FADE_TIME (3000)


ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_1_BIT, // 1-bit resolution (on/off only)
    .freq_hz = 15000,                     // 15kHz frequency for the transducer
    .speed_mode = LEDC_HIGH_SPEED_MODE,  // High-speed mode
    .timer_num = LEDC_HS_TIMER,          // High-speed timer
    .clk_cfg = LEDC_AUTO_CLK             // Auto-select the source clock
};

// // Prepare individual configuration for each channel of LED Controller
// ledc_channel_config_t ledc_channel = {
//     .channel = LEDC_HS_CH1_CHANNEL,
//     .duty = 0,                          // Duty cycle set to 1 for 1-bit resolution (on)
//     .gpio_num = LEDC_HS_CH1_GPIO,       // GPIO pin connected to the transducer
//     .speed_mode = LEDC_HIGH_SPEED_MODE, // High-speed mode
//     .hpoint = 0,                        // High point set to 0
//     .timer_sel = LEDC_HS_TIMER          // Using high-speed timer
// };

// Channel configuration
ledc_channel_config_t ledc_channel = {
    .channel = LEDC_HS_CH1_CHANNEL,
    .duty = 0,                          // Initial duty set to 0 (off)
    .gpio_num = LEDC_HS_CH1_GPIO,       // GPIO pin connected to the transducer
    .speed_mode = LEDC_HS_MODE,         // High-speed mode
    .hpoint = 0,                        // High point set to 0
    .timer_sel = LEDC_HS_TIMER          // Using high-speed timer
};