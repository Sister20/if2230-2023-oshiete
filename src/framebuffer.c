#include "lib-header/framebuffer.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/portio.h"

static const size_t VGA_WIDTH = 80;                            // width
static const size_t VGA_HEIGHT = 25;                           // height
static uint16_t *framebuffer = (uint16_t *)MEMORY_FRAMEBUFFER; // type cast to uint16

void framebuffer_set_cursor(uint8_t r, uint8_t c)
{
    // TODO : Implement
    uint16_t pos = r * VGA_WIDTH + c;

    out(CURSOR_PORT_CMD, 14);                 
    out(CURSOR_PORT_DATA, pos >> 8);
    out(CURSOR_PORT_CMD, 15);              
    out(CURSOR_PORT_DATA, pos & 0xff);
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg)
{
    // TODO : Implement
    uint16_t* location = framebuffer + (row * VGA_WIDTH + col);
    uint16_t entry = (uint16_t) c | framebuffer_set_color(fg,bg) << 8;
    *location = entry;
    memset(location + 1, entry, 1);
}

void framebuffer_clear(void)
{
    // TODO : Implement
    uint16_t data = (uint16_t) ' ' << 8 | framebuffer_set_color(0,0); 
    size_t size = VGA_HEIGHT * VGA_WIDTH; 
    memset(framebuffer, data, size*2); 
}

uint8_t framebuffer_set_color(enum vga_color fg, enum vga_color bg)
{
    return fg | bg << 4;
};
