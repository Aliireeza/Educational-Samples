# Kernel Version Kernel Module

a simple module to make an introduction to Kernel module programming and some debugging techniques


### How it works
This module show the way of obtaining Kernel version at compile time and decide on which commands to use at compile level.


### Install
You need to insert this command as root user or use sudo as an administrator
```
cd /to/the/module/source/code/
make
insmod ./modulename.ko
```

### Test
to see the output `dmesg`
to see module information `modinfo ./modulename.ko`


### Uninstall
```
rmmod modulename
make clean
```

for more information read the comments in the code
