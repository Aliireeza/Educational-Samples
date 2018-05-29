# Simple Timer Kernel Module

a simple module to make an introduction to Kernel module programming using Timers and Tasklets


### How it works
Apart from showing you the anatomy of a Kernel module, this module just print some text in the Linux Kernel log using `printk` function, with some delays.


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
