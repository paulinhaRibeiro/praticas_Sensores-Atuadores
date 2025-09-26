#include "servoMotor_9G_SG90.h"
#include "sensor_mpu6050.h"
#include "st7789.h"
#include "colors.h"

// Limite de inclinação para alerta
#define ANGULO_ALERTA_GRAUS 30.0f


int main()
{
    stdio_init_all();
    // ==== MPU6050
    setup_mpu6050();
    inicializar_mpu6050();
    // ==== Servo motor
    setup_servoMotor_9G_SG90();
    // ==== display ST7789 ###
    st7789_init(); 
    // Adiciona fundo branco a tela
    st7789_fill_screen(COLOR_WHITE);

    
    int16_t acel_x, acel_y, acel_z, gyro_x, gyro_y, gyro_z;
    float pitch = 0.0f;
    float roll = 0.0f;

    // Mensagens fixas no dispĺay
    draw_centered_text("Monitoramento", 50, COLOR_BLUE, COLOR_WHITE, 3);
    draw_centered_text("da Inclinacao", 80, COLOR_BLUE, COLOR_WHITE, 3);

    // Estado do alerta
    bool alertAtivo = false;
    while (true)
    {

        ler_acelerometro_gyro(&acel_x, &acel_y, &acel_z, &gyro_x, &gyro_y, &gyro_z);
        calcular_inclinacao(acel_x, acel_y, acel_z, &pitch, &roll);

        printf("Inclinação: Pitch (Para frente/trás) - %.2f deg, Roll (Para os lados) - %.2f deg\n", pitch, roll);

        if (fabsf(pitch) > ANGULO_ALERTA_GRAUS || fabsf(roll) > ANGULO_ALERTA_GRAUS){
            if(!alertAtivo){ // Só escreve se não estiver mostrando
                // Mostra alerta no display
                draw_centered_text("Alerta (Inclinacao):", 130, COLOR_RED, COLOR_WHITE, 2);
                draw_centered_text("Limite Ultrapassado", 160, COLOR_GRAY, COLOR_WHITE, 2);
            }
            alertAtivo = true;
            // Ativa servo (posição de alerta)
            ajustar_servo(1);

        } else {
            if (alertAtivo){
                // Limpa área de alerta no display
                st7789_fill_rect(30, 130, 250, 60, COLOR_WHITE);
                alertAtivo = false;
                ajustar_servo(0); // Retorna servo à posição normal - parado
            }

        }
        sleep_ms(1000);
    }
}