#include "lib-header/framebuffer.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/portio.h"

static uint16_t *framebuffer = (uint16_t *)MEMORY_FRAMEBUFFER; // type cast to uint16

uint16_t framebuffer_get_cursor()
{
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
            framebuffer[index] = framebuffer_set_entry('\0', DEFAULT_FG, DEFAULT_BG);
        }
    }
}

uint16_t framebuffer_set_entry(char c, enum vga_color fg, enum vga_color bg)
{
    return (uint16_t)c | (uint16_t)(fg | bg << 4) << 8;
};

char framebuffer_getchar(uint8_t row, uint8_t col)
{
    return framebuffer[row * VGA_WIDTH + col] & 0xff;
}

void framebuffer_scroll_down()
{
    for (size_t i = 0; i < VGA_HEIGHT; i++)
    {
        for (size_t j = 0; j < VGA_WIDTH; j++)
        {
            if (i == VGA_HEIGHT - 1)
            {
                framebuffer_write(i, j, '\0', DEFAULT_FG, DEFAULT_BG);
            }
            else
            {
                framebuffer_write(i, j, framebuffer_getchar(i + 1, j), DEFAULT_FG, DEFAULT_BG);
            }
        }
    }
}

void puts_buff(const char *s, size_t length, enum vga_color fg)
{
    for (size_t i = 0; i < length; i++)
    {
        uint16_t cursor = framebuffer_get_cursor();
        uint8_t row, col;

        row = cursor / VGA_WIDTH;
        col = cursor % VGA_WIDTH;

        if (s[i] == '\n')
        {
            if (row == VGA_HEIGHT - 1)
            {
                framebuffer_scroll_down();
                framebuffer_set_cursor(row, 0);
            }
            else
            {
                framebuffer_set_cursor(row + 1, 0);
            }
        }
        else
        {
            framebuffer_write(row, col, s[i], fg, DEFAULT_BG);

            if (row == VGA_HEIGHT - 1 && col == VGA_WIDTH - 1)
            {
                framebuffer_scroll_down();
            }
            framebuffer_set_cursor(row, col + 1);
        }
    }
}