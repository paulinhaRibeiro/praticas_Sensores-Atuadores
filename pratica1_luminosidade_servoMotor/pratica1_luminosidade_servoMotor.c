#include "pico/stdlib.h" // inclui funções de GPIO, UART, timers
#include <stdio.h> // Biblioteca padrão do C para entrada e saída de dados
#include "servoMotor_9G_SG90.h"
#include "sensor_lumi_BH1750.h"


int main()
{
    // Inicializa UART para debug
    stdio_init_all();

    setup_servoMotor_9G_SG90();
    setup_BH1750();

    // Inicia a medição no sensor BH1750
    bh1750_start_measurement();

    int lux = 0;

    while (true)
    {
        // Ler a luminosidade
        lux = bh1750_read_lux();

        if (lux >= 0)
        {
            printf("Luminosidade: %ld lux\n", lux);
            // Ajustar o servo de acordo
            ajustar_servo((uint16_t)lux);
        } else {
            printf("Erro na leitura do BH1750!\n");
        }

        sleep_ms(1000); // Aguarda 1 segundo para a próxima leitura
    }

    return 0;
}
