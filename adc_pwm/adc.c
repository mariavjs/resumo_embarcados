// lab 1

// configuracao de leitura de 2 pinos adc
// ATENCAO PARA O ID, VER NO ESQUEMA DO LADO!!!
// Codigo cria duas tasks 

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/adc.h"

void adc_1_task(void *p) {
    adc_init();
    adc_gpio_init(27);

    // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
    const float conversion_factor = 3.3f / (1 << 12);

    uint16_t result;
    while (1) {
        adc_select_input(1); // Select ADC input 1 (GPIO27)
        result = adc_read();
        printf("voltage 1: %f V\n", result * conversion_factor);

        // CÓDIGO AQUI



        vTaskDelay(pdMS_TO_TICKS(200));
    }
}


void adc_0_task(void *p) {
    adc_init();
    adc_gpio_init(26);

    // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
    const float conversion_factor = 3.3f / (1 << 12);

    uint16_t result;
    while (1) {
        adc_select_input(0); // Select ADC input 0 (GPIO26)
        result = adc_read();
        printf("voltage 2: %f V\n", result * conversion_factor);

        // CÓDIGO AQUI

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS \n");
    adc_init();

    xTaskCreate(adc_1_task, "LED_Task 1", 4095, NULL, 1, NULL);
    xTaskCreate(adc_0_task, "LED_Task 2", 4095, NULL, 1, NULL);
    vTaskStartScheduler();

    while (true) {
    }
}



