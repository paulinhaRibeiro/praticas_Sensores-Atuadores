#ifndef COLORS_H
#define COLORS_H

// ==========================
// Macros de cor RGB565
// ==========================
/* Cada pixel ocupa 16 bits:
        Vermelho: 5 bits (0–31) --> deslocado para os bits 15–11
        Verde: 6 bits (0–63) --> deslocado para os bits 10–5
        Azul: 5 bits (0–31) --> fica nos bits 4–0
   As máscaras usadas:
        0x1F = 0001 1111 (5 bits)
        0x3F = 0011 1111 (6 bits)
*/
#define RGB565(r,g,b) (~(((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F)) & 0xFFFF)

// ==========================
// Cores predefinidas (8 bits escalados para 5/6 bits)
// ==========================
#define COLOR_RED     RGB565(31, 0, 0)       // 255,0,0
#define COLOR_GREEN   RGB565(0, 63, 0)       // 0,255,0
#define COLOR_BLUE    RGB565(0, 0, 31)       // 0,0,255
#define COLOR_YELLOW  RGB565(31, 63, 0)      // 255,255,0
#define COLOR_CYAN    RGB565(0, 63, 31)      // 0,255,255
#define COLOR_MAGENTA RGB565(31, 0, 31)      // 255,0,255
#define COLOR_WHITE   RGB565(31, 63, 31)     // 255,255,255
#define COLOR_BLACK   RGB565(0, 0, 0)        // 0,0,0
#define COLOR_GRAY    RGB565(6, 12, 6)       // ≈50,50,50 em 5/6 bits


#endif