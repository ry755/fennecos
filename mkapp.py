#!/usr/bin/env python3
# mkapp.py
# turn a raw binary into an app file

import os
import sys
import struct

if len(sys.argv) != 5:
    print(f"usage: {sys.argv[0]} <input> <output> <app name> <bss size>")
    exit()

input_binary_path = sys.argv[1]
output_app_path = sys.argv[2]
app_name = sys.argv[3]
bss_size = sys.argv[4]

if bss_size.startswith("0x"):
    bss_size = int(bss_size, 16)
else:
    bss_size = int(bss_size)

print(f"Making app called \"{app_name}\" with bss size {bss_size}")

input_bin = open(input_binary_path, 'rb')
output_app = open(output_app_path, 'w+b')

output_app.write(bytearray([ord('A'), ord('P'), ord('P'), 0]))
output_app.write(struct.pack('<L', bss_size))
padding = [0] * (32 - len(app_name)) # pad app name to 32 bytes
output_app.write(bytearray(app_name, 'utf-8'))
output_app.write(bytearray(padding))
output_app.write(input_bin.read())
