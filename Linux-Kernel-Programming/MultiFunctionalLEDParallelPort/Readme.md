# Net Device Notifier Kernel Module

a simple module to make an introduction to Kernel module programming using parallel port, interrupt handeling and timers along side IOCTL, SysFS and ProcFS Interfaces


### How it works
This module register a parallel port driver with interrupts to make connected LED's blinking in three mode; single LED blinking, Whole LED's blinking and Counter


### Install
You need to insert this command as root user or use sudo as an administrator
```
cd /to/the/module/source/code/
make
insmod ./modulename.ko
```

### Test
to see the output `dmesg`
follow the instruction in the dmesg
to see module information `modinfo ./modulename.ko`
you have to connect 8 LEDs to the pin 2 to 9 of the computer's parallel port to see the output


### Uninstall
```
rmmod modulename
make clean
```

for more information read the comments in the code
