#ifndef SENSOR_AHT10
#define SENSOR_AHT10

#include <stdio.h> // biblioteca padrão para funções de entrada e saída
#include "pico/stdlib.h" // biblioteca padrão do Raspberry Pi Pico, com funcionalidades para GPIO, timers
#include "hardware/i2c.h" // biblioteca para comunicação I2C com periféricos

extern void setup_aht10();
extern void aht10_config();
extern void aht10_reset();
extern bool aht10_read(float *temperature, float *humidity);

#endif
