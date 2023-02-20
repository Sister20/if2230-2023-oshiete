#include "lib-header/framebuffer.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/portio.h"

static const size_t VGA_WIDTH = 80;                            // width
static const size_t VGA_HEIGHT = 25;                           // height
static uint16_t *framebuffer = (uint16_t *)MEMORY_FRAMEBUFFER; // type cast to uint16

// static size_t framebuffer_row;
// static size_t framebuffer_col;
// static uint8_t framebuffer_color;

// void framebuffer_set_cursor(uint8_t r, uint8_t c)
// {
//     // TODO : Implement
// }

// void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg)
// {
//     // TODO : Implement
// }


void framebuffer_clear(void)
{
    // TODO : Implement
    memset(framebuffer, framebuffer_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), VGA_WIDTH * VGA_HEIGHT * sizeof(char));
}
