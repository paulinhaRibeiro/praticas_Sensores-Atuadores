#ifndef ST7789_H 
#define ST7789_H


#include <stdint.h>


// ==========================
// Protótipos das funções
// ==========================
extern void st7789_init(void);
extern void st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
extern void st7789_fill_screen(uint16_t color);
extern void draw_centered_text(const char *txt, int y, uint16_t color, uint16_t bg, int scale);
extern void draw_bar(int x, int y, int w, int h, int percent, uint16_t color);

#endif
