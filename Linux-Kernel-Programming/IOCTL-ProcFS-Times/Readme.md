# IOCTL Procfs Times Kernel Module

a simple module to make an introduction to Kernel module programming using proc file system and IOCTL commands


### How it works
This module register a proc interface and create the associated file. This proc file could be reached through open systemcall and then could be used by a user space program to catch IOCTL commands and return system times


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
