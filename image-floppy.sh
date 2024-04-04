#!/bin/bash

set -e

if [[ $(id -u) -ne 0 ]]; then
    echo "run as root"
    exit 1
fi

GRUB_INSTALL=grub-install
if ! command -v grub-install &> /dev/null; then
    GRUB_INSTALL=grub2-install
fi

image=floppy.img

rm -f $image
fallocate -l 1440K $image
loop_device="$(losetup -P --show -f $image)"
mkfs.fat -F12 $loop_device

mkdir -p mountdir
mount $loop_device mountdir
cd mountdir
rclone copy --exclude=res/ramdisk.img ../base_image/ .
${GRUB_INSTALL} --target=i386-pc --install-modules="multiboot" --locales="" --themes="" --fonts="ascii" --allow-floppy --compress=xz --force --boot-directory="$PWD/boot" $loop_device
if [ $GRUB_INSTALL = "grub2-install" ]; then
    mv boot/grub/* boot/grub2/
    rm -rf boot/grub
fi
cd ..

umount mountdir
sync
rm -rf mountdir
losetup -d $loop_device

if [[ -n $SUDO_UID ]]; then
    chown $SUDO_UID:$SUDO_GID $image
fi
