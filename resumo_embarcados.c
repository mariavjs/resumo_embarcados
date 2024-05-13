// ADC - hardware capaz de converter um valor analógico em um valor digital

// Para usar o PWM você deve modificar o CMakeLists.txt adicionando hardware_adc 
// no target_link_libraries: target_link_libraries(hardware_adc) (adicionar)

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
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

int main() {
    stdio_init_all();

    adc_init(); // ADICIONAR

    xTaskCreate(adc_1_task, "LED_Task 1", 4095, NULL, 1, NULL); // ADICIONAR
    vTaskStartScheduler(); // ADICIONAR

    while (true) {
    }
}

// OLED 

#include "ssd1306.h"
#include "gfx.h"

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

void oled(void *p) {
    printf("Inicializando Driver\n");
    ssd1306_init();

    printf("Inicializando GLX\n");
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);

    printf("Inicializando btn and LEDs\n");
    oled1_btn_led_init();

    // LED
    gpio_put(LED_NUMERO_OLED, 0); // Liga
    gpio_put(LED_NUMERO_OLED, 1); // Desliga

    // Botao
    gpio_get(BTN_NUMERO_OLED) // Se o resultado for 0 ele foi apertado, se for 1 nao foi apertado

    // Escrever na tela
    gfx_clear_buffer(&disp); // Limpa o display
    gfx_draw_string(&disp, 0, 0, 1, "Mensagem"); // (onde vai ser escrito, x_init, y_init, msg)
    gfx_show(&disp); // Escreve no display

    // Linha na tela
    gfx_clear_buffer(&disp); // Limpa o display
    gfx_draw_line(disp, 0, 22, 10, 22); // (x_init, y_init, x_fim, y_fim) (normalmente a linha tera o msm y para inicio e fim)
    gfx_show(&disp); // Escreve no display
}

// RTOS

// Task - pequenos programas executados pelo sistema operacional (é uma função que não retorna e possui laço infinito [while(1)])

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 250;
    while (true) {
        gpio_put(LED_PIN_R, 1);
        vTaskDelay(pdMS_TO_TICKS(delay)); // Como fazer delay em task
        gpio_put(LED_PIN_R, 0);
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}

int main() {
    stdio_init_all();

    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL); // (ponteiro para a funcao da task, nome, tamanho da pilha, ponteiro para os parametros, grau de prioridade, ponteiro para o identificador da tarefa criada)
    vTaskStartScheduler(); // Passa o comando do core para o FreeRTOS!

    while (true)
        ;
}

// Semaphore - utilizados para gerenciar o acesso a recursos compartilhados por múltiplas tarefas
SemaphoreHandle_t xSemaphoreMotor; // Inicializar no codigo

void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) {         // fall edge
        xSemaphoreGiveFromISR(xSemaphore, 0); // Se for liberado de uma ISR
    }
} 

void task_main(void) {
 // ....

    while(1) { 

        if(xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(100))) // Entra se o semafaro for liberado
            // faz alguma coisa
        } else { 
            // cai aqui se o semáforo não for liberado em 100 ms!
        }
}

// outro jeito de liberar o semafaro (feito na AV1)

// Dentro de uma task quando for pra liberar o semafaro
xSemaphoreGive(xSemaphoreMotor); // Se for liberado de outra task

// Na task que estava esperando pelo semafaro
if (xSemaphoreTake(xSemaphoreMotor, portMAX_DELAY) == pdTRUE) { // Pode subistituir o "portMAX_DELAY" por "pdMS_TO_TICKS(500)"
            // O semáforo foi tomado com sucesso, podemos operar o motor
}

int main() {
    xSemaphoreMotor = xSemaphoreCreateBinary(); // Isso ativa o semafaro na main

    while (true)
        ;
}

// Queue - enviar dados entre tarefa em um sistema operacional (callback para task, task para callback e task para task)

// Enviar dados
xQueueSendFromISR(xQueueTime, &DADO, 0); // Se enviado via IRS (callback) para task
xQueueSend(xQueueBtn, &DADO, 0 ); // Se enviado via task para task

// Receber dados
if (xQueueReceive(xQueueBtn, &DADO,  pdMS_TO_TICKS(100))) {
      printf("Botão pressionado pino %d", DADO);
    } else {
      // cai aqui se não chegar um dado em 100 ms!
}

// Usando 
QueueHandle_t xQueueButId; // Definindo no codigo

int main() {
    stdio_init_all();

    xQueueButId = xQueueCreate(32, sizeof(char) ); // Definir na main (quantidade de itens, tipo dos itens)

    while (true)
        ;
}

// Queue com struct

typedef struct btn {
    int id;
    int status;
} btn_t;

xQueueBtn = xQueueCreate(32, sizeof(btn_t)); // Criar a fila com o tamanho do struct

// Enviar 
void btn_callback(uint gpio, uint32_t events) {
    btn_t btn_data;
    btn_data.id = gpio;
    btn_data.status = events;

    // FromISR pq estamos em uma IRS!
    xQueueSendFromISR(xQueueData, &btn_data, 0);
}

// Receber - fica igual
if (xQueueReceive(xQueueData, &btn_data, portMAX_DELAY)) {}