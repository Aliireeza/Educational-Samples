# Keyboard Notifier Kernel Module

a simple module to make an introduction to Kernel module programming using notifier chains


### How it works
This module register an keyboard notifier to monitor events that could be happening on keyboard


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
