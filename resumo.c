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

#include "pico/util/datetime.h"
//#include "hardware/rtc.h"

const uint BTN_1_OLED = 28;
const uint BTN_2_OLED = 26;
const uint BTN_3_OLED = 27;

const uint LED_1_OLED = 20;
const uint LED_2_OLED = 21;
const uint LED_3_OLED = 22;

// Filas e semáforos para sincronização
QueueHandle_t xQueueBtn;

typedef struct btn {
    int id;
    int status;
    uint32_t tempo;
} btn_t;

typedef struct tempo_disparo {
    uint32_t inicial;
    uint32_t final;
} tempo_disparo_t;


void btn_callback(uint gpio, uint32_t events) {
    btn_t btn_data;
    static uint32_t last_press_time = {0};

    if (events == 0x4) {
        btn_data.id = gpio;
        btn_data.status = events;
        uint32_t now = to_us_since_boot(get_absolute_time()); // Obtém o tempo atual
        if (now - last_press_time > 200000) {
            btn_data.tempo = now;
            xQueueSendFromISR(xQueueBtn, &btn_data, 0);
        }
    }
    
}

void oled1_btn_led_init(void) {
    gpio_init(BTN_1_OLED);
    gpio_set_dir(BTN_1_OLED, GPIO_IN);
    gpio_pull_up(BTN_1_OLED);
    gpio_set_irq_enabled_with_callback(
         BTN_1_OLED, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &btn_callback);

    gpio_init(BTN_2_OLED);
    gpio_set_dir(BTN_2_OLED, GPIO_IN);
    gpio_pull_up(BTN_2_OLED);
    gpio_set_irq_enabled_with_callback(
         BTN_2_OLED, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &btn_callback);

    gpio_init(BTN_3_OLED);
    gpio_set_dir(BTN_3_OLED, GPIO_IN);
    gpio_pull_up(BTN_3_OLED);
    gpio_set_irq_enabled_with_callback(
         BTN_3_OLED, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &btn_callback);
}

void oled1_demo_1(void *p) {
    printf("Inicializando Driver\n");
    ssd1306_init();

    printf("Inicializando GLX\n");
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);

    printf("Inicializando btn and LEDs\n");
    oled1_btn_led_init();

    char cnt = 15;
    while (1) {

        if (gpio_get(BTN_1_OLED) == 0) {
            cnt = 15;
            gpio_put(LED_1_OLED, 0);
            gfx_clear_buffer(&disp);
            gfx_draw_string(&disp, 0, 0, 1, "LED 1 - ON");
            gfx_show(&disp);
        } else if (gpio_get(BTN_2_OLED) == 0) {
            cnt = 15;
            gpio_put(LED_2_OLED, 0);
            gfx_clear_buffer(&disp);
            gfx_draw_string(&disp, 0, 0, 1, "LED 2 - ON");
            gfx_show(&disp);
        } else if (gpio_get(BTN_3_OLED) == 0) {
            cnt = 15;
            gpio_put(LED_3_OLED, 0);
            gfx_clear_buffer(&disp);
            gfx_draw_string(&disp, 0, 0, 1, "LED 3 - ON");
            gfx_show(&disp);
        } else {

            gpio_put(LED_1_OLED, 1);
            gpio_put(LED_2_OLED, 1);
            gpio_put(LED_3_OLED, 1);
            gfx_clear_buffer(&disp);
            gfx_draw_string(&disp, 0, 0, 1, "PRESSIONE ALGUM");
            gfx_draw_string(&disp, 0, 10, 1, "BOTAO");
            gfx_draw_line(&disp, 15, 27, cnt,
                          27);
            vTaskDelay(pdMS_TO_TICKS(50));
            if (++cnt == 112)
                cnt = 15;

            gfx_show(&disp);
        }
    }
}

void oled1_demo_2(void *p) {
    printf("Inicializando Driver\n");
    ssd1306_init();

    printf("Inicializando GLX\n");
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);

    printf("Inicializando btn and LEDs\n");
    oled1_btn_led_init();

    while (1) {

        gfx_clear_buffer(&disp);
        gfx_draw_string(&disp, 0, 0, 1, "Mandioca");
        gfx_show(&disp);
        vTaskDelay(pdMS_TO_TICKS(150));

        gfx_clear_buffer(&disp);
        gfx_draw_string(&disp, 0, 0, 2, "Batata");
        gfx_show(&disp);
        vTaskDelay(pdMS_TO_TICKS(150));

        gfx_clear_buffer(&disp);
        gfx_draw_string(&disp, 0, 0, 4, "Inhame");
        gfx_show(&disp);
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void oled_task(void *pvParameters) {
    ssd1306_init();
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);
    btn_t btn_data;
    tempo_disparo_t tempo_data;
    uint32_t dif_tempo;
    int press_count = 0;
    char buffer[128];

    while (1) {
        while (press_count < 2) {
            gfx_clear_buffer(&disp);
            snprintf(buffer, sizeof(buffer), "Botao pressionado %d", press_count);
            gfx_draw_string(&disp, 0, 0, 1, buffer);
            gfx_draw_string(&disp, 0, 12, 1, "vezes");
            gfx_show(&disp);

            if (xQueueReceive(xQueueBtn, &btn_data, portMAX_DELAY)) {
                press_count ++;
                if (press_count == 1) {
                    tempo_data.inicial = btn_data.tempo;
                } else if (press_count == 2) {
                    tempo_data.final = btn_data.tempo;
                }
            }
        }

        if (press_count == 2){
            gfx_clear_buffer(&disp);
            dif_tempo = tempo_data.final - tempo_data.inicial;
            snprintf(buffer, sizeof(buffer), "Tempo que o botao ");
            gfx_draw_string(&disp, 0, 0, 1, buffer);
            snprintf(buffer, sizeof(buffer), "ficou ativo foi: %lu");
            gfx_draw_string(&disp, 0, 12, 1, buffer);
            snprintf(buffer, sizeof(buffer), "%lu segundos", dif_tempo);
            gfx_draw_string(&disp, 0, 24, 1, buffer);
            gfx_show(&disp);
            vTaskDelay(pdMS_TO_TICKS(2500));
            press_count = 0;
        }
    }
}


int main() {
    stdio_init_all();

    oled1_btn_led_init();

    xQueueBtn = xQueueCreate(32, sizeof(btn_t));

    xTaskCreate(oled_task, "OLED Task", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}