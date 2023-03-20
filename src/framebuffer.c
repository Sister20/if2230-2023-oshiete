#include "lib-header/framebuffer.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/portio.h"

static const size_t VGA_WIDTH = 80;                            // width
static const size_t VGA_HEIGHT = 25;                           // height
static uint16_t *framebuffer = (uint16_t *)MEMORY_FRAMEBUFFER; // type cast to uint16

uint16_t framebuffer_get_cursor() {
    uint16_t currentPos;
    out(CURSOR_PORT_CMD, 14);
    currentPos = in(CURSOR_PORT_DATA) << 8;
    out(CURSOR_PORT_CMD, 15);
    currentPos |= in(CURSOR_PORT_DATA);

    return currentPos;
}

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
    uint16_t *location = framebuffer + (row * VGA_WIDTH + col);
    uint16_t entry = framebuffer_set_entry(c, fg, bg);
    *location = entry;
}

void framebuffer_clear(void)
{
    for (size_t row = 0; row < VGA_HEIGHT; row++)
    {
        for (size_t col = 0; col < VGA_WIDTH; col++)
        {
            const size_t index = row * VGA_WIDTH + col;
            framebuffer[index] = framebuffer_set_entry(' ', VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        }
    }
}

uint16_t framebuffer_set_entry(char c, enum vga_color fg, enum vga_color bg)
{
    return (uint16_t)c | (uint16_t)(fg | bg << 4) << 8;
};
