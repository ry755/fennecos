#!/bin/bash

set -e

if [[ $(id -u) -ne 0 ]]; then
    echo "run as root"
    exit 1
fi

image=boot.img

rm -f $image
fallocate -l 64M $image
echo -e "o\nn\n\n\n\n\nw\n" | fdisk $image
loop_device="$(losetup -P --show -f $image)"
loop_part=${loop_device}p1
mkfs.vfat -F32 $loop_part

mkdir -p mountdir
mount $loop_part mountdir
cd mountdir
cp -r ../base_image/* .
grub-install --modules=part_msdos --locales="" --themes="" --target=i386-pc --boot-directory="$PWD/boot" $loop_device
cd ..

umount mountdir
sync
rm -rf mountdir
losetup -d $loop_device

if [[ -n $SUDO_UID ]]; then
    chown $SUDO_UID:$SUDO_GID $image
fi
