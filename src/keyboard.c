#include "lib-header/keyboard.h"
#include "lib-header/portio.h"
#include "lib-header/framebuffer.h"
#include "lib-header/stdmem.h"

static struct KeyboardDriverState keyboard_state = {
    FALSE,  // read_extended_mode
    TRUE,   // keyboard_input_on
    0,      // buffer_index
    {'\0'}, // keyboard_buffer
    FALSE,  // shift_pressed
    FALSE   // capslock_activated
};

const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

void keyboard_state_activate(void) {
    keyboard_state.keyboard_input_on = TRUE;
}

void keyboard_state_deactivate(void) {
    keyboard_state.keyboard_input_on = FALSE;
}

void get_keyboard_buffer(char *buf) {
    memcpy(buf, keyboard_state.keyboard_buffer, keyboard_state.buffer_index);
}

bool is_keyboard_blocking(void) {
    return keyboard_state.keyboard_input_on;
}

void keyboard_isr(void) {
    if (!keyboard_state.keyboard_input_on)
        keyboard_state.buffer_index = 0;
    else {
        uint8_t  scancode    = in(KEYBOARD_DATA_PORT);
        char     mapped_char = keyboard_scancode_1_to_ascii_map[scancode];

        // Get Cursor Position
        uint16_t currentPos = framebuffer_get_cursor();
        uint8_t row = currentPos / VGA_WIDTH, col = currentPos % VGA_WIDTH;

        // Action
        if (scancode == 0x2A || scancode == 0x36) {
            keyboard_state.shift_pressed = TRUE;
        } else if (scancode == 0xAA || scancode == 0xB6) {
            keyboard_state.shift_pressed = FALSE;
        } else if (scancode == 0x3A) {
            keyboard_state.capslock_activated = !keyboard_state.capslock_activated;
        } else if (mapped_char == '\0') {

        } else if (mapped_char == '\b') {
            uint8_t newRow, newCol;
            if (col == 0) {
                if (row == 0) {
                    newRow = 0, newCol = 0;
                } else {
                    newRow = row - 1, newCol = VGA_WIDTH - 1;
                    while (newCol > 0 && framebuffer_getchar(newRow, newCol - 1) == '\0') {
                        newCol--;
                    }
                }
            } else {
                newRow = row, newCol = col - 1;
            }
            framebuffer_set_cursor(newRow, newCol);
            framebuffer_write(newRow, newCol, '\0', DEFAULT_FG, DEFAULT_BG);
        } else if (mapped_char == '\n') {
            if (row == VGA_HEIGHT - 1) {
                framebuffer_scroll_down();
                framebuffer_set_cursor(row, 0);
            } else {
                framebuffer_set_cursor(row + 1, 0);
            }
            keyboard_state_deactivate();
        } else {
            if (mapped_char >= 'a' && mapped_char <= 'z' && (keyboard_state.shift_pressed ^ keyboard_state.capslock_activated)) {
                mapped_char = mapped_char + 'A' - 'a';
            }

            framebuffer_write(row, col, mapped_char, DEFAULT_FG, DEFAULT_BG);
            keyboard_state.keyboard_buffer[keyboard_state.buffer_index++] = mapped_char;

            if (row == VGA_HEIGHT - 1 && col == VGA_WIDTH - 1) {
                framebuffer_scroll_down();
                framebuffer_set_cursor(row, 0);
            } else {
                framebuffer_set_cursor(row, col + 1);
            }
        }
    }
    pic_ack(IRQ_KEYBOARD);
}

