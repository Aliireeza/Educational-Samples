# Port Filter Kernel Modules

Two simple modules to make an introduction to Kernel module programming using netfilter hooks


### How it works
This module will filter all incoming and outgoing network traffic of your system due to the specified network port by registering a netfilter hook


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
