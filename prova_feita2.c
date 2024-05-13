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
const uint BTN_2_OLED = 27;
const uint BTN_3_OLED = 26;

const uint LED_1_OLED = 20;
const uint LED_2_OLED = 21;
const uint LED_3_OLED = 22;

// define as portas do motor 
const int M1 = 5; 
const int M2 = 4;
const int M3 = 2;
const int M4 = 3;

const int volta_motor = 2048; 
const uint32_t senha[4] = {BTN_1_OLED, BTN_1_OLED, BTN_1_OLED, BTN_1_OLED};

QueueHandle_t xQueueBTN;
SemaphoreHandle_t xSemaphoreMotor;


// BTN CALLBACK
void btn_callback(uint gpio, uint32_t events) {
    // verifica qual botão foi apertado
    // printf("Botao apertado: %d\n", gpio);
    if (gpio == BTN_1_OLED) {
        // printf("enviou fila apertou 1 \n"); 
        xQueueSendFromISR(xQueueBTN, &BTN_1_OLED, 0);
    } else if (gpio == BTN_2_OLED) {
        // printf("enviou fila apertou 2\n"); 
        xQueueSendFromISR(xQueueBTN, &BTN_2_OLED, 0);
    } else if (gpio == BTN_3_OLED) {
        // printf("enviou fila apertou 3\n"); 
        xQueueSendFromISR(xQueueBTN, &BTN_3_OLED, 0);
    }
}

// SENHA
// deve ser a sequencia de botoes = 1, 3, 2, 3
// para cada botão apertado deve aparecer um * no display
// se a sequencia for correta, deve aparecer "Senha correta" no display
// se a senha por incorreta, deve aparecer senha incorreta no display
void task_senha(void *p) {
    printf("Inicializando Driver\n");
    ssd1306_init();

    printf("Inicializando GLX\n");
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);

    printf("Inicializando btn and LEDs\n");
    oled1_btn_led_init();
    // uint32_t senha[4] = {BTN_1_OLED, BTN_3_OLED, BTN_2_OLED, BTN_3_OLED};
    uint32_t senha_digitada[4] = {0, 0, 0, 0};
    uint32_t senha_digitada_index = 0;
    bool senha_correta = false;

    while (1) {
        uint32_t btn;
        if (xQueueReceive(xQueueBTN, &btn, portMAX_DELAY) == pdTRUE) {
            // ascende botao recebido
            gpio_put(btn, 1); 
            senha_digitada[senha_digitada_index] = btn;
            senha_digitada_index++;

            gfx_clear_buffer(&disp);
            gfx_draw_string(&disp, 0, 0, 1, "Digite a senha:");
            
            // Desenha '*' para cada dígito digitado
            for (int i = 0; i < senha_digitada_index; i++) {
                char char_to_draw = '*'; // Caractere a ser desenhado
                char str_to_draw[2] = {char_to_draw, '\0'}; // Constrói uma string com o caractere a ser desenhado
                gfx_draw_string(&disp, i * 8, 10, 1, str_to_draw); // Desenha a string no display
            }

            gfx_show(&disp);

            if (senha_digitada_index == 4) {
                // Verifica se a senha digitada está correta
                senha_correta = true;
                for (int i = 0; i < 4; i++) {
                    if (senha_digitada[i] != senha[i]) {
                        senha_correta = false;
                        break;
                    }
                }

                // Libera o semáforo caso a senha esteja correta para o motor girar 180 graus
                if (senha_correta) {
                    printf("Senha correta - enviando sinal no semaforo\n");
                    xSemaphoreGive(xSemaphoreMotor);
                }

                gfx_clear_buffer(&disp);
                gfx_draw_string(&disp, 0, 0, 1, senha_correta ? "OPEN" : "ERROR");
                gfx_show(&disp);
                senha_digitada_index = 0;
                vTaskDelay(pdMS_TO_TICKS(5000)); // Aguarda 2 segundos antes de limpar o OLED
                gfx_clear_buffer(&disp);
            }
        }
    }
}


// task para girar o motor
void task_motor(void *p) {
    // inicializando o motor
    printf("Inicializando motor\n"); 
    motor_init(); 

    while (1) {
        if (xSemaphoreTake(xSemaphoreMotor, portMAX_DELAY)) {
            printf("Girando motor 180 graus\n");
            for (int i = 0; i < volta_motor / 8; i++)
            {
                gpio_put(M1, 1);
                vTaskDelay(pdMS_TO_TICKS(10)); 
                gpio_put(M1, 0);
                vTaskDelay(pdMS_TO_TICKS(10)); 
                gpio_put(M2, 1);
                vTaskDelay(pdMS_TO_TICKS(10)); 
                gpio_put(M2, 0);
                vTaskDelay(pdMS_TO_TICKS(10)); 
                gpio_put(M3, 1);
                vTaskDelay(pdMS_TO_TICKS(10)); 
                gpio_put(M3, 0);
                vTaskDelay(pdMS_TO_TICKS(10)); 
                gpio_put(M4, 1);
                vTaskDelay(pdMS_TO_TICKS(10)); 
                gpio_put(M4, 0);
                vTaskDelay(pdMS_TO_TICKS(10)); 
            }

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

// inicializa oled, botões e leds
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
    gpio_set_irq_enabled_with_callback(BTN_1_OLED, GPIO_IRQ_EDGE_RISE, true, &btn_callback);

    gpio_init(BTN_2_OLED);
    gpio_set_dir(BTN_2_OLED, GPIO_IN);
    gpio_pull_up(BTN_2_OLED);
    gpio_set_irq_enabled(BTN_2_OLED, GPIO_IRQ_EDGE_RISE, true); // Adicionado callback para BTN_2_OLED

    gpio_init(BTN_3_OLED);
    gpio_set_dir(BTN_3_OLED, GPIO_IN);
    gpio_pull_up(BTN_3_OLED);
    gpio_set_irq_enabled(BTN_3_OLED, GPIO_IRQ_EDGE_RISE, true); // Adicionado callback para BTN_3_OLED
}


// inicializa o motor
void motor_init(void) {
    gpio_init(M1);
    gpio_set_dir(M1, GPIO_OUT);

    gpio_init(M2);
    gpio_set_dir(M2, GPIO_OUT);

    gpio_init(M3);
    gpio_set_dir(M3, GPIO_OUT);

    gpio_init(M4);
    gpio_set_dir(M4, GPIO_OUT);
}



int main() {

    // inicializa o stdio e os componentes
    stdio_init_all();

    // inicializa a fila 
    xQueueBTN = xQueueCreate(10, sizeof(uint32_t));

    // inicializa o semaforo
    xSemaphoreMotor = xSemaphoreCreateBinary();

    // trocar as tasks ativas, apenas uma por vez!
    xTaskCreate(task_senha, "Senha", 4096, NULL, 1, NULL);
    xTaskCreate(task_motor, "Motor", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
