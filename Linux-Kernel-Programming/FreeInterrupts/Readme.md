# Free Interrupts Kernel Module

a simple module to make an introduction to interrupt handling using proc file system


### How it works
This module register a proc interface and create the associated file. Then start to register an interrupt handler for some kinds of interrupts and return the number of free irq lines for each kind


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
