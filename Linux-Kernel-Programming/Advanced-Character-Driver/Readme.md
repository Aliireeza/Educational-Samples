# Advanced Charachter Driver Kernel Module

a simple module to make an introduction to module kernel programming using a character driver


### How it works
This module register a character driver, obtain major number from the kernel and print relvant commands for the user in the Kernel log to create the essential device file. This device file could be read and write by users. Also it produces IOCTL and Proc Interfaces.


### Install
You need to insert this command as root user or use sudo as an administrator
```
cd /to/the/module/source/code/
make
insmod ./modulename.ko
```
for the user space application
```
gcc -o application ./userspace.c
```

### Test
to see the output `dmesg`
follow the instruction in the dmesg
to see module information `modinfo ./modulename.ko`
to work with the loaded module run the application

### Uninstall
```
rmmod modulename
make clean
```

for more information read the comments in the code

### TODO
1. Adding sysfs interface
2. Modify the Userspace program to provide a full set of functionality
3. Provide a set of various encryption/decryption methods
4. Using dynamic memory allocation
