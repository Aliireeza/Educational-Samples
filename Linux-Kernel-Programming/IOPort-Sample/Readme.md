# IOMemSample Kernel Module

a simple I/O Port module, which could check wether an I/O port is free at the moment or not


### How it works
This module register a range of IO Port from the main memory.So, you could findout that range is free or not.


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
