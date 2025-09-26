#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "st7789.h"
#include "colors.h"
#include "sensor_AHT10.h"

// --- Constantes para limites de temperatura e umidade ---
#define TEMP_MIN 10   // Temperatura mínima para barra (°C)
#define TEMP_MAX 50   // Temperatura máxima para barra (°C)
#define HUM_MIN 10    // Umidade mínima para barra (%)
#define HUM_MAX 90    // Umidade máxima para barra (%)


int main()
{
    stdio_init_all(); // Inicializa entrada/saída padrão
    st7789_init(); // Inicializa o display ST7789

    st7789_fill_screen(COLOR_WHITE);

    // Inicialização do sensor AHT10
    printf("Inicializando AHT10...\n");
    setup_aht10();
    aht10_config();
    printf("AHT10 inicializado com sucesso!\n");

    float temperature, humidity; // Variáveis para armazenar leituras

    char buffer[32]; // Buffer para formatar strings exibidas no display

    while (true)
    {
        // Tenta ler dados do AHT10
        if (aht10_read(&temperature, &humidity))
        {
            // Exibe valores no terminal
            printf("Temperatura: %.2f °C | Umidade: %.1f %%\n", temperature, humidity);

            // --- Exibição da Temperatura ---
            snprintf(buffer, sizeof(buffer), "Temp: %.1f C", temperature);
            draw_centered_text(buffer, 20,
                            // Cor do texto depende da faixa da temperatura
                               temperature < 20.0f ? COLOR_BLUE : (temperature > 28.0f ? COLOR_RED : COLOR_BLACK),
                               COLOR_WHITE, 2);

            // Calcula porcentagem da temperatura em relação ao intervalo definido
            int temp_percent = (int)(((temperature - TEMP_MIN) * 100.0f) / (TEMP_MAX - TEMP_MIN));
            if (temp_percent < 0)
                temp_percent = 0;
            if (temp_percent > 100)
                temp_percent = 100;
            
            // Desenha barra de progresso para temperatura
            draw_bar(40, 50, 240, 20, temp_percent,
                     temperature < 20.0f ? COLOR_BLUE : (temperature > 28.0f ? COLOR_RED : COLOR_GREEN));



            // --- Exibição da Umidade ---
            snprintf(buffer, sizeof(buffer), "Umid: %.1f %%", humidity);
            draw_centered_text(buffer, 90,
                                // Se umidade > 70%, destaca em azul
                               humidity > 70.0f ? COLOR_BLUE : COLOR_BLACK,
                               COLOR_WHITE, 2);

            // Calcula porcentagem da umidade em relação ao intervalo definido
            int hum_percent = (int)(((humidity - HUM_MIN) * 100.0f) / (HUM_MAX - HUM_MIN));
            if (hum_percent < 0)
                hum_percent = 0;
            if (hum_percent > 100)
                hum_percent = 100;

            // Desenha barra de progresso para umidade
            draw_bar(40, 120, 240, 20, hum_percent,
                     humidity > 70.0f ? COLOR_BLUE : COLOR_RED);


            // --- Avisos visuais no display ---
            if (humidity > 70.0f)
            {
                // Aviso de umidade alta
                draw_centered_text("Umid Alta!", 180, COLOR_BLUE, COLOR_WHITE, 3);
            }
            if (temperature < 20.0f)
            {
                // Aviso de temperatura baixa
                draw_centered_text("Temp Baixa!", 150, COLOR_BLUE, COLOR_WHITE, 3);
                draw_centered_text("FRIO!", 220, COLOR_BLUE, COLOR_WHITE, 2);
            }
        }
        else
        { // Caso falhe a leitura do sensor
            printf("Falha na leitura do AHT10. Tentando resetar...\n");
            aht10_reset(); // Tenta resetar em caso de falha
            sleep_ms(500); // Pequeno atraso antes de tentar novamente
        }

        // Aguarda 1 segundo antes da próxima leitura
        sleep_ms(1000);
    }
}
