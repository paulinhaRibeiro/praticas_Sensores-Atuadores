#include "sd_card.h"
#include <stdio.h> // Funções de entrada/saída 
#include <string.h> // Funções de manipulação de strings (
#include "pico/stdlib.h" // Biblioteca padrão do Raspberry Pi Pico
#include "hardware/spi.h" // Driver SPI do Pico
#include "ff.h" // biblioteca FatFs para sistemas de arquivos


// SPI Pins
#define SD_PIN_MISO 16 // MISO - Master In Slave Out - dados do SD card para Pico
#define SD_PIN_CS   28 // CS   - Chip Select - seleciona o dispositivo SPI
#define SD_PIN_SCK  18 // SCK  - Clock - sinal de sincronização
#define SD_PIN_MOSI 19 // MOSI - Master Out Slave In - dados do Pico para SD card

// Nome do arquivo no SD Card
#define FILENAME "dista.txt"


// Variáveis globais do sistema de arquivos
FATFS fs;  // Estrutura que representa o sistema de arquivos FAT
FIL fil;   // Estrutura que representa um arquivo aberto


// Função de Timestamp (get_fattime)
DWORD get_fattime(void) {
    return ((DWORD)(2025 - 1980) << 25) // Ano desde 1980 (2025-1980 = 45)
         | ((DWORD)7 << 21) // Mês (julho)
         | ((DWORD)9 << 16) // Dia (9)
         | ((DWORD)16 << 11) // Hora (16h)
         | ((DWORD)45 << 5) // Minuto (45)
         | ((DWORD)0 >> 1); // Segundos/2 (0)
}


// Inicialização do SPI 
void init_spi_sdcard() {
    spi_init(spi0, 1000 * 1000); // Inicializa SPI0 a 1MHz 
    // Configura pinos MISO, SCK e MOSI como função SPI
    gpio_set_function(SD_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SD_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SD_PIN_MOSI, GPIO_FUNC_SPI);

    // Configura pino CS como saída digital
    gpio_init(SD_PIN_CS);
    // Coloca CS em nível alto (1)
    gpio_set_dir(SD_PIN_CS, GPIO_OUT);
    gpio_put(SD_PIN_CS, 1); // Deseleciona SDCard

    printf("SPI SDCard initialized\n");
}


// Escrita no SD Card
void write_to_sd(char *text) {
    FRESULT fr;

    fr = f_mount(&fs, "", 1); // Monta o sistema de arquivos
    if (fr != FR_OK) {
        printf("\nFalha ao montar o SD Card (erro: %d)\n", fr);
        return;
    }

    /* Abre arquivo com flags:
            FA_WRITE: Permite escrita
            FA_OPEN_ALWAYS: Abre se existir, cria se não existir
    */
    fr = f_open(&fil, FILENAME, FA_WRITE | FA_OPEN_ALWAYS);
    if (fr == FR_OK) {
        f_lseek(&fil, f_size(&fil)); // Posiciona cursor no final do arquivo (modo append)


        // Bytes escritos 
        UINT bw;
        // Escreve dados no arquivo
        fr = f_write(&fil, text, strlen(text), &bw);

        // Verifica se escrita foi bem-sucedida
        if (fr == FR_OK && bw == strlen(text)) {
            printf("\nDados escritos (%u bytes)\n", bw);
        } else {
            printf("\nErro ao escrever (erro: %d)\n", fr);
        }
        // Fecha o arquivo
        f_close(&fil);
    } else {
        printf("\nErro ao abrir arquivo (erro: %d)\n", fr);
    }
}


// Leitura do SD Card
void read_from_sd() { // Abre arquivo em modo leitura (FA_READ)
    FRESULT fr;
    char buffer[128]; // Buffer de 128 bytes para leitura
    UINT br;

    fr = f_open(&fil, FILENAME, FA_READ);
    if (fr != FR_OK) {
        printf("\nFalha ao abrir arquivo para leitura (erro: %d)\n", fr);
        return;
    }

    printf("\nConteúdo de de %s:\n", FILENAME);
    //  Lê em chunks de 127 bytes (deixa 1 byte para '\0')
    do {
        // Lê dados do arquivo
        fr = f_read(&fil, buffer, sizeof(buffer) - 1, &br);
        if (fr != FR_OK) {
            printf("\nErro ao ler arquivo (erro: %d)\n", fr);
            break;
        }
        buffer[br] = '\0'; // Adiciona terminador nulo para imprimir como string
        printf("%s", buffer);
    } while (br == sizeof(buffer) - 1); // enquanto ler quantidade máxima de bytes

    // Fecha o arquivo após leitura
    f_close(&fil);
}
