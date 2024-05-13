
**OLED:**
*Escrvevendo no display:*

```
ssd1306_t disp;
#define OLED_WIDTH 128
#define OLED_HEIGHT 32

Dentro da task

	char buffer[32]; //Limpar o que esta escrito antes no display
	ssd1306_init();
	gfx_init(&disp, 128, 32);
	gfx_clear_buffer(&disp);
	gfx_draw_string(&disp, 0, 0, 1, "TEXTO!");
	gfx_show(&disp);

```


*Botões e LEDs (Oled):*
```
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

const uint BTN_1_OLED = 28;
const uint BTN_2_OLED = 26;
const uint BTN_3_OLED = 27;

const uint LED_1_OLED = 20;
const uint LED_2_OLED = 21;
const uint LED_3_OLED = 22;

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


int main() {
    stdio_init_all();

    xTaskCreate(oled1_demo_1, "Demo 2", 4095, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}


```

ou 

```
void task_senha(void *params) {
    uint btn_id;

    while (1) {
        if (xQueueReceive(xQueueBtn, &btn_id, portMAX_DELAY)) {
            // Limpa o display antes de mostrar o novo texto
            ssd1306_init();
            gfx_init(&disp, 128, 32);
            gfx_clear_buffer(&disp);

            // Checa qual botão foi pressionado e exibe a mensagem correspondente
            if (btn_id == 1) {
                gfx_draw_string(&disp, 0, 0, 1, "Botao 1");
            } else if (btn_id == 2) {
                gfx_draw_string(&disp, 0, 0, 1, "Botao 2");
            } else if (btn_id == 3) {
                gfx_draw_string(&disp, 0, 0, 1, "Botao 3");
            }
            gfx_show(&disp);

            // Aguarda 2 segundos e limpa o display
            vTaskDelay(pdMS_TO_TICKS(2000));
            gfx_clear_buffer(&disp);
            gfx_show(&disp);
        }

        vTaskDelay(pdMS_TO_TICKS(50));  // Pequeno delay para a iteração do loop
    }
}
```


*Joystick:*
```
Lab 5 - Joystick

gnd - gnd
+5V - 3v3_EN
VRX - 26
VRY - 27
```


*Botoes e LED RGB:*
```
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const uint LED_PIN = 15;   // GP15 para o LED
const uint BUTTON_PIN = 14; // GP14 para o botão 

int main() {
    // Inicializa o sistema de I/O padrão
    stdio_init_all();
    
    // Configura o pino do LED como saída
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // Configura o pino do botão como entrada
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);  // Ativa o resistor de pull-up interno

    bool led_state = false;        // Estado atual do LED
    bool last_button_state = true; // Último estado conhecido do botão (true = não pressionado)
    while (true) {        
        // Lê o estado atual do botão
        bool current_button_state = gpio_get(BUTTON_PIN);

        // Verifica se houve transição de não pressionado para pressionado
        if (last_button_state && !current_button_state) {
            // Inverte o estado do LED
            led_state = !led_state;
            gpio_put(LED_PIN, led_state);
            printf("Botao pressionado, LED %s\n", led_state ? "ligado" : "desligado");
        }

        // Atualiza o último estado do botão
        last_button_state = current_button_state;

        // Pequeno atraso para reduzir o rebote do botão e a velocidade de leitura
        sleep_ms(50);
    } 
    return 0;
}


```


*Motor:*
```
LAB 1 - pra
Ao apertar botao, led ascende e motor gira.

IN4 - GP18
IN3 - GP19
IN2 - GP20
IN1 - GP21
```


*Potenciômetro:*
```


```


*Distância:*
```


```
