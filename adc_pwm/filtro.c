// aplica filtro nos dados que chegam numa fila
// faz a media movel desses dados


#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

#include "data.h"
QueueHandle_t xQueueData;

// não mexer! Alimenta a fila com os dados do sinal
// pega os dados O objetivo principal da data_task é garantir que a fila 
//esteja sempre abastecida com novos dados para processamento, imitando um 
// cenário de aquisição de dados em tempo real.
void data_task(void *p) {
    vTaskDelay(pdMS_TO_TICKS(400));

    int data_len = sizeof(sine_wave_four_cycles) / sizeof(sine_wave_four_cycles[0]);
    for (int i = 0; i < data_len; i++) {
        xQueueSend(xQueueData, &sine_wave_four_cycles[i], 1000000);
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


//Funcionalidade: A process_task espera receber dados da fila. Quando novos 
//dados são disponibilizados, ela atualiza um buffer de dados (data_buffer) e 
// calcula a média desses valores. Esse cálculo de média móvel atua como um filtro 
// simples, ajudando a suavizar variações abruptas nos dados.
// Objetivo: O principal propósito da process_task é processar os dados recebidos de 
// forma a obter informações úteis ou prepará-los para um posterior processamento ou análise,
//  como a filtragem ou cálculo de médias.
void process_task(void *p) {
    int data;
    int data_buffer[5];
    int media;
    while (true) {
        if (xQueueReceive(xQueueData, &data, 100)) {
            // implementar filtro aqui!


            data_buffer[0]=data_buffer[1];
            data_buffer[1]=data_buffer[2];
            data_buffer[2]=data_buffer[3];
            data_buffer[3]=data_buffer[4];
            data_buffer[4]=data;
            
        
            media=((data_buffer[4]+data_buffer[3]+data_buffer[2]+data_buffer[1]+data_buffer[0])/5);
            printf("%d\n",media);
        
        
            // deixar esse delay!
           
            vTaskDelay(pdMS_TO_TICKS(50));

        }
    }
}

int main() {
    stdio_init_all();

    xQueueData = xQueueCreate(64, sizeof(int));

    xTaskCreate(data_task, "Data task ", 4096, NULL, 1, NULL);
    xTaskCreate(process_task, "Process task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}