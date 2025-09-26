#ifndef SD_CARD_H
#define SD_CARD_H


extern void init_spi_sdcard();
extern void write_to_sd(double lat, double lon);
extern void read_from_sd();

#endif