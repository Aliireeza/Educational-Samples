# Emergency Halt Kernel/Machie Kernel Modules

Two simple modules to make an introduction to Kernel module programming using reboot.h header file


### How it works
This module will turn off your sysem the moment you try to install it ;)


### Install
You need to insert this command as root user or use sudo as an administrator
```
cd /to/the/module/source/code/
make
insmod ./modulename.ko
```

### Test
Your computer will halt


### Uninstall
No need actually ;)

for more information read the comments in the code
