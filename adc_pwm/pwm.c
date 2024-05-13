// codigo cria tasks de leds no pwm que voce indicar
// led red opera num duty cycle de 80%
// led green opera num duty cycle de 20%

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/pwm.h"

const int PWM_0_PIN = 4;
const int PWM_1_PIN = 6;

void led_task(void *p) {
    gpio_set_function(PWM_0_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PWM_0_PIN);
    pwm_set_clkdiv(slice_num, 125);
    pwm_set_wrap(slice_num, 100);

    pwm_set_chan_level(slice_num, PWM_CHAN_A, 80); // defino aqui o duty cycle que vou operar, no caso é 80%
    pwm_set_enabled(slice_num, true);

    while (true) {
    }
}

void led_task_g(void *p) {
    gpio_set_function(PWM_1_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PWM_1_PIN);
    pwm_set_clkdiv(slice_num, 125);
    pwm_set_wrap(slice_num, 100);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 20); // defino aqui o duty cycle que vou operar, no caso é 20%
    pwm_set_enabled(slice_num, true);

    while (true) {
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    xTaskCreate(led_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_task_g, "LED_Task 2", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
