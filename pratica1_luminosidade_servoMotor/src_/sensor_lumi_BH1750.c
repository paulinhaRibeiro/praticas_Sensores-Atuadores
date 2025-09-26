#include "sensor_lumi_BH1750.h"
#include "pico/stdlib.h" // inclui funções de GPIO, UART, timers
#include "hardware/i2c.h" // Biblioteca para manipulação de I2C (Inter-Integrated Circuit): protocolo de comunicação usado para conectar BH1750.
#include <stdio.h> // Biblioteca padrão do C para entrada e saída de dados


#define I2C_PORT i2c0 // barramento I2C utilizado
// Definem os pinos GPIO para comunicação I2C
#define SDA_PIN 0 // GPIO0 para Serial Data Line (dados)
#define SCL_PIN 1 // GPIO1 para Serial Clock Line (clock)

// ====== Definições do Sensor BH1750
#define BH1750_ADDR 0x23      // Endereço I2C do sensor de luminosidade BH1750
#define BH1750_CMD_START 0x10 // Comando para iniciar a medição contínua em alta resolução

// ===== Função para iniciar a medição do BH1750
void bh1750_start_measurement()
{ // Cria o comando (cmd) de 8 bits (uint8_t) com o valor "0x10" para iniciar a medição
    uint8_t cmd = BH1750_CMD_START;

    /*  i2c_write_blocking(): Escreve o comando para o sensor
            - I2C_PORT: Porta I2C selecionada
            - BH1750_ADDR: Endereço do sensor
            - &cmd: Ponteiro para o comando
            - 1: Quantidade de bytes (no caso, apenas 1)
            - false: Indica que não será realizada uma leitura logo após a escrita*/
    i2c_write_blocking(I2C_PORT, BH1750_ADDR, &cmd, 1, false);
}

// ===== Função para lê o valor da luminosidade do BH1750
int bh1750_read_lux()
{ // Cria um array de 2 bytes (data[2]) para armazenar o valor lido
    uint8_t data[2];
    /*  i2c_read_blocking(): Lê os dados do sensor
            I2C_PORT: Porta I2C
            BH1750_ADDR: Endereço do sensor
            data: Ponteiro para armazenar os dados lidos
            2: Quantidade de bytes a serem lidos
            false: Não será feita uma "repetição de start" (start repeated)*/
    i2c_read_blocking(I2C_PORT, BH1750_ADDR, data, 2, false);
    /*O valor é lido em dois bytes e juntado em um uint16_t de 16 bits.
        A operação (data[0] << 8) | data[1] faz um "shift" de 8 bits no primeiro byte (MSB)
        e junta com o segundo byte (LSB).*/
    return (data[0] << 8) | data[1];
}

void setup_BH1750()
{
    // Inicializa o I2C para comunicação com o BH1750
    i2c_init(I2C_PORT, 100 * 1000); // Frequência de 100 kHz
    // Configura os pinos SDA e SCL como I2C
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    // Ativa o pull-up dos pinos SDA e SCL
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}