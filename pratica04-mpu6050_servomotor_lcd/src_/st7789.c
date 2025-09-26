#include "st7789.h" // Cabeçalho - protótipos
#include "font.h"   // Tabela de fonte (glyphs) e dimensões (FONT_WIDTH/HEIGHT)
#include "colors.h" // Macros de cor 

#include <string.h> // usadas no texto
#include "pico/stdlib.h" // Para os GPIOs
#include "hardware/spi.h" // API de SPI do RP2040


// ==========================
// Pinos (SPI0)
// ==========================
#define PIN_MOSI 19 // GPIO 19 -> linha MOSI do SPI0 (Master Out, Slave In)
#define PIN_SCK  18 // GPIO 18 -> clock do SPI0
#define PIN_CS   17 // GPIO 17 -> Chip Select (CS) do display (ativo em nível baixo)
#define PIN_DC    4 // GPIO 4  -> Data/Command: 0=comando, 1=dados
#define PIN_RST  20 // GPIO 20 -> Reset do display (ativo em nível baixo)
#define PIN_BL    9 // GPIO 9  -> Backlight (luz de fundo)


// ==========================
// ST7789 - comandos e geometry
// ==========================
#define ST7789_WIDTH   320  // Largura lógica do frame buffer: 320 px
#define ST7789_HEIGHT  240  // Altura lógica do frame buffer: 240 px

#define ST7789_CASET   0x2A // Comando: Column Address Set (define coluna inicial/final)
#define ST7789_RASET   0x2B // Comando: Row Address Set (define linha inicial/final)
#define ST7789_RAMWR   0x2C // Comando: RAM Write (escrita de pixels)
#define ST7789_MADCTL  0x36 // Comando: Memory Data Access Control (orientação, ordem de bits)
#define ST7789_COLMOD  0x3A // Comando: Interface Pixel Format (profundidade de cor)
#define ST7789_SLPOUT  0x11 // Comando: Sleep Out (sai do modo de baixo consumo)
#define ST7789_DISPON  0x29 // Comando: Display ON

#define ST7789_MADCTL_VAL 0x60     // Valor do MADCTL para landscape 320x240


// ==========================
// GPIO helpers - Controlam seleção do display e modo comando/dado
// ==========================
static inline void st7789_select(void)   { gpio_put(PIN_CS, 0); }  // Baixa CS: seleciona o display
static inline void st7789_deselect(void) { gpio_put(PIN_CS, 1); }  // Sobe CS: deseleciona o display
static inline void st7789_dc_cmd(void)   { gpio_put(PIN_DC, 0); }  // DC=0: a próxima transferência é comando
static inline void st7789_dc_data(void)  { gpio_put(PIN_DC, 1); }  // DC=1: a próxima transferência é dados


// ==========================
// SPI helpers - Enviam comandos/dados.
// ==========================
static inline void st7789_write_cmd(uint8_t cmd) {   // Envia um único byte de comando
    st7789_select(); // Seleciona o periférico (CS baixo)
    st7789_dc_cmd();  // Informa que é comando (DC=0)
    spi_write_blocking(spi0, &cmd, 1);  // Transfere 1 byte via SPI0, bloqueante
    st7789_deselect();  // Libera o periférico (CS alto)
}


static inline void st7789_write_data(const uint8_t *data, size_t len) { // Envia um bloco de dados
    if (!len) return;  // Nada a enviar? sai
    st7789_select();  // Seleciona o display
    st7789_dc_data();  // DC=1 para dados
    spi_write_blocking(spi0, data, len); // Envia len bytes
    st7789_deselect(); // Termina a transação
}


static inline void st7789_write_data_byte(uint8_t d) { // Atalho para enviar 1 byte de dados
    st7789_write_data(&d, 1);
}



// Define área de desenho
static void st7789_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t d[4];  // Buffer temporário para 4 bytes (MSB, LSB)

    st7789_write_cmd(ST7789_CASET);  // Seleciona o registro de colunas
    d[0] = x0 >> 8; d[1] = x0 & 0xFF;  // Coluna inicial (MSB, LSB)
    d[2] = x1 >> 8; d[3] = x1 & 0xFF;  // Coluna final   (MSB, LSB)
    st7789_write_data(d, 4);    // Envia as 4 bytes de faixa de colunas

    st7789_write_cmd(ST7789_RASET); // Seleciona o registro de linhas
    d[0] = y0 >> 8; d[1] = y0 & 0xFF;  // Linha inicial (MSB, LSB)
    d[2] = y1 >> 8; d[3] = y1 & 0xFF; // Linha final (MSB, LSB)
    st7789_write_data(d, 4);   // Envia as 4 bytes de faixa de linhas

    st7789_write_cmd(ST7789_RAMWR);  // Prepara a RAM para escrita de pixels
}


// ==========================
// Inicializa o display e prepara para uso
// ==========================
void st7789_init(void) {
    gpio_init(PIN_CS);  gpio_set_dir(PIN_CS,  GPIO_OUT); gpio_put(PIN_CS, 1); // CS como saída, inicia alto (inativo)
    gpio_init(PIN_DC);  gpio_set_dir(PIN_DC,  GPIO_OUT);  // DC como saída
    gpio_init(PIN_RST); gpio_set_dir(PIN_RST, GPIO_OUT);  // RST como saída
    gpio_init(PIN_BL);  gpio_set_dir(PIN_BL,  GPIO_OUT);   // BL como saída

    spi_init(spi0, 40 * 1000 * 1000);  // Inicializa SPI0 a 40 MHz
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST); // 8 bits, modo 0, MSB primeiro
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);  // Configura GPIO MOSI para função SPI
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);  // Configura GPIO SCK  para função SPI

    gpio_put(PIN_RST, 0); sleep_ms(50);  // Reset físico do display (baixa RST por 50 ms)
    gpio_put(PIN_RST, 1); sleep_ms(50); // Libera reset e aguarda estabilizar

    gpio_put(PIN_BL, 1);  // Liga o backlight

    st7789_write_cmd(ST7789_SLPOUT); // Sai do modo sleep
    sleep_ms(120);  // Espera 120 ms (tempo recomendado pelo datasheet)

    st7789_write_cmd(ST7789_COLMOD); // Define formato de pixel
    st7789_write_data_byte(0x55);  // 0x55 = 16 bits/pixel (RGB565)

    st7789_write_cmd(ST7789_MADCTL); // Define orientação e ordem dos eixos
    st7789_write_data_byte(ST7789_MADCTL_VAL);  // Valor pré-definido para landscape 320x240

    st7789_write_cmd(ST7789_DISPON);  // Liga o display (saída de vídeo)
    sleep_ms(120);  // Tempo para a imagem aparecer
}


// ==========================
// Primitivas de desenho - Desenho de áreas sólidas
// ==========================
void st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;   // Fora da tela? nada a fazer
    if (x + w > ST7789_WIDTH)  w = ST7789_WIDTH  - x;  // Clipping na borda direita
    if (y + h > ST7789_HEIGHT) h = ST7789_HEIGHT - y;  // Clipping na borda inferior

    st7789_set_addr_window(x, y, x + w - 1, y + h - 1);   // Define a janela a preencher

    uint8_t px[2] = { (uint8_t)(color >> 8), (uint8_t)(color & 0xFF) }; // Cor em 2 bytes (MSB, LSB)
    const uint32_t total = (uint32_t)w * h;  // Número total de pixels a escrever

    uint8_t buf[512];  // Buffer de transmissão (512 bytes = 256 px)
    for (size_t i = 0; i < sizeof(buf); i += 2) { // Pré-carrega o buffer com a cor repetida
        buf[i]   = px[0];
        buf[i+1] = px[1];
    }

    st7789_select(); // Mantém CS baixo durante o streaming
    st7789_dc_data();  // Envia dados de pixel
    uint32_t remaining = total; // Pixels restantes a enviar
    while (remaining) {  // Envia em lotes do tamanho do buffer
        uint32_t batch = remaining > (sizeof(buf)/2) ? (sizeof(buf)/2) : remaining; // Até 256 px por lote
        spi_write_blocking(spi0, buf, batch * 2);  // Envia batch*2 bytes
        remaining -= batch;   // Atualiza restante
    }
    st7789_deselect();  // Termina a transação
}


void st7789_fill_screen(uint16_t color) { // Atalho: preenche a tela inteira
    st7789_fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}


// ==========================
// Texto - Escrita de texto com fonte 5x7
// ==========================
void st7789_draw_char(uint16_t x, uint16_t y, char c,
                      uint16_t color, uint16_t bg, uint8_t scale) {
    if (c < 32 || c > 126) return;  // Apenas caracteres imprimíveis da ASCII básica
    const uint8_t *glyph = font5x7[c - 32];  // Obtém ponteiro para a coluna 0 do glyph 5x7
    for (int col = 0; col < FONT_WIDTH; col++) {  // Varre colunas do glyph (largura = 5)
        uint8_t bits = glyph[col];   // Cada byte tem os 7 bits das linhas dessa coluna
        for (int row = 0; row < FONT_HEIGHT; row++) {  // Varre linhas (altura = 7)
            uint16_t px = (bits & (1 << row)) ? color : bg; // Testa bit: 1=cor do texto, 0=cor do fundo
            for (int dx = 0; dx < scale; dx++) { // Escala em X (repete pixels)
                for (int dy = 0; dy < scale; dy++) {  // Escala em Y
                    st7789_fill_rect(x + col*scale + dx,  // Desenha um pixel "escalado" como 1x1
                                     y + row*scale + dy,
                                     1, 1, px);
                }
            }
        }
    }
}


void st7789_draw_text(uint16_t x, uint16_t y, const char *text,
                      uint16_t color, uint16_t bg, uint8_t scale) {
    while (*text) {  // Percorre cada char da string
        st7789_draw_char(x, y, *text, color, bg, scale); // Desenha o caractere atual
        x += (FONT_WIDTH + 1) * scale; // Avança X (largura da fonte + 1 px de espaçamento)
        text++; // Próximo caractere
    }
}


void draw_centered_text(const char *txt, int y, uint16_t color, uint16_t bg, int scale) {
    int char_width = (FONT_WIDTH + 1) * scale;  // Largura efetiva por caractere
    int len = strlen(txt);  // Comprimento da string
    int total_width = len * char_width; // Largura total em pixels
    int x = (ST7789_WIDTH - total_width) / 2; // Calcula X centralizado na tela
    st7789_draw_text(x, y, txt, color, bg, scale);  // Desenha a string centrada
}


// Desenha barra de progresso com bordas
void draw_bar(int x, int y, int w, int h, int percent, uint16_t color) {
    st7789_fill_rect(x, y, w, h, COLOR_GRAY);  // Fundo da barra (cinza escuro)
    int filled = (w * percent) / 100;  // Quantos pixels preencher baseado na %
    st7789_fill_rect(x, y, filled, h, color);   // Parte preenchida com a cor dada
    st7789_fill_rect(x, y, w, 1, COLOR_WHITE); // Borda superior (linha branca)
    st7789_fill_rect(x, y+h-1, w, 1, COLOR_WHITE); // Borda inferior
    st7789_fill_rect(x, y, 1, h, COLOR_WHITE);  // Borda esquerda
    st7789_fill_rect(x+w-1, y, 1, h, COLOR_WHITE);  // Borda direita
}