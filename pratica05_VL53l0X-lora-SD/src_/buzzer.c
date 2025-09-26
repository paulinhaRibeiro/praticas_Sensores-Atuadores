#include "buzzer.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h> 

void config_pwm_buzzer()
{
  gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);   // Configura o pino como PWM
  uint slice = pwm_gpio_to_slice_num(BUZZER_PIN); // Obtem o slice PWM do pino
  pwm_set_clkdiv(slice, PWM_DIVIDER);             // Aplica o divisor de clock
  pwm_set_enabled(slice, true);                   // Habilita o PWM
}

// Função para tocar um tom no buzzer
void play_tone(int frequency, int duration) {
  uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
  uint32_t wrap = CLOCK_PWM / (frequency * PWM_DIVIDER);  // Cálculo do wrap

  // Proteção contra valor inválido de wrap (> 65535)
  if (wrap > 65535) {
    printf("Erro: wrap excede limite de 16 bits para PWM!\n");
    return; // Sai da função sem tocar o tom
  }

  pwm_set_wrap(slice, wrap);  // Define o wrap para a frequência
  pwm_set_gpio_level(BUZZER_PIN, wrap / 2);  // Define o duty cycle

  sleep_ms(duration);  // Toca o tom pelo tempo especificado
  pwm_set_gpio_level(BUZZER_PIN, 0);  // Desliga o buzzer
}


// Função que simula alerta com beeps intermitentes baseados na distância (<10cm)
void play_alerta_cm() {
  // 2000 - Frequência do beep em Hz
  // 3000 - Duração do beep em ms
  // 100  - Duração do silencio

  for (int i = 0; i < 2; i++) {
    play_tone(2000, 3000);
    sleep_ms(100);
  }

}