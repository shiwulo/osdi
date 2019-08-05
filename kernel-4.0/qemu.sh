#!/bin/bash
sudo qemu-system-x86_64  -nographic -kernel ./bzImage -append "rdinit=/linuxrc nokaslr console=ttyS0" --virtfs local,id=sharedFolder,path=./sharedFolder,security_model=none,mount_tag=sharedFolder -m 256 -initrd ./initramfs_data.cpio.gz
