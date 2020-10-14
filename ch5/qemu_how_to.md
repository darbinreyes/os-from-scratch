
```

# Suppressing the warning "WARNING: Image format ... guessed raw."

qemu-system-i386 -drive file=os-image.bin,format=raw

# This command is equivalent to the bochs floppy configuration.
qemu-system-i386 -fda os-image

# This command is equivalent to the bochs floppy configuration AND fixes the "WARNING: ... guessed raw." For full docs, see here: https://www.qemu.org/docs/master/system/invocation.html#hxtool-1
qemu-system-i386 -drive file=os-image-1,if=floppy,format=raw

```