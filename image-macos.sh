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

image=boot.img

#rm -f $image
#truncate -s 128M $image
#fdisk -i -a dos $image
loop_device="$(hdiutil attach -nomount $image | head -n1 | cut -d " " -f1)"
loop_part=${loop_device}s1
mkfs.vfat -F32 $loop_part

mkdir -p mountdir
mount -t msdos $loop_part mountdir
cd mountdir
cp -r ../base_image/* .
${GRUB_INSTALL} --modules=part_msdos --locales="" --themes="" --target=i386-pc --boot-directory="./boot" ../$image
if [ $GRUB_INSTALL = "grub2-install" ]; then
    mv boot/grub/* boot/grub2/
    rm -rf boot/grub
fi
cd ..

umount mountdir
sync
rm -rf mountdir
hdiutil detach $loop_device

if [[ -n $SUDO_UID ]]; then
    chown $SUDO_UID:$SUDO_GID $image
fi
