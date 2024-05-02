#!/bin/bash

set -e

./build.sh
qemu-system-i386 -hda boot.img -serial stdio -m 256M
