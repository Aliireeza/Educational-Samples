# Concurrency Sample Kernel Module

a simple module to make an introduction to module kernel programming using a character driver and proc interface


### How it works
This module register a character driver, obtain major number from the kernel and print relvant commands for the user in the Kernel log to create the essential device file. This device file could be read and write by users. Also the driver do the same for proc interface and then bring a set of concurrency handling mechanisms to show how you could manage simultaneous requests to your driver. 
```
Note: It had few ways of deadlocks intentionally
```


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
