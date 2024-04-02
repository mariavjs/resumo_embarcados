/*
 * LED blink with FreeRTOS
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "ssd1306.h"
#include "gfx.h"

#include "pico/stdlib.h"
#include <stdio.h>

//OLED
const uint BTN_1_OLED = 28;
const uint BTN_2_OLED = 26;
const uint BTN_3_OLED = 27;

const uint LED_1_OLED = 20;
const uint LED_2_OLED = 21;
const uint LED_3_OLED = 22;

//DISTANCIA E TIMER
const int TRIG_PIN = 16;
const int ECHO_PIN = 17;

int SOUND_SPEED = 340;

// CRIANDO FILAS E SEMAFOROS
QueueHandle_t xQueue_time;
SemaphoreHandle_t xSemaphore_trigger;
QueueHandle_t xQueue_distance;


//OLED
void oled1_btn_led_init(void) {
    gpio_init(LED_1_OLED);
    gpio_set_dir(LED_1_OLED, GPIO_OUT);

    gpio_init(LED_2_OLED);
    gpio_set_dir(LED_2_OLED, GPIO_OUT);

    gpio_init(LED_3_OLED);
    gpio_set_dir(LED_3_OLED, GPIO_OUT);

    gpio_init(BTN_1_OLED);
    gpio_set_dir(BTN_1_OLED, GPIO_IN);
    gpio_pull_up(BTN_1_OLED);

    gpio_init(BTN_2_OLED);
    gpio_set_dir(BTN_2_OLED, GPIO_IN);
    gpio_pull_up(BTN_2_OLED);

    gpio_init(BTN_3_OLED);
    gpio_set_dir(BTN_3_OLED, GPIO_IN);
    gpio_pull_up(BTN_3_OLED);
}

void oled_task(void *p) {
    printf("Inicializando Driver\n");
    ssd1306_init();
    printf("Inicializando GLX\n");
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);
    printf("Inicializando btn and LEDs\n");
    oled1_btn_led_init();
    
    while (1) {
        float distance;
        
        if ((xSemaphoreTake(xSemaphore_trigger, 0) == pdTRUE)) {
            if (xQueueReceive(xQueue_distance, &distance, 0)) {
                char distance_str[20];
                gfx_clear_buffer(&disp);
                snprintf(distance_str, sizeof(distance_str), "Distancia: %.2f cm", distance); // ajusta o tamanho da string para ficar dentro dos limites
                gfx_draw_string(&disp, 0, 0, 1, distance_str);

                // Defina a largura máxima e mínima da barra
                float MAX_DISTANCE = 400.0;
                float MIN_DISTANCE = 2.0; 
                int MAX_BAR_WIDTH = 100; // Largura máxima da barra
                int MIN_BAR_WIDTH = 10;  // Largura mínima da barra

                // Limitando a distância dentro do intervalo válido
                if (distance < MIN_DISTANCE)
                    distance = MIN_DISTANCE;
                else if (distance > MAX_DISTANCE)
                    distance = MAX_DISTANCE;

                // Calcule a largura da barra com base na distância medida
                int bar_width; 
                // Calculando a largura da barra proporcional à distância
                bar_width = (int)(((distance - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE)) * MAX_BAR_WIDTH);

                // Desenha a barra na tela
                gfx_draw_line(&disp, 15, 27, 15 + bar_width, 27);

                // gfx_draw_line(&disp, 15, 27, distance, 27);
                gfx_show(&disp);
                vTaskDelay(pdMS_TO_TICKS(50));
            } 
             else {
                gfx_clear_buffer(&disp);
                gfx_draw_string(&disp, 0, 0, 1, "ERRO");
                gfx_show(&disp);
            }
           
        }
    }
}

//DISTANCIA E TIMER
void pin_callback(uint gpio, uint32_t events) {
    static uint32_t start_time, end_time, duration;
    if (events == 0x8) {
        if (gpio == ECHO_PIN) {
           start_time = to_us_since_boot(get_absolute_time());
        }

    } else if (events == 0x4) {
        if (gpio == ECHO_PIN) {
            end_time = to_us_since_boot(get_absolute_time());
            duration = end_time - start_time;
            xQueueSendFromISR(xQueue_time, &duration, 0);

        }
    }
}

void send_pulse(){
    gpio_put(TRIG_PIN, 1);
    sleep_us(10);
    gpio_put(TRIG_PIN, 0);
}

void trigger_task() {
    
    printf("Inicializando Driver\n");
    ssd1306_init();

    printf("Inicializando GLX\n");
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);

    printf("Inicializando btn and LEDs\n");
    oled1_btn_led_init();

    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_put(TRIG_PIN, 0);

    while (true) {
        send_pulse();
        xSemaphoreGive(xSemaphore_trigger);
    }
    
}

void echo_sensor() {
    uint32_t duration;

    printf("Inicializando Driver\n");
    ssd1306_init();

    printf("Inicializando GLX\n");
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);

    printf("Inicializando btn and LEDs\n");
    oled1_btn_led_init();

    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
    gpio_pull_up(ECHO_PIN);

    while (true) {
        if (xQueueReceive(xQueue_time, &duration, pdMS_TO_TICKS(50))) {
            float distance = duration / 58.0;
            xQueueSend(xQueue_distance, &distance, 0);
            printf("Distancia: %.2f cm\n", distance);
        }
    
    }

}

int main() {

    stdio_init_all();

    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &pin_callback);


    xSemaphore_trigger = xSemaphoreCreateBinary();
    xQueue_time = xQueueCreate(32, sizeof(uint64_t)); // Ajuste conforme necessário
    xQueue_distance = xQueueCreate(32, sizeof(float)); // Ajuste conforme necessário

    

    //TASKS E SCHEDULE
    //task trigger:
    xTaskCreate(trigger_task, "Trigger Sensor", 4095, NULL, 1, NULL);

    //task echo:
    xTaskCreate(echo_sensor, "Echo Sensor", 4095, NULL, 1, NULL);

    //task oled:
    xTaskCreate(oled_task, "OLED", 4095, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true) {
        ;

    }
}