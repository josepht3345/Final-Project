#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "rom/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"

#define SERVOPIN 15
#define S_LOCKED (3277 * (1.5))
#define S_UNLOCKED (3277 * (.6))
#define ADCPIN1 
#define ACPCIN2
#define ADCPIN3

void periphinit (void) {
    //adc_oneshot_unit_init_cfg_t pot1; 


    //LEDC -> PWM FOR SERVO MOTOR 
    //3 things required for a pwm channel -> Timer -> Channel -> Update Duty Cycle 
    //Timer
    ledc_timer_config_t servo_lock = {.duty_resolution = LEDC_TIMER_16_BIT, .timer_num = 0, .freq_hz = 50, .speed_mode = LEDC_HIGH_SPEED_MODE, .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&servo_lock);
    //LEDC Channel
    //for duty cycle remember  -> 1/50hz = 20ms , 16 bit resolution = 2^16 = 65536 -> 0-65535, 1ms = 5% = ~3277 -> 1ms duty cycle should point the servo to the left 
    ledc_channel_config_t servo_conf = {.gpio_num = SERVOPIN, .speed_mode = LEDC_HIGH_SPEED_MODE, .channel = 0, .duty = 3277*(.6), .hpoint = 0 }; 
    // *.60 percent -> unlocked, * 1.5 percent -> locked 
    ledc_channel_config(&servo_conf);
}

void app_main (void) 
{
periphinit();

}