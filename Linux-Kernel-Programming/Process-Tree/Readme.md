# Process Tree Kernel Module

a simple module to make an introduction to manipulating system process tree using proc file system


### How it works
This module register six proc interfaces and create the associated files. This proc files could be could be used to move around the system process tree and reveald each process information

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
