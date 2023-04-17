#!/bin/bash

set -e

zig build
sudo ./image.sh
qemu-system-i386 -hda boot.img -serial stdio
