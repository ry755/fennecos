#pragma once

int init_serial();
int is_transmit_empty();
void write_serial(char c);
void write_serial_string(char *str);
int serial_received();
char read_serial();
