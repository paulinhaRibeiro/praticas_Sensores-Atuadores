#include "hardware/i2c.h" // Inclui biblioteca de hardware I2C para comunicação com o sensor
#include "sensor_VL53L0X.h"


// Definições de I2C
#define I2C_PORT i2c0 // Define a porta I2C usada
#define SDA_PIN 0     // Pino de dados
#define SCL_PIN 1     // Pino de clock
// Endereço I2C do sensor VL53L0X
#define VL53L0X_ADDR 0x29

// Registradores do Sensor
#define REG_IDENTIFICATION_MODEL_ID 0xC0 // Registrador que contém o ID do modelo do sensor
#define REG_SYSRANGE_START 0x00          // Registrador para iniciar medição de distância
#define REG_RESULT_RANGE_STATUS 0x14     // Registrador que indica status da medição
#define REG_RESULT_RANGE_MM 0x1E         // Registrador onde a distância medida é armazenada


// inicialização e configuração do I2C
int config_i2c()
{
    // Inicializa I2C na frequência de 100kHz
    i2c_init(I2C_PORT, 100 * 1000);
    // Configura os pinos SDA e SCL para função I2C
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    // Ativa resistores de pull-up nos pinos I2C
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}

// Inicialização do sensor
int vl53l0x_init()
{
    // Define variável com endereço do registrador de ID
    uint8_t reg = REG_IDENTIFICATION_MODEL_ID;
    // Declara variável para armazenar ID lido
    uint8_t id;

    // Escreve no sensor pedindo para ler o registrador de ID - Se falhar, retorna 0 (erro)
    if (i2c_write_blocking(I2C_PORT, VL53L0X_ADDR, &reg, 1, true) != 1)
        return 0;

    //  Lê o ID do sensor (1 byte) - Se falhar, retorna 0 (erro)
    if (i2c_read_blocking(I2C_PORT, VL53L0X_ADDR, &id, 1, false) != 1)
        return 0;

    // Verifica se ID é 0xEE (ID correto do VL53L0X)
    if (id != 0xEE)
    {
        printf("ID inválido: 0x%02X (esperado: 0xEE)\n", id);
        // Se for diferente, imprime erro e retorna 0
        return 0;
    }
    // Se tudo ok, retorna 1 (sucesso)
    return 1;
}


// Leitura da distância
int vl53l0x_read_distance_mm()
{
    // Cria comando para iniciar medição
    uint8_t cmd[2] = {REG_SYSRANGE_START, 0x01};
    // Envia comando ao sensor para iniciar medição única
    if (i2c_write_blocking(I2C_PORT, VL53L0X_ADDR, cmd, 2, false) != 2)
        return -1;

    // Loop por até 100 tentativas (500ms máximo)
    for (int i = 0; i < 100; i++)
    {
        // Prepara para ler registrador de status
        uint8_t reg = REG_RESULT_RANGE_STATUS;
        uint8_t status;

        // Lê status da medição do sensor
        if (i2c_write_blocking(I2C_PORT, VL53L0X_ADDR, &reg, 1, true) != 1)
            return -1;
        if (i2c_read_blocking(I2C_PORT, VL53L0X_ADDR, &status, 1, false) != 1)
            return -1;

        // Verifica bit 0 do status (1 = medição pronta)
        if (status & 0x01)
            break;
        // Espera 5ms se medição não estiver pronta
        sleep_ms(5);
    }

    // Prepara para ler registrador de distância
    uint8_t reg = REG_RESULT_RANGE_MM;
    // Buffer para armazenar 2 bytes da distância
    uint8_t buffer[2];

    // Lê 2 bytes contendo a distância medida
    if (i2c_write_blocking(I2C_PORT, VL53L0X_ADDR, &reg, 1, true) != 1)
        return -1;
    if (i2c_read_blocking(I2C_PORT, VL53L0X_ADDR, buffer, 2, false) != 2)
        return -1;

    // Combina os 2 bytes em um valor de 16 bits (distância em mm)
    return (buffer[0] << 8) | buffer[1];
}


// Verifica se a distância é valida
int distancia_valida(int distancia)
{
    // Verifica se distância está na faixa operacional válida
    return (distancia >= DISTANCIA_MINIMA_VALIDA &&
            distancia <= DISTANCIA_MAXIMA_VALIDA);
}