#ifndef BUZZER_H
#define BUZZER_H

/*  ***********************************
        Variáveis PWM - BUZZER
    ***********************************/
#define BUZZER_PIN 10 // Pino do buzzer
#define CLOCK_PWM 125000000
#define PWM_DIVIDER 100.0

// Função para configurar o PWM para o buzzer
extern void config_pwm_buzzer();
extern void play_alerta_cm();
#endif