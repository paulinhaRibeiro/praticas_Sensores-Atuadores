#include "servoMotor_9G_SG90.h"
#include "hardware/pwm.h" // Biblioteca para manipulação do PWM via hardware
#include "pico/stdlib.h" // inclui funções de GPIO, UART, timers
#include <stdio.h> // Biblioteca padrão do C para entrada e saída de dados


// ==== Configurações do Servo Motor ====
#define PINO_SERVO 2          // GPIO 2 ligado ao sinal do servo
#define PERIODO_SERVO 20000   // Período total em microssegundos (20 ms = 50 Hz)

// Duty Cycle (largura de pulso em microssegundos)
#define DUTY_MIN 1000u  // motor gira continuamente, sentido horário, velocidade máxima
#define DUTY_MID 1500u  // motor para (neutro)
#define DUTY_MAX 2000u  // motor gira continuamente, sentido anti-horário, velocidade máxima

// Variáveis globais para controle PWM
static uint slice_num; // slices (controladores) do PWM
static uint channel; // Canal do slice
static uint16_t wrap_value; // valor máximo do contador do PWM, que define o período


// ---- Define o duty cycle do servo ----
void set_servo_duty(uint32_t duty_us)
{
    // Como 1 tick = 1 µs, basta enviar o valor diretamente
    pwm_set_chan_level(slice_num, channel, (uint16_t)duty_us);
}


// ---- Ajusta o servo em função do ângulo de inclinação detectado ----
void ajustar_servo(uint8_t alertIncli)
{
    uint32_t duty_us;

    if (alertIncli == 1) {
        duty_us = DUTY_MIN; // giro horário rápido
        printf("Servo: Giro horário rápido - Duty: %uus\n", duty_us);
    }
    else {
        duty_us = DUTY_MID; // parado
        printf("Servo: Parado - Duty: %uus\n", duty_us);
    }
    
    // Usa hardware PWM para manter sinal
    set_servo_duty(duty_us);
}



// ---- Inicialização do PWM do servo ----
void setup_servoMotor_9G_SG90(void)
{
    // Configura o pino para função PWM
    gpio_set_function(PINO_SERVO, GPIO_FUNC_PWM);
    
    // Identifica o slice e channel do PWM
    slice_num = pwm_gpio_to_slice_num(PINO_SERVO);
    channel   = pwm_gpio_to_channel(PINO_SERVO);
    
    // Configuração do PWM
    pwm_config config = pwm_get_default_config();

    // Configura o clock divider para 125 -> 1 tick = 1 µs
    pwm_config_set_clkdiv(&config, 125.0f);

    // wrap = PERIODO_SERVO - 1 = 20000 - 1 = 19999 -> período de 20 ms
    wrap_value = PERIODO_SERVO - 1;
    pwm_config_set_wrap(&config, wrap_value);
    
    // Aplica a configuração e inicia o PWM
    pwm_init(slice_num, &config, true);
    
    // Inicialmente para o servo (neutro)
    set_servo_duty(DUTY_MID);
    
    printf("Servo PWM inicializado - Wrap value: %u\n", wrap_value);
}