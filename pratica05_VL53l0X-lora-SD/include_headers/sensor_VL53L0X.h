#ifndef SENSOR_VL53L0X_H
#define SENSOR_VL53L0X_H


#include "pico/stdlib.h"  // Inclui biblioteca para funções básicas
#include <stdio.h>        // Inclui biblioteca padrão de entrada/saída


// Limites de Distância - Define faixa válida de operação do sensor (50mm a 2000mm)
#define DISTANCIA_MINIMA_VALIDA 50   // 50 mm
#define DISTANCIA_MAXIMA_VALIDA 2000 // 2000 mm (2 metros)


// inicialização e configuração do I2C
extern int config_i2c();
// Inicialização do sensor
extern int vl53l0x_init();
// Leitura da distância
extern int vl53l0x_read_distance_mm();
// Verifica se a distância é valida
extern int distancia_valida(int distancia);

#endif