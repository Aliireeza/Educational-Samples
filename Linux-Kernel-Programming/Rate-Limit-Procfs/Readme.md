# Rate Limit Procfs Kernel Module

a simple module to make an introduction to Kernel module programming and some debugging techniques


### How it works
This module creates a proc file which could count the running process on the system each time user try to read it, also using Kernel logs and printk_ratelimit to save the Kernel log from overloaded by debugging messages.


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
