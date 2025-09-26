#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdio.h>

extern void init_spi_sdcard();
extern void write_to_sd(char *text);
extern void read_from_sd();

#endif