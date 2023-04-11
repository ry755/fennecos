#include <user/user.h>
#include <user/framebuffer.h>
#include <user/keyboard.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define FOREGROUND 0
#define BACKGROUND 1
#define CONSOLE_WIDTH 80
#define CONSOLE_HEIGHT 30
#define DEFAULT_FOREGROUND_COLOR 8
#define DEFAULT_BACKGROUND_COLOR 0
#define MAX_ESC_CODE_PARAMETERS 8

static char on_screen_buffer[CONSOLE_HEIGHT][CONSOLE_WIDTH];
static char on_screen_color_buffer[CONSOLE_HEIGHT][CONSOLE_WIDTH][2];
static char update_buffer[CONSOLE_HEIGHT][CONSOLE_WIDTH];
static char update_color_buffer[CONSOLE_HEIGHT][CONSOLE_WIDTH][2];
static int16_t console_x = 0;
static int16_t console_y = 0;
static uint8_t current_foreground_color_offset = DEFAULT_FOREGROUND_COLOR;
static uint8_t current_background_color_offset = DEFAULT_BACKGROUND_COLOR;

static uint8_t escape_code_parameters[MAX_ESC_CODE_PARAMETERS];
static uint8_t escape_code_parameter_count = 0;
static bool is_in_escape_code = false;

font_t *global_font = (void *) 0xFF000000;

static uint32_t colors[9] = {
    0xFF1E1E2E, // black
    0xFFF38BA8, // red
    0xFFA6E3A1, // green
    0xFFF9E2AF, // yellow
    0xFF89B4FA, // blue
    0xFFEBA0AC, // magenta
    0xFF94E2D5, // cyan
    0xFFCDD6F4, // white
    0xFFCDD6F4  // default
};

void redraw_console() {
    uint8_t foreground_color_offset;
    uint8_t background_color_offset;
    for (uint8_t y = 0; y < CONSOLE_HEIGHT; y++) {
        for (uint8_t x = 0; x < CONSOLE_WIDTH; x++) {
        if ((on_screen_buffer[y][x] != update_buffer[y][x]) ||
            (on_screen_color_buffer[y][x][FOREGROUND] != update_color_buffer[y][x][FOREGROUND]) ||
            (on_screen_color_buffer[y][x][BACKGROUND] != update_color_buffer[y][x][BACKGROUND])) {
            on_screen_buffer[y][x] = update_buffer[y][x];
            on_screen_color_buffer[y][x][FOREGROUND] = update_color_buffer[y][x][FOREGROUND];
            on_screen_color_buffer[y][x][BACKGROUND] = update_color_buffer[y][x][BACKGROUND];
            foreground_color_offset = update_color_buffer[y][x][FOREGROUND];
            background_color_offset = update_color_buffer[y][x][BACKGROUND];
            draw_font_tile(update_buffer[y][x], x * global_font->width, y * global_font->height, colors[foreground_color_offset], colors[background_color_offset], global_font);
            update_buffer[y][x] = 0;
            update_color_buffer[y][x][FOREGROUND] = 0;
            update_color_buffer[y][x][BACKGROUND] = 0;
        }
        }
    }
}

void redraw_console_line() {
    uint8_t foreground_color_offset;
    uint8_t background_color_offset;
    for (uint8_t x = 0; x < CONSOLE_WIDTH; x++) {
        if ((on_screen_buffer[console_y][x] != update_buffer[console_y][x]) ||
            (on_screen_color_buffer[console_y][x][FOREGROUND] != update_color_buffer[console_y][x][FOREGROUND]) ||
            (on_screen_color_buffer[console_y][x][BACKGROUND] != update_color_buffer[console_y][x][BACKGROUND])) {
            on_screen_buffer[console_y][x] = update_buffer[console_y][x];
            on_screen_color_buffer[console_y][x][FOREGROUND] = update_color_buffer[console_y][x][FOREGROUND];
            on_screen_color_buffer[console_y][x][BACKGROUND] = update_color_buffer[console_y][x][BACKGROUND];
            foreground_color_offset = update_color_buffer[console_y][x][FOREGROUND];
            background_color_offset = update_color_buffer[console_y][x][BACKGROUND];
            draw_font_tile(update_buffer[console_y][x], x * global_font->width, console_y * global_font->height, colors[foreground_color_offset], colors[background_color_offset], global_font);
        }
    }
}

void scroll_console() {
    for (uint8_t i = 1; i < CONSOLE_HEIGHT; i++) {
        memmove(&update_buffer[i - 1], &on_screen_buffer[i], CONSOLE_WIDTH);
        memmove(&update_color_buffer[i - 1], &on_screen_color_buffer[i], CONSOLE_WIDTH * 2);
    }
    memset(&update_buffer[CONSOLE_HEIGHT - 1], 0, CONSOLE_WIDTH);
    memset(&update_color_buffer[CONSOLE_HEIGHT - 1], 0, CONSOLE_WIDTH * 2);
    console_y = CONSOLE_HEIGHT - 1;
    redraw_console();
}

void console_handle_esc_code(char character) {
    if (character >= '0' && character <= '9') {
        escape_code_parameters[escape_code_parameter_count] *= 10;
        escape_code_parameters[escape_code_parameter_count] += character - '0';
        return;
    }

    if (character == '[') {
        return;
    } else if (character == ';') {
        escape_code_parameter_count++;
        if (escape_code_parameter_count >= MAX_ESC_CODE_PARAMETERS)
        escape_code_parameter_count = 0;
        return;
    }

    is_in_escape_code = false;

    // TODO: implement the rest of the control codes
    switch (character) {
        case 'm': { // set color
        for (int i = 0; i <= escape_code_parameter_count; i++) {
            uint8_t parameter = escape_code_parameters[i];
            if (parameter == 0) {
            // reset colors
            current_foreground_color_offset = DEFAULT_FOREGROUND_COLOR;
            current_background_color_offset = DEFAULT_BACKGROUND_COLOR;
            } else if (parameter == 39) {
            // reset foreground color
            current_foreground_color_offset = DEFAULT_FOREGROUND_COLOR;
            } else if (parameter == 49) {
            // reset background color
            current_background_color_offset = DEFAULT_BACKGROUND_COLOR;
            } else if (parameter >= 30 && parameter <= 37) {
            // set foreground color
            current_foreground_color_offset = parameter - 30;
            } else if (parameter >= 40 && parameter <= 47) {
            // set background color
            current_background_color_offset = parameter - 40;
            }
        }
        break;
        }

        case 'H': { // home cursor OR move to position depending on number of paramters
        if (escape_code_parameter_count == 0) {
            // no paramters, so just home the cursor
            console_x = 0;
            console_y = 0;
        } else {
            // set the cursor position
            console_x = escape_code_parameters[0];
            console_y = escape_code_parameters[1];
        }
        break;
        }

        case 'f': { // move to position
        console_x = escape_code_parameters[0];
        console_y = escape_code_parameters[1];
        break;
        }

        case 'A': { // move up
        console_y -= escape_code_parameters[0];
        if (console_y < 0)
            console_y = 0;
        break;
        }

        case 'B': { // move down
        console_y += escape_code_parameters[0];
        if (console_y > CONSOLE_HEIGHT - 1)
            console_y = CONSOLE_HEIGHT - 1;
        break;
        }

        case 'C': { // move right
        console_x += escape_code_parameters[0];
        if (console_x > CONSOLE_WIDTH - 1)
            console_x = CONSOLE_WIDTH - 1;
        break;
        }

        case 'D': { // move left
        console_x -= escape_code_parameters[0];
        if (console_x < 0)
            console_x = 0;
        break;
        }

        case 'G': { // move to column
        console_x = escape_code_parameters[0];
        break;
        }
    }
    }

void print_character_to_console(char character) {
    // check for various characters
    if (character == 0) {
        // null
        return;
    } else if (character == '\b') {
        // backspace
        if (console_x > 0)
        console_x--;
        return;
    } else if (character == '\r') {
        // carriage return
        redraw_console_line();
        console_x = 0;
        return;
    } else if (character == '\n') {
        // line feed
        redraw_console_line();
        console_x = 0;
        console_y++;
        if (console_y >= CONSOLE_HEIGHT)
            scroll_console();
        return;
    } else if (character == '\e') {
        is_in_escape_code = true;
        escape_code_parameter_count = 0;
        for (int i = 0; i < MAX_ESC_CODE_PARAMETERS; i++) {
            escape_code_parameters[i] = 0;
        }
        return;
    }

    if (is_in_escape_code) {
        console_handle_esc_code(character);
        return;
    }

    // check if we are at the end of this line
    if (console_x >= CONSOLE_WIDTH) {
        // if so, redraw and increment to the next line
        redraw_console_line();
        console_x = 0;
        console_y++;
    }

    // check if we need to scroll the console
    if (console_y >= CONSOLE_HEIGHT)
        scroll_console();

    // set character and colors in the update buffers
    update_buffer[console_y][console_x] = character;
    update_color_buffer[console_y][console_x][FOREGROUND] = current_foreground_color_offset;
    update_color_buffer[console_y][console_x][BACKGROUND] = current_background_color_offset;

    // increment X counter
    console_x++;
}

void main() {
    event_t event;
    char buffer[1];
    char *string = "FennecOS console\nstarting sh.elf\n\n";
    while (*string) {
        print_character_to_console(*string);
        string++;
    }
    if (!new_process("sh.elf", NULL)) {
        string = "failed to create new process for sh.elf\n";
        while (*string) {
            print_character_to_console(*string);
            string++;
        }
        exit();
    }

    while (true) {
        // read from other processes' stdout
        buffer[0] = 0;
        if (read(1, buffer, 1) && buffer[0]) {
            print_character_to_console(buffer[0]);
            redraw_console_line();
        }

        // write keyboard input to other processes' stdin
        if (get_next_event(&event)) {
            if (event.type == KEY_DOWN) {
                buffer[0] = scancode_to_ascii(event.arg0);
                write(0, buffer, 1);
            }
        }

        yield();
    }
}
