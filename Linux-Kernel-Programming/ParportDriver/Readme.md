# Simple Parport Driver

a simple module that uses parport interface to provide functionality over parallel port


### How it works
This module will create a DevFS [/dev/parport] entry so you could read from or write into it that works on parallel port.


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
