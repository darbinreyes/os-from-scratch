#!/bin/sh

file=kernel.bin
# 16384 = 512*32 # SECTOR_READ_COUNT=32
maxsize=16384
actualsize=$(wc -c < "$file")
echo Max size is $maxsize. Is this up to date?
echo kernel.bin size is $actualsize
if [ $actualsize -lt $maxsize ]; then
    echo kernel.bin size is OK.
    exit 0
else
    echo kernel.bin TOO LARGE increase boot_sect.asm SECTOR_READ_COUNT.
    exit 1
fi