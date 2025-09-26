#include "gps_gy-neo6mv2.h"
#include <stdio.h>          // Funções de entrada/saída (printf)
#include <string.h>         // Manipulação de strings (strtok, strncmp)
#include <stdlib.h>         // Funções utilitárias (atof, atoi)
#include "hardware/uart.h"  // Controle de UART da Pico


#define BAUD_RATE      9600  // Velocidade padrão do GPS NEO-6M
#define UART_TX_PIN    0     // GPIO0 como TX -> conecta ao RX do GPS
#define UART_RX_PIN    1     // GPIO1 como RX <- conecta ao TX do GPS


// Configura hardware UART
void setup_gps(){
    // Configura hardware UART
    uart_init(UART_ID, BAUD_RATE);  // Inicializa UART com baud rate
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);  // Configura pino como UART
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

// Conversão de coordenadas
double nmea_to_decimal(const char *s) { // NMEA fornece latitude/longitude como ddmm.mmmm (ou dddmm.mmmm para longitude)
    double v = atof(s);         // Converte string para número 
    int deg = (int)(v / 100.0); // Extrai graus
    double minutes = v - (deg * 100.0); // Extrai minutos
    return deg + minutes / 60.0; // Converte para decimal
}


// Parser da sentença $GPRMC
bool parse_gprmc(char *sentence, double *lat_out, double *lon_out) {
    char *fields[20];  // Array para armazenar os campos da sentença
    int n = 0;         // Contador de campos
    
    // Divide a string usando vírgulas como delimitadores
    char *tok = strtok(sentence, ",");
    while (tok != NULL && n < 20) {
        fields[n++] = tok;  // Armazena cada campo no array
        tok = strtok(NULL, ",");  // Pega próximo campo
    }
    
    if (n < 7) return false;  // Sentença muito curta -> inválida
    // --> fields: 0=$GPRMC 1=time 2=status 3=lat 4=N/S 5=lon 6=E/W
    // fields[2] = status ('A' = ativo/válido, 'V' = inválido)
    if (fields[2][0] != 'A') return false;
    
    // Verifica se campos de lat/lon não estão vazios
    if (fields[3][0] == '\0' || fields[5][0] == '\0') return false;
    
    // Converte e aplica sinal (N/S, E/W)
    double lat = nmea_to_decimal(fields[3]);
    if (fields[4][0] == 'S') lat = -lat;  // Sul --> negativo
    
    double lon = nmea_to_decimal(fields[5]);
    if (fields[6][0] == 'W') lon = -lon;  // Oeste --> negativo
    
    // Devolve os resultados por ponteiro e indica sucesso
    *lat_out = lat;
    *lon_out = lon;
    return true;
}


// Parser da sentença $GPGGA
bool parse_gpgga(char *sentence, double *lat_out, double *lon_out) {
    // Estrutura similar ao GPRMC mas com campos diferentes
    char *fields[20];
    int n = 0;
    char *tok = strtok(sentence, ",");
    while (tok != NULL && n < 20) {
        fields[n++] = tok;
        tok = strtok(NULL, ",");
    }
    
    if (n < 7) return false;
    // --> GPGGA fields: 0=$GPGGA 1=time 2=lat 3=N/S 4=lon 5=E/W 6=fixQuality (0=invalid)
    // fields[6] = qualidade do fix (0 = inválido, 1 = GPS, 2 = DGPS)
    int fix = atoi(fields[6]);
    if (fix == 0) return false;  // Sem fix válido
    
    if (fields[2][0] == '\0' || fields[4][0] == '\0') return false;
    
    // Conversão igual ao GPRMC mas com índices diferentes
    double lat = nmea_to_decimal(fields[2]);
    if (fields[3][0] == 'S') lat = -lat;
    
    double lon = nmea_to_decimal(fields[4]);
    if (fields[5][0] == 'W') lon = -lon;
    
    *lat_out = lat;
    *lon_out = lon;
    return true;
}