# Remote Port Blocker Filter Kernel Modules

Two simple modules to make an introduction to Kernel module programming using netfilter hooks


### How it works
This module will filter all incoming and outgoing network traffic of your system on a single port, then if it received a specific ICMP packet it will open it for the sender IP address and by receiving the second magic packet it closes it again. This will be perform by registering two netfilter hooks.

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
