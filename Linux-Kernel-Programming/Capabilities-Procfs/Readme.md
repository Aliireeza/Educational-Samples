# Capability Procfs Kernel Module

a simple module to make an introduction to Kernel module programming using proc file system


### How it works
This module register a proc interface and create the associated file. This proc file could be read and it will generate a simple output that shows the permissions and capabilities of the user process who read it.


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
