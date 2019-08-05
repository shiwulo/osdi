#!/bin/bash
gdb ./vmlinux -ex 'target remote localhost:666' -ex 'b start_kernel' -ex 'b trap_init'
