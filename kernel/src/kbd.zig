const lshift_press = 0x2A;
const lshift_release = 0xAA;
const rshift_press = 0x36;
const rshift_release = 0xB6;
const lctrl_press = 0x1D;
const lctrl_release = 0x9D;

const scancode_map = [_]u8{
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8,
    '\t', // Tab
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n',
    0, // Control
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    0,
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0,
    '*',
    0, // Alt
    ' ', // Space
    0, // Caps lock
    0, // F1
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // F10
    0, // Num lock
    0, // Scroll lock
    0, // Home
    0, // Up arrow
    0, // Page Up
    '-',
    0, // Left arrow
    0,
    0, // Right arrow
    '+',
    0, // End
    0, // Down arrow
    0, // Page Down
    0, // Insert
    0, // Delete
    0,
    0,
    0,
    0, // F11
    0, // F12
    0, // All other keys are undefined
};

const scancode_map_shift = [_]u8{
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 8,
    '\t', // Tab
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '{',
    '}',
    '\n',
    0, // Control
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    ':',
    '"',
    '~',
    0,
    '|',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    '<',
    '>',
    '?',
    0,
    '*',
    0, // Alt
    ' ', // Space
    0, // Caps lock
    0, // F1
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // F10
    0, // Num lock
    0, // Scroll lock
    0, // Home
    0, // Up arrow
    0, // Page Up
    '-',
    0, // Left arrow
    0,
    0, // Right arrow
    '+',
    0, // End
    0, // Down arrow
    0, // Page Down
    0, // Insert
    0, // Delete
    0,
    0,
    0,
    0, // F11
    0, // F12
    0, // All other keys are undefined
};

var shift = false;
var ctrl = false;

pub fn keyboard_event(scancode: u8) void {
    if (scancode == lshift_press or scancode == rshift_press)
        shift = true
    else if (scancode == lshift_release or scancode == rshift_release)
        shift = false
    else if (scancode == lctrl_press)
        ctrl = true
    else if (scancode == lctrl_release)
        ctrl = false;
}

pub fn scancode_to_ascii(scancode: u8) u8 {
    if (shift)
        return scancode_map_shift[scancode]
    else if (ctrl)
        return if (scancode != 0 and scancode != lctrl_press) scancode_map_shift[scancode] - '@' else 0
    else
        return scancode_map[scancode];
}
