#include <stdio.h>
#include <stdbool.h>
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

//cannot use ADC2 
#define ADCPIN1 ADC_CHANNEL_5 //GPIO 33
#define ADCPIN2 ADC_CHANNEL_4 //GPIO 32
#define ADCPIN3 ADC_CHANNEL_7 //GPIO 35
#define LED1 14
#define LED2 2
#define LED3 4
#define BUTTON1 5 //needs to be interrupt //scrambles or solves step 1 
#define GPIO_INPUT_PIN_SEL (1ULL << BUTTON1)
#define low 0
#define high 4095
static int leds[3] = {LED1, LED2, LED3};



//const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
//const static char http_html_txt[] = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\n";
/**const static char http_index_hml[] = R"=====(
<!DOCTYPE html>
<html>

<head>
    <meta charset=UTF-8 name="viewport" content="width = device-width initial-scale = 1.0">
    <title>Home Page</title>
</head>
<!-- need to set up script function to acknowledge every button press--> 
<body>
    <div class="header">
        <h1>Joseph Torres</h1>
<!-- Keypad Buttons -->       
    </div> <input class="btn" id="btn0" type="button" value="0" onclick="addDigit('0')"> 
        <input class="btn" id="btn1" type="button" value="1" onclick="addDigit('1')">
        <input class="btn" id="btn2" type="button" value="2" onclick="addDigit('2')">
        <input class="btn" id="btn3" type="button" value="3" onclick="addDigit('3')">
        <input class="btn" id="btn4" type="button" value="4" onclick="addDigit('4')">
        <input class="btn" id="btn5" type="button" value="5" onclick="addDigit('5')">
        <input class="btn" id="btn6" type="button" value="6" onclick="addDigit('6')">
        <input class="btn" id="btn7" type="button" value="7" onclick="addDigit('7')">
        <input class="btn" id="btn8" type="button" value="8" onclick="addDigit('8')">
        <input class="btn" id="btn9" type="button" value="9" onclick="addDigit('9')">
<!-- Send and Clear Buttons -->        
        <input class="btn" type="button" value="Clear" onclick="clearbox()">
        <input class="btn" type="button" value="Send" onclick="sendRequest()">        
        <input type="text" id="numBox" maxlength = "3" readonly> 
        
    <style>
        * {
            margin: 0;
            padding: 0;
        }

        body {
            background-color: #D4DCE2;
        }

        .header {
            width: 100%;
            height: 55px;
            color: white;
            background-color: #04254D;
            padding: 0;
            text-align: center;
        }

        .header h1 {
            color: white;
            vertical-align: center;
            font-size: 42px;
        }

        .btn {
            margin: 0;
            margin-top: .5%;
            background-color: #d041fb;
            width: 48%;
            border: none;
            color: white;
            padding: 25px 38px;
            text-align: center;
            text-decoration: none;
            font-size: 16px;
        }

        .JOSEPH {
            margin: 0;
            margin-top: .5%;
            width: 100%;
            height: 70px;
            color: white;
            background-color: #e5aaec;
            padding: 0;
            text-align: center;
        }

        .sensorVal p {
            color: white;
            vertical-align: center;
            font-size: 38px;
        }
    </style>
    <script> 
            var MAX_DIGITS = 3;

            function addDigit(d) {
                var textbox = document.getElementById("numBox");
                if (textbox.value.length >= MAX_DIGITS) { return; }
                textbox.value = textbox.value + d;
            }

            function clearbox() {
                document.getElementById("numBox").value = "";
                  
            }
            
            function sendRequest() { 
                var boxvalue = document.getElementById("numBox").value;
                if (boxvalue === "") { return; }

                var http = new XMLHttpRequest(); 
                http.onreadystatechange = (() => { 
                    if (http.readyState === 4) { 
                        if (http.status === 200) { 
                            var res = http.responseText; 
                            } 
                        } 
                    });
                    http.open("GET", "/send?value=" + boxvalue, true); 
http.send(); 
            }
    </script>
</body>

</html>
)=====";
**/

adc_oneshot_unit_handle_t puzzletask1;


void periphinit (void) {
    //Input Set Up 
    rom_gpio_pad_select_gpio(BUTTON1);
    gpio_set_direction(BUTTON1, GPIO_MODE_INPUT);
    gpio_pullup_en(BUTTON1); 
    
    
    
    //LED Set Up

    for (int i = 0; i < 3; i++) {
        rom_gpio_pad_select_gpio(leds[i]); 
        gpio_set_direction(outputs[i], GPIO_MODE_OUTPUT);
        gpio_set_level(outputs[i], 0);
    }
   
    //using ADC1 Channels 
    adc_oneshot_unit_init_cfg_t pot_handle = {.unit_id = ADC_UNIT_1}; 
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


//set up the solving blinkrate -> we need one main one to be offsetted by values to guarantee solvability
void scrambleLEDs (int value) {
int value1 = 1500 + esp_random() % 2001; //generates random number between 1500 - 3500 
return value1; 

}

void app_main (void) 
{
periphinit();

/**TEST ADC
int cap_pot1, cap_pot2, cap_pot3;
while (1) {
adc_oneshot_read(puzzletask1, ADCPIN1, &cap_pot1);
adc_oneshot_read(puzzletask1, ADCPIN2, &cap_pot2);
adc_oneshot_read(puzzletask1, ADCPIN3, &cap_pot3);
printf("%d\n", cap_pot1);
printf("%d\n", cap_pot2);
printf("%d\n", cap_pot3);
vTaskDelay(pdMS_TO_TICKS(100));
} **/

}