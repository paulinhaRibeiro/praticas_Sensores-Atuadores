#ifndef SENSOR_MPU6050
#define SENSOR_MPU6050

#include "pico/stdlib.h" // Biblioteca padrão para GPIO, UART, timers
#include "hardware/i2c.h" // Biblioteca para comunicação I2C com periféricos
#include <math.h> // Biblioteca matemática para funções como `sqrt` e `atan2`
#include <stdio.h> // Biblioteca padrão de entrada/saída (input/output)

extern void setup_mpu6050();
extern void inicializar_mpu6050();
extern void ler_acelerometro_gyro(int16_t *acel_x, int16_t *acel_y, int16_t *acel_z,
                                  int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z);
extern void calcular_inclinacao(int16_t acel_x, int16_t acel_y, int16_t acel_z, float *pitch, float *roll);
// extern float filtrar_dado(float novo_valor, float *historico, int tamanho);
#endif