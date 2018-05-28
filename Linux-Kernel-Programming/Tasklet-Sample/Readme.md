# IOMemSample Kernel Module

a simple Tasklet module, which could check wether an deferred work could be schedule for later execution


### How it works
This module register a tasklet function and evalute the time before and after running it, So, you could findout that the timelapse of the deferred work execution.


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
