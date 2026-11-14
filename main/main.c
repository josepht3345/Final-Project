#include <stdio.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "rom/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define SERVOPIN 15
#define S_LOCKED (3277 * (1.5))
#define S_UNLOCKED (3277 * (.6))
#define ADCPIN1 ADC_CHANNEL_8
#define ADCPIN2 ADC_CHANNEL_9
#define ADCPIN3 ADC_CHANNEL_7
#define LED1 14
#define LED2 2
#define LED3 4


adc_oneshot_unit_handle_t puzzletask1;


void periphinit (void) {
    //LED Set Up
    int outputs[3] = {LED1, LED2, LED3};
    for (int i = 0; i < 3; i++) {
        rom_gpio_pad_select_gpio(outputs[i]); 
        gpio_set_direction(outputs[i], GPIO_MODE_OUTPUT);
        gpio_set_level(outputs[i], 0);
    }
   
    //using ADC2 Channels 
    adc_oneshot_unit_init_cfg_t pot_handle = {.unit_id = ADC_UNIT_2}; 
    adc_oneshot_new_unit(&pot_handle, &puzzletask1);
    //Channel Set Up 
    adc_oneshot_chan_cfg_t pot_channel = {.bitwidth = ADC_BITWIDTH_12, .atten = ADC_ATTEN_DB_12};
    adc_oneshot_config_channel(puzzletask1, ADCPIN1, &pot_channel);
    adc_oneshot_config_channel(puzzletask1, ADCPIN2, &pot_channel);
    adc_oneshot_config_channel(puzzletask1, ADCPIN3, &pot_channel);


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
int cap_pot1, cap_pot2, cap_pot3;
while (1) {


adc_oneshot_read(puzzletask1, ADCPIN1, &cap_pot1);
adc_oneshot_read(puzzletask1, ADCPIN2, &cap_pot2);
adc_oneshot_read(puzzletask1, ADCPIN3, &cap_pot3);

printf("%d\n", cap_pot1);
printf("%d\n", cap_pot2);
printf("%d\n", cap_pot3);
vTaskDelay(pdMS_TO_TICKS(100));



}
}