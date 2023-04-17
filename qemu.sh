#!/bin/bash

set -e

zig build
qemu-system-i386 -hda boot.img -serial stdio
