#include "sensor_VL53L0X.h"
#include "sd_card.h"
#include "buzzer.h"


int main()
{
    stdio_init_all();                                   // Inicializa comunicação USB/Serial
    sleep_ms(3000);                                     // Espera 3 segundos para estabilizar
    printf("VL53L0X - Leitura de Distância (Laser)\n"); // Imprime título do programa

    printf("Mostrando apenas medições reais: %d mm a %d mm\n\n",
           DISTANCIA_MINIMA_VALIDA, DISTANCIA_MAXIMA_VALIDA);

    config_i2c();   // Configura hardware I2C
    sleep_ms(3000); // Espera 3 segundos para estabilização

    // Tenta inicializar sensor; se falhar, termina programa
    if (!vl53l0x_init())
    {
        printf("Falha ao inicializar o VL53L0X.\n");
        return 1;
    }

    printf("Sensor iniciado com sucesso.\n\n");

    config_pwm_buzzer(); // Configura o PWM do buzzer (para emitir sons)

    // ### Inicialização do SD ###
    printf("Inicializando SD Card...\n");
    init_spi_sdcard();
    sleep_ms(500);
    // #########################

    // Variável para detectar quando objeto entra na área de proximidade
    bool last_detect = false;

    while (1)
    {
        // Lê distância do sensor
        int distancia = vl53l0x_read_distance_mm();

        if (distancia_valida(distancia))
        {
            // Se distância é válida, imprime valor formatado --> MOSTRA APENAS MEDIÇÕES REAIS (50mm a 2000mm)
            printf("\n * Distância REAL: %d mm (%.2f m)\n", distancia, distancia / 1000.0f);

            // Detecta se objeto está a menos de 10cm
            bool detected = (distancia < 100); // < 10 cm

            // Se é uma nova detecção (não detectado antes), imprime alerta
            if (detected && !last_detect)
            {
                // calcula o tempo de atividade do sistema
                static uint32_t inicio = 0;
                if (inicio == 0)
                {
                    inicio = to_ms_since_boot(get_absolute_time());
                }

                uint32_t decorrido = to_ms_since_boot(get_absolute_time()) - inicio;
                uint32_t segundos = decorrido / 1000;

                // Monta a string com o tempo de atividade e o alerta
                char text[64];
                snprintf(text, sizeof(text), "[%02d:%02d:%02d]: ALERTA - Objeto a menos de 10 cm!\n",
                         segundos / 3600,
                         (segundos % 3600) / 60,
                         segundos % 60);

                // imprime aviso de evento
                printf(text);

                play_alerta_cm(); // Toca som de alerta com o buzzer

                // ### Escreve os dados no SDCard
                write_to_sd(text);
                // ### Lendo dados do SD
                read_from_sd();
            }
            // Atualiza estado anterior da detecção
            last_detect = detected;
        }
        else
        { // Se distância inválida, imprime mensagem de erro
            printf("\n===========================\n");
            printf(" === Medição inválida ===");
            printf("\n===========================\n");
        }
        // Espera 500ms entre leituras
        sleep_ms(500);
    }

    return 0;
}
