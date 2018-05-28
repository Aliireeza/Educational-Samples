# Duplicate Procfs Kernel Module

a simple module to make an introduction to Kernel module programming using proc file system


### How it works
This module register two proc interfaces and create the associated files. These proc files could be read and they will generate a simple output each time. Also we could put them in a simple folder.


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
