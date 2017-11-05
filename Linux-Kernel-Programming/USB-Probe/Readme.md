# USB Probe Kernel Module

a simple module to make an introduction to Kernel module programming using Kernel USB driver interfaces


### How it works
This module register an USB interface driver to monitor events that could be happening on USB devices which have been introduced to the system by `MODULE_DEVICE_TABLE`


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


### Uninstall
```
rmmod modulename
make clean
```

for more information read the comments in the code
