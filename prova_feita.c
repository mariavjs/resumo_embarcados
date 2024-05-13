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

#include <string.h>

const uint BTN_1_OLED = 28;
const uint BTN_2_OLED = 26;
const uint BTN_3_OLED = 27;

const uint LED_1_OLED = 20;
const uint LED_2_OLED = 21;
const uint LED_3_OLED = 22;

const int PINO_1_MOTOR = 7;
const int PINO_2_MOTOR = 8;
const int PINO_3_MOTOR = 12;
const int PINO_4_MOTOR = 13;

const char senha[4] = "1323";
const char open[5] = "OPEN";
const char error[5] = "ERROR";

SemaphoreHandle_t xSemaphoreMotor = NULL;
QueueHandle_t xQueueBTN;

void btn_callback(uint gpio, uint32_t events) {
    if (events == GPIO_IRQ_EDGE_FALL) {
        uint botao;
        if (gpio_get(BTN_1_OLED) == 0) {
            botao = 1;
            xQueueSendFromISR(xQueueBTN, &botao, NULL);
        } else if (gpio_get(BTN_2_OLED) == 0) {
            botao = 2;
            xQueueSendFromISR(xQueueBTN, &botao, NULL);
        } else if (gpio_get(BTN_3_OLED) == 0) {
            botao = 3;
            xQueueSendFromISR(xQueueBTN, &botao, NULL);
        }
    }
}

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
    gpio_set_irq_enabled_with_callback(BTN_1_OLED, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);

    gpio_init(BTN_2_OLED);
    gpio_set_dir(BTN_2_OLED, GPIO_IN);
    gpio_pull_up(BTN_2_OLED);
    gpio_set_irq_enabled_with_callback(BTN_2_OLED, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);

    gpio_init(BTN_3_OLED);
    gpio_set_dir(BTN_3_OLED, GPIO_IN);
    gpio_pull_up(BTN_3_OLED);
    gpio_set_irq_enabled_with_callback(BTN_3_OLED, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);
}

void motor_task(void *p) {
    if (xSemaphoreTake(xSemaphoreMotor, portMAX_DELAY) == pdTRUE) {
        for (int i = 0; i < 512; i++) {
                gpio_put(PINO_1_MOTOR, 1);
                vTaskDelay(pdMS_TO_TICKS(10));
                gpio_put(PINO_1_MOTOR, 0);
                gpio_put(PINO_2_MOTOR, 1);
                vTaskDelay(pdMS_TO_TICKS(10));
                gpio_put(PINO_2_MOTOR, 0);
                gpio_put(PINO_3_MOTOR, 1);
                vTaskDelay(pdMS_TO_TICKS(10));
                gpio_put(PINO_3_MOTOR, 0);
                gpio_put(PINO_4_MOTOR, 1);
                vTaskDelay(pdMS_TO_TICKS(10));
                gpio_put(PINO_4_MOTOR, 0);
            }
    }
    xSemaphoreGive(xSemaphoreMotor);
}

void senha_task(void *p) {
    printf("Inicializando Driver\n");
    ssd1306_init();

    printf("Inicializando GLX\n");
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);

    printf("Inicializando btn and LEDs\n");
    oled1_btn_led_init();

    char senha_digitada[5] = "0000"; // Inicialize senha_digitada com "0000"
    uint botao = 0;
    int num_buttons_pressed = 0; // Inicialize num_buttons_pressed com 0
    char progress_str[5];
    gfx_clear_buffer(&disp);

    while (1) {
        if (xQueueReceive(xQueueBTN, &botao, 1000)) {
            int valor = num_buttons_pressed - 1;
            if (senha_digitada[valor] == (botao + '0')) {
                continue;
            } else{
                senha_digitada[num_buttons_pressed++] = botao + '0'; // Converte o número do botão para um caractere e adiciona à senha digitada
            }
            printf("Botão pressionado: %d\n", botao);

            gfx_clear_buffer(&disp);
            
            memset(progress_str, '*', num_buttons_pressed);
            progress_str[num_buttons_pressed] = '\0';
            if (num_buttons_pressed < 4){
                gfx_draw_string(&disp, 0, 0, 3, progress_str);
            }
            gfx_show(&disp);

            if(num_buttons_pressed == 4 && senha_digitada[0] == senha[0] && senha_digitada[1] == senha[1] && senha_digitada[2] == senha[2] && senha_digitada[3] == senha[3]){
                gfx_clear_buffer(&disp);
                gfx_draw_string(&disp, 0, 0, 3, open);
                gfx_show(&disp);
                num_buttons_pressed = 0;
                memset(senha_digitada, '0', sizeof(senha_digitada));
                vTaskDelay(pdMS_TO_TICKS(1000));
                xSemaphoreGive(xSemaphoreMotor);
            } else if (num_buttons_pressed == 4 && (senha_digitada[0] != senha[0] || senha_digitada[1] != senha[1] || senha_digitada[2] != senha[2] || senha_digitada[3] != senha[3])) {
                gfx_clear_buffer(&disp);
                gfx_draw_string(&disp, 0, 0, 3, error);
                gfx_show(&disp);
                num_buttons_pressed = 0;
                memset(senha_digitada, '0', sizeof(senha_digitada));
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            vTaskDelay(pdMS_TO_TICKS(150));
        }
        
    }
}

int main() {
    stdio_init_all();

    gpio_init(PINO_1_MOTOR);
    gpio_init(PINO_2_MOTOR);
    gpio_init(PINO_3_MOTOR);
    gpio_init(PINO_4_MOTOR);

    gpio_set_dir(PINO_1_MOTOR, GPIO_OUT);
    gpio_set_dir(PINO_2_MOTOR, GPIO_OUT);
    gpio_set_dir(PINO_3_MOTOR, GPIO_OUT);
    gpio_set_dir(PINO_4_MOTOR, GPIO_OUT);

    xSemaphoreMotor = xSemaphoreCreateBinary();
    xQueueBTN = xQueueCreate(2, sizeof(uint));
    
    xTaskCreate(senha_task, "Senha task", 4095, NULL, 1, NULL);
    xTaskCreate(motor_task, "Motor task", 4095, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
