#!/bin/bash

set -e

if [[ $(id -u) -ne 0 ]]; then
    echo "run as root"
    exit 1
fi

image=boot.img

loop_device="$(losetup -P --show -f $image)"
loop_part=${loop_device}p1

mkdir -p mountdir
mount $loop_part mountdir
