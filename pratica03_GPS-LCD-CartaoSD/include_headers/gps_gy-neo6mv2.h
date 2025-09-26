#ifndef GPS_GY_NEO6MV2
#define GPS_GY_NEO6MV2

#include "pico/stdlib.h"

#define UART_ID uart0 // Usa a UART0 da Pico

extern void setup_gps();
extern bool parse_gprmc(char *sentence, double *lat_out, double *lon_out);
extern bool parse_gpgga(char *sentence, double *lat_out, double *lon_out);
#endif



























// #include "pico/stdlib.h" // Biblioteca padrão para controle de GPIO, UART, timers e outras funções básicas do Raspberry Pi Pico.
// #include "hardware/uart.h" // Biblioteca específica para controlar a interface UART do RP2040.
// #include <stdio.h> // Biblioteca padrão de entrada e saída (como printf).
// #include <string.h> // Contém funções para manipulação de strings (como strcpy, strtok).
// #include <stdlib.h> // Fornece conversões de string para número (como atoi, atof).

// // Definições da UART e pinos
// #define GPS_UART_ID uart1 // Identifica qual UART será usada (o RP2040 tem uart0 e uart1).
// #define GPS_BAUD_RATE 9600 // Define a taxa de transmissão de dados (baud rate) para 9600 bits por segundo (padrão para o GY-NEO6MV2)
// #define GPS_TX_PIN 5 // Define o pino de transmissão (TX) para o módulo GPS
// #define GPS_RX_PIN 4 // Define o pino de recepção (RX) para o módulo GPS

// // // Buffer para armazenar a sentença NMEA
// // char nmea_buffer[256]; // buffer (área de memória) para armazenar os dados que chegam do módulo GPS em formato de string
// // volatile absolute_time_t last_data_time; // Guarda o tempo em que o último dado foi recebido, para monitorar "sinal perdido"


// // Declaração externa (extern) para ser visível em main.c
// extern volatile absolute_time_t last_data_time;


// // Inicialização 
// extern void setup_gps();
// // Função para analisar os dados do GPS
// extern void parse_gps_data(char *nmea_sentence);
// extern float convert_to_decimal(const char *coordinate, char direction);




















// #ifndef GPS_H
// #define GPS_H

// // Funções para conversão e análise dos dados do GPS
// float convert_to_decimal(const char *coordinate, char direction);
// void parse_gps_data(char *nmea_sentence);

// #endif