#include "gps_gy-neo6mv2.h"
#include "st7789.h"
#include "colors.h"
#include "sd_card.h"

#include <stdio.h>          // Funções de entrada/saída (printf)
#include <string.h>         // Manipulação de strings (strtok, strncmp)
#include "pico/stdlib.h"    // SDK da Raspberry Pi Pico


// Variáveis globais
static double last_lat = 0.0;  // Armazena a última latitude válida
static double last_lon = 0.0;  // Armazena a última longitude válida  
static bool have_fix = false;  // Flag: true quando GPS tem sinal válido


int main() {
    stdio_init_all();  // Inicializa stdio (USB serial para printf)
    
    // ### Inicialização do GPS ###
    setup_gps();
    // ### Inicialização do display ST7789 ###
    st7789_init(); 
    // Adiciona fundo branco a tela
    st7789_fill_screen(COLOR_WHITE);
    // ### Inicialização do SD ###
    printf("Inicializando SD Card...\n");
    init_spi_sdcard();
    sleep_ms(500);
    // #########################

    
    char line[128];    // Buffer para acumular uma linha NMEA (até 127 chars + \0)
    int idx = 0;       // Índice atual no buffer
    absolute_time_t last_print = get_absolute_time();  // Timestamp último print
    
    printf("Iniciando leitura GPS");  // Mensagem inicial

    char buffer_lat[22];  // Para armazenar latitude
    char buffer_long[23]; // Para armazenar longitude

    // Mensagens fixas no dispĺay
    draw_centered_text("Monitoramento", 50, COLOR_BLUE, COLOR_WHITE, 3);
    draw_centered_text("da Posicao", 80, COLOR_BLUE, COLOR_WHITE, 3);
    
    while (1) {
        // Processa todos os caracteres disponíveis na UART
        while (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);  // Lê um caractere
            
            if (c == '\r') continue;  // Ignora carriage return (\r)
            
            if (c == '\n') {  // Fim de linha encontrado
                line[idx] = '\0';  // Termina string com null
                
                // Faz cópia porque strtok modifica a string original
                char copy[128];
                strncpy(copy, line, sizeof(copy));
                copy[127] = '\0';  // Garante terminação
                
                // Identifica tipo de sentença
                if (strncmp(copy, "$GPRMC", 6) == 0) { // Se é RMC, chama parse_gprmc
                    double lat, lon;
                    // Se o parser retornar true, atualiza a última posição e marca have_fix = true
                    if (parse_gprmc(copy, &lat, &lon)) {
                        last_lat = lat; last_lon = lon; have_fix = true;
                    }
                } else if (strncmp(copy, "$GPGGA", 6) == 0) { // Se é GGA, chama parse_gpgga
                    double lat, lon;
                    // Se o parser retornar true, atualiza a última posição e marca have_fix = true
                    if (parse_gpgga(copy, &lat, &lon)) {
                        last_lat = lat; last_lon = lon; have_fix = true;
                    }
                }
                
                idx = 0;  // Reseta buffer para próxima linha
            } else {
                // Adiciona caractere ao buffer, prevenindo overflow
                if (idx < 127) line[idx++] = c;
                else idx = 0;  // Overflow --> descarta linha
            }
        }
        
        // Verifica se passaram 1 segundos desde último print
        if (absolute_time_diff_us(last_print, get_absolute_time()) >= 1000000) {
            last_print = get_absolute_time();  // Atualiza timestamp
            
            if (have_fix) { // Se fix válido, imprime a última lat/lon com 6 casas decimais
                // Formatação das strings
                snprintf(buffer_lat, sizeof(buffer_lat), "Latitude: %.6f", last_lat);
                snprintf(buffer_long, sizeof(buffer_long), "Longitude: %.6f", last_lon);

                printf("%s | %s\n", buffer_lat, buffer_long);

                // Escreve os dados de localização no Display
                draw_centered_text(buffer_lat, 130, COLOR_GRAY, COLOR_WHITE, 2);
                draw_centered_text(buffer_long, 160, COLOR_GRAY, COLOR_WHITE, 2);

                // ### Escreve os dados de localização no sd
                write_to_sd(last_lat, last_lon);
                // ### Lendo dados de localização no SD
                read_from_sd();

            } else { // Caso contrário, avisa que ainda não há fix.
                printf("Sem fix GPS ainda (aguardando satélites)...\n");
            }
        }
        
        sleep_ms(10);  // Pequena pausa para reduzir consumo de CPU
    }
    
    return 0;  
}