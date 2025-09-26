#include "sensor_AHT10.h"

#define I2C_PORT i2c0       // Define que a porta I2C usada será a i2c0
#define I2C_BAUDRATE 100000 // Define a frequência do I2C: 100 kHz (padrão)
#define SDA_PIN 0           // Define o pino GPIO 0 para a linha de dados (SDA) do barramento I2C
#define SCL_PIN 1           // Define o pino GPIO 1 para o clock (SCL) do barramento I2C
//
#define AHT10_ADDRESS 0x38          // Define o endereço I2C do sensor AHT10 (0x38 é o endereço padrão deste sensor)
#define AHT10_CMD_INITIALIZE 0xE1   // Define o comando para inicialização/calibração
#define AHT10_CMD_MEASURE 0xAC      // Define o comando para iniciar a medição no sensor
#define AHT10_CMD_SOFT_RESET 0xBA   // Define o comando para resetar o sensor - "soft reset" (reinicialização via software)
#define AHT10_STATUS_BUSY_MASK 0x80 // Define a máscara para verificar se o sensor está ocupado
#define AHT10_STATUS_CAL_MASK 0x08  // Define a máscara para verificar se o sensor está calibrado

// Função de inicialização do AHT10
void aht10_config()
{
    /*  Cria um array de 3 bytes com os comandos de inicialização:
            0xE1: Comando de inicialização.
            0x08: Parâmetro de configuração (ativa o sensor, modo normal).
            0x00: para completar o protocolo.*/
    uint8_t init_cmd[3] = {AHT10_CMD_INITIALIZE, AHT10_STATUS_CAL_MASK, 0x00};
    /*  Envia os comandos para o AHT10 usando I2C:
            I2C_PORT: Porta I2C usada (i2c0).
            AHT10_ADDRESS: Endereço do sensor.
            init_cmd: Array de comandos.
            3: Quantidade de bytes a serem enviados.
            false: Indica que a comunicação é finalizada (sem comunicação contínua).*/
    int ret = i2c_write_blocking(I2C_PORT, AHT10_ADDRESS, init_cmd, 3, false);
    // Verifica se não deu erro ao inicializar/calibrar o sensor
    if (ret == PICO_ERROR_GENERIC)
    {
        printf("Erro ao escrever comando de inicializacao para AHT10.\n");
        return;
    }
    sleep_ms(100); // Aguarda 100ms para o sensor concluir a inicialização

    // Verifica o estado de calibração
    uint8_t status;
    i2c_read_blocking(I2C_PORT, AHT10_ADDRESS, &status, 1, false);
    if (!(status & AHT10_STATUS_CAL_MASK))
    {
        printf("AHT10 NAO CALIBRADO! Tente reiniciar o sistema.\n");
    }
    else
    {
        printf("AHT10 inicializado e calibrado com sucesso.\n");
    }
}

// Função para Reseta o sensor AHT10
void aht10_reset()
{
    uint8_t reset_cmd = AHT10_CMD_SOFT_RESET;
    // Envia 1 byte (0xBA) para o endereço do sensor (AHT10_ADDRESS) na porta I2C_PORT - instrui o sensor a se reinicializar internamente.
    int ret = i2c_write_blocking(I2C_PORT, AHT10_ADDRESS, &reset_cmd, 1, false);
    // Caso o envio falhe, um erro é impresso no monitor serial
    if (ret == PICO_ERROR_GENERIC)
    {
        printf("Erro ao enviar comando de reset para AHT10.\n");
    }
    sleep_ms(20); // O sensor precisa de 20ms para completar o reset
}

// Função para leitura de temperatura/umidade e armazená-los nas variáveis fornecidas
bool aht10_read(float *temperature, float *humidity)
{
    /*  Cria um array de 3 bytes com os comandos de inicialização:
            0xAC: comando para iniciar a medição
            0x33: ativa medição de temperatura + umidade.
            0x00: byte de preenchimento (como no reset), exigido pelo protocolo*/
    uint8_t measure_cmd[3] = {AHT10_CMD_MEASURE, 0x33, 0x00};

    // Envia 3 bytes via I2C para o AHT10
    int ret = i2c_write_blocking(I2C_PORT, AHT10_ADDRESS, measure_cmd, 3, false);
    if (ret == PICO_ERROR_GENERIC) // Mostra erro e encerra a função
    {
        printf("Erro ao enviar comando de medicao para AHT10.\n");
        return false;
    }
    sleep_ms(100); // Aguarda 100ms para a medição ser concluída

    uint8_t status_byte;
    // Lê 1 byte - o status do sensor (para verificar se o sensor está ocupado).
    i2c_read_blocking(I2C_PORT, AHT10_ADDRESS, &status_byte, 1, false);

    // Verifica se o sensor ainda está ocupado
    if (status_byte & AHT10_STATUS_BUSY_MASK)
    {
        printf("AHT10 Ocupado, nao foi possivel ler os dados.\n");
        return false;
    }

    uint8_t data[6];
    /*  ===== Lê os dados brutos
            Byte 0: status
            Byte 1: umidade [19:12]
            Byte 2: umidade [11:4]
            Byte 3: umidade [3:0] + temperatura [19:16]
            Byte 4: temperatura [15:8]
            Byte 5: temperatura [7:0]*/
    ret = i2c_read_blocking(I2C_PORT, AHT10_ADDRESS, data, 6, false);
    if (ret == PICO_ERROR_GENERIC)
    {
        printf("Erro ao ler dados do AHT10.\n");
        return false;
    }

    /*raw_humidity: Converte os dados brutos para obter a umidade:
                - Concatena os 20 bits de umidade
                data[1] (MSB)
                data[2]
                data[3] (só os 4 bits mais altos --> por isso o >> 4)
                >> 4: Desloca 4 bits para a direita, como os 4 LSBs estão no data[3].*/
    uint32_t raw_humidity = (((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | data[3]) >> 4;

    /*raw_temperature: Converte os dados brutos para obter a temperatura:
                    data[3] (só os 4 bits mais baixos: & 0x0F)
                    data[4]
                    data[5]
                Isso forma os 20 bits da temperatura */
    uint32_t raw_temperature = ((uint32_t)data[3] & 0x0F) << 16 | ((uint32_t)data[4] << 8) | data[5];

    // ===== Conversão para valores reais
    /*      O valor bruto é dividido por 1048576.0 (2^20) para converter para a proporção decimal.
                    Umidade: multiplica-se por 100.0 para obter a porcentagem (%).
                    Temperatura: multiplica-se por 200.0 e subtrai 50.0 para obter a escala em Celsius.*/
    *humidity = ((float)raw_humidity / 1048576.0) * 100.0;
    *temperature = ((float)raw_temperature / 1048576.0) * 200.0 - 50.0;

    return true; // consegiu capturar os dados de temp e hum
}

// Função para configurar o barramento I2C
void setup_aht10()
{
    i2c_init(I2C_PORT, I2C_BAUDRATE); // Inicializa a I2C na porta especificada com frequência de 100 kHz
    // Configura os pinos 0 e 1 para a função I2C (SDA e SCL, respectivamente)
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    // Ativa o pull-up interno nos pinos SDA e SCL, garantindo o nível lógico alto em repouso.
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    aht10_reset(); // Tenta resetar o sensor
}
