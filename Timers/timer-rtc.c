// RESUMO DE TIMERS
// timer - periferico

#include <stdio.h>
#include "pico/stdlib.h"

// FUNÇÕES DE TEMPO ABSLUTO
int main() {
    stdio_init_all();
    while(1){
        // funcoes para pegar o tempo absoluto desde a inicializacao
        // get_absolute_time() retorna o tempo absoluto em microsegundos
        uint32_t start_ms = to_ms_since_boot(get_absolute_time()); 
        uint64_t start_us = to_us_since_boot(get_absolute_time());
        sleep_ms(100);
    }
}

// FUNÇÕES DE TIMER REPETITIVO
// criar timers que chamam função de callback a cada x Hz
// lembrar de configurar uma função de callback PARA CADA TIMER

bool timer_0_callback(repeating_timer_t *rt) { // callback do timer, deve contar a função a ser executada
    g_timer_0 = 1;
    return true; // keep repeating - timer periodico
}

int main() {
    stdio_init_all();

    int timer_0_hz = 5; // definir o hx do timer
    repeating_timer_t timer_0;

    // tb pode ser usado dd_repeating_timer_ms
    if (!add_repeating_timer_us(1000000 / timer_0_hz, 
                                timer_0_callback,
                                NULL, 
                                &timer_0)) {
        printf("Failed to add timer\n");
    }

    // PARA CRIAR MAIS DE UM TIMER:
    // Lembre de que para cada timer você precisa declarar um novo
    // repeating_timer e uma nova função de callback.

    // PARA SER UM TIMER QUE APITA APENAS UMA VEZ
    // return do callback deve ser false
    // PARA SER UM TIMER PERIODICO
    // return do callback deve ser true

    // PARA CANCELAR UM TIMER CHAMAR:
    // cancel_repeating_timer(&timer);

    // Tempo negativo ou positivo:
    // - Se o atraso for > 0, então este é o atraso entre o término do callback anterior e o início do próximo.
    // - Se o atraso for negativo, então a próxima chamada ao callback será exatamente 500ms após o início da chamada ao último callback.

    while(1){
        if(g_timer_0){ // se o timer 0 foi chamado
            printf("Hello timer 0 \n");
            g_timer_0 = 0;
        }
    }
}

// FUNÇÕES DE ALARM
// usado para criar eventos NÃO PERIÓDICOS
volatile bool timer_fired = false;

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    timer_fired = true; // variavel que define se o alarme foi chamado ou não
    // Can return a value here in us to fire in the future
    return 0;
}

int main() {
    stdio_init_all();

    // Call alarm_callback in 300 ms
    // tb pode ser usado add_alarm_at_us
    alarm_id_t alarm = add_alarm_in_ms(300, alarm_callback, NULL, false)

    if (!alarm) {
        printf("Failed to add timer\n");
    }

    // usar esse alarm para cancelar 

    // cancel_alarm(alarm); 

    while(1){ 
        if(timer_fired){ // alarme foi chamado ? 
            timer_fired = 0;
            printf("Hello from alarm!");
        }
    }
}