/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#include "hardware/rtc.h"
#include <string.h>

#define BUFFER_SIZE 10 //vai armazenar a entrada do usuario

const int TRIG_PIN = 15;
const int ECHO_PIN = 14;

int SOUND_SPEED = 340;

//medicao do sensor habilitada ou nao
volatile bool measure_enabled = false;

volatile bool measurement_enabled = false;

volatile absolute_time_t time_init;
volatile absolute_time_t time_end;
static volatile bool timer_fired = false;
volatile bool sec_fired = false; 

static void alarm_callback(alarm_id_t id, void *user_data) {
    timer_fired = true;
}

//callback registra o tempo em que ocorreu o rise e fall
static void gpio_callback(uint gpio, uint32_t events) {
    if (events == GPIO_IRQ_EDGE_RISE) {
        time_init = get_absolute_time();
    } else if (events == GPIO_IRQ_EDGE_FALL) {
        time_end = get_absolute_time();
    }
}


void print_log(absolute_time_t timestamp, float distance) {
    // printa no oled
    uint64_t time_microseconds;
    memcpy(&time_microseconds, &timestamp, sizeof(uint64_t));
    int32_t time_seconds = time_microseconds / 1000000;
    time_microseconds %= 1000000;

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d - %.2lf cm", time_seconds / 3600, (time_seconds % 3600) / 60, time_seconds % 60, distance);

    gfx_clear_buffer(&disp);
    gfx_draw_string(&disp, 0, 0, 1, buffer);
    gfx_show(&disp);

    //printf("%02d:%02d:%02d - %.2lf cm\n", time_seconds / 3600, (time_seconds % 3600) / 60, time_seconds % 60, distance);
} 

// void process_command(char *buffer) {
//     //comparacao entre strings 
//     if (strcmp(buffer, "start") == 0) {
//         measurement_enabled = true;
//         printf("Medição iniciada.\n");
//     } else if (strcmp(buffer, "stop") == 0) {
//         measurement_enabled = false;
//         printf("Medição parada.\n");
//     } else {
//         printf("Comando inválido.\n");
//     }
// }    

int main() {
    stdio_init_all();

    gpio_init(TRIG_PIN);
    gpio_set_dir(TRIG_PIN, GPIO_OUT);

    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(ECHO_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    // configura o rtc para iniciar em um momento especifico
    datetime_t t = {
        .year  = 2020,
        .month = 01,
        .day   = 13,
        .dotw  = 3, // 0 is Sunday, so 3 is Wednesday
        .hour  = 11,
        .min   = 20,
        .sec   = 00
    };
    rtc_init();
    rtc_set_datetime(&t);

    // configura o alarme para disparar uma vez a cada segundo
    datetime_t alarm = {
        .year  = -1,
        .month = -1,
        .day   = -1,
        .dotw  = -1,
        .hour  = -1,
        .min   = -1,
        .sec   = 01 
    };

    rtc_set_alarm(&alarm, &alarm_callback);


    char buffer[BUFFER_SIZE + 1]; // +1 para o caractere nulo
    int index = 0;

    while (true) {
        int c = getchar_timeout_us(20);

        if (c != PICO_ERROR_TIMEOUT) { //se algum caractere foi lido dentro dos 20 microssegundos, fazemos o processamento, senao aguardamos a entrada
            buffer[index++] = c; // Adiciona o caractere ao buffer e incrementa o índice
            buffer[index] = '\0'; // Adiciona um caractere nulo ao final do buffer

            if (strcmp(buffer, "start") == 0 || strcmp(buffer, "stop") == 0) {
                process_command(buffer); // Processa o comando
                index = 0; // Reinicia o índice do buffer

                if (!measurement_enabled) {
                    continue; // Pular a medição se ela estiver desabilitada
                }
            }
            // } else if (index >= BUFFER_SIZE) { // Se o buffer estiver cheio
            //     printf("Buffer cheio. Ignorando caracteres adicionais.\n");
            //     index = 0; // Reinicia o índice do buffer em caso de buffer cheio
            // }   
        }

        if (measurement_enabled) {
            // gera pulso no pino trig:
            gpio_put(TRIG_PIN, 1); 
            sleep_us(10);
            gpio_put(TRIG_PIN, 0);

            // calcula a duracao do pulso de eco recebido pelo sensor. time_end e time_init marcam a subida e descida de borda
            int ultrassom_duration = absolute_time_diff_us(time_end, time_init);

            //distabcia = velocidade * duracan
            float distancia_cm = 0.017015 * ultrassom_duration * (-1);

            // horario da medicao e medida
            print_log(time_init, distancia_cm);

            timer_fired = false; 
            sleep_ms(1000); 
        }

    }
}