#!/bin/bash
echo "start to copy files"
cp ./linux-4.0/usr/initramfs_data.cpio.gz ./
cp ./linux-4.0/vmlinux ./
cp ./linux-4.0/arch/x86_64/boot/bzImage ./
echo "create system.map"
nm ./vmlinux > system.map
