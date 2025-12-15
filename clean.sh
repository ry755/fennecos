#!/bin/bash

rm -rf build/
rm -rf base_image/app/*.app
rm -rf base_image/sys/*.app
rm -f base_image/boot/kernel.elf
rm -f base_image/boot/kernel.sym
rm -f boot.img
rm -f floppy.img
