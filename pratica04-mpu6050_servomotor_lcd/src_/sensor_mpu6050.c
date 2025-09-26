#include "sensor_mpu6050.h"

// ======== Definições do sensor ========
#define MPU6050_I2C_PORT i2c0       // Define o barramento I2C utilizado (i2c0).
#define PINO_SDA 0                  // Pino de comunicação I2C (SDA)
#define PINO_SCL 1                  // Pino de comunicação I2C (SCL)
#define MPU6050_I2C_BAUDRATE 100000 // Define a velocidade da comunicação I2C (100kHz é padrão)

#define MPU6050_ADDRESS 0x68      // Define o endereço I2C do MPU6050 (0x68 é o valor padrão)
#define MPU6050_PWR_MGMT_1 0x6B   // Define o endereço do registrador de gerenciamento de energia.
#define MPU6050_ACCEL_XOUT_H 0x3B // Define o endereço inicial do registrador de dados do acelerômetro
#define ACCEL_SCALE_FACTOR 16384.0f // Define o fator de escala para converter a leitura bruta para aceleração em "g".
#define ANGULO_ALERTA_GRAUS 30.0f // Define o valor limite de inclinação para alertas.

// Função de Configuração do MPU6050
void setup_mpu6050()
{
  i2c_init(MPU6050_I2C_PORT, MPU6050_I2C_BAUDRATE); // Inicializa o barramento I2C0 a 100kHz (velocidade padrão)
  gpio_set_function(PINO_SDA, GPIO_FUNC_I2C);   // Define o pino SDA como I2C
  gpio_set_function(PINO_SCL, GPIO_FUNC_I2C);   // Define o pino SCL como I2C
}

// Função de Inicialização do MPU6050
void inicializar_mpu6050()
{
  /* Array reset:
        0x6B: Endereço do registrador de gerenciamento de energia.
        0x00: Valor 0x00 para "acordar" o sensor (sai do modo de sleep padrão)*/
  uint8_t reset[2] = {MPU6050_PWR_MGMT_1, 0x00};
  // Escreve os dois bytes para o sensor no endereço 0x68
  // false indica que a comunicação termina após essa escrita
  int ret = i2c_write_blocking(MPU6050_I2C_PORT, MPU6050_ADDRESS, reset, 2, false);

  if (ret == PICO_ERROR_GENERIC) // Verifica se houve erro na comunicação.
  {
    printf("Erro ao acordar MPU6050! Verifique conexoes e endereco I2C.\n");
  }
  else
  {
    printf("MPU6050 acordado e inicializado com sucesso.\n");
  }
  sleep_ms(100); // Aguarda 100ms para garantir estabilidade após acordar o sensor
}


//  Função para Leitura do MPU6050 (Acelerômetro e Giroscópio)
void ler_acelerometro_gyro(int16_t *acel_x, int16_t *acel_y, int16_t *acel_z,
                           int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z)
{
  /* 0 a 5  -> Aceleração (X, Y, Z) → 2 bytes para cada eixo.
     8 a 13 -> Giroscópio (X, Y, Z) → 2 bytes para cada eixo.*/
  uint8_t buffer[14]; // armazena os 14 bytes lidos do sensor
  uint8_t reg_addr = MPU6050_ACCEL_XOUT_H; // Define o endereço de início da leitura

  // ====== Envia o endereço do registrador e mantém a conexão I2C ativa (repeated start = true)
  /* i2c_write_blocking:
        i2c0: o barramento I2C usando.
        MPU6050_ADDRESS: endereço I2C do MPU6050 (0x68).
        0x3B: array temporário com o valor 0x3B. Esse valor é o endereço do registrador de início de leitura no MPU6050.
        1: quantidade de bytes enviados (apenas 0x3B).
        true: mantém a comunicação aberta (repeated start) para a próxima leitura*/
  int ret = i2c_write_blocking(MPU6050_I2C_PORT, MPU6050_ADDRESS, &reg_addr, 1, true);
  if (ret == PICO_ERROR_GENERIC) // Se houver erro de leitura, todos os valores são zerados
  {
    printf("Erro ao solicitar leitura de dados do MPU6050.\n");
    // Em caso de erro, zera os arrays para evitar dados inválidos
    *acel_x = 0;
    *acel_y = 0;
    *acel_z = 0;
    *gyro_x = 0;
    *gyro_y = 0;
    *gyro_z = 0;
    return;
  }

  // ====== Leitura dos 14 bytes de dados do sensor
  /* i2c_read_blocking:
        buffer: onde os dados lidos serão armazenados.
        14: quantidade de bytes a serem lidos.
        false: indica que a comunicação pode ser finalizada após a leitura*/
  ret = i2c_read_blocking(MPU6050_I2C_PORT, MPU6050_ADDRESS, buffer, 14, false);
  if (ret == PICO_ERROR_GENERIC) // Se houver erro de leitura, todos os valores são zerados
  {
    printf("Erro ao solicitar leitura de dados do MPU6050.\n");
    // Em caso de erro, zera os arrays para evitar dados inválidos
    *acel_x = 0;
    *acel_y = 0;
    *acel_z = 0;
    *gyro_x = 0;
    *gyro_y = 0;
    *gyro_z = 0;
    return;
  }

  // ====== Conversão dos Dados (Big Endian para Inteiro)
  // O MPU6050 envia os dados em formato Big Endian (byte mais significativo primeiro)
  *acel_x = (int16_t)((buffer[0] << 8) | buffer[1]); // Concatena os dois bytes do eixo X (MSB primeiro) e converte para int16_t.
  *acel_y = (int16_t)((buffer[2] << 8) | buffer[3]);
  *acel_z = (int16_t)((buffer[4] << 8) | buffer[5]);

  // A leitura de giroscópio começa a partir do buffer[8].
  *gyro_x = (int16_t)((buffer[8] << 8) | buffer[9]);
  *gyro_y = (int16_t)((buffer[10] << 8) | buffer[11]);
  *gyro_z = (int16_t)((buffer[12] << 8) | buffer[13]);
}

// ======== Função para Calcular a Inclinação ========
void calcular_inclinacao(int16_t acel_x, int16_t acel_y, int16_t acel_z, float *pitch, float *roll)
{
  // Converte os dados brutos para unidades de gravidade terrestre (g).
  float acel_x_g = acel_x / ACCEL_SCALE_FACTOR;
  float acel_y_g = acel_y / ACCEL_SCALE_FACTOR;
  float acel_z_g = acel_z / ACCEL_SCALE_FACTOR;

  // Fórmula para Pitch (inclinação para frente/trás) em graus
  // O pitch é calculado a partir da aceleração em X e da raiz quadrada das acelerações em Y e Z
  *pitch = atan2f(acel_x_g, sqrtf(acel_y_g * acel_y_g + acel_z_g * acel_z_g)) * 180.0 / M_PI;

  // Fórmula para Roll (inclinação para os lados) em graus
  // O roll é calculado com base em aceleração no eixo Y e a raiz quadrada das acelerações em X e Z
  *roll = atan2f(acel_y_g, sqrtf(acel_x_g * acel_x_g + acel_z_g * acel_z_g)) * 180.0f / M_PI;
}
