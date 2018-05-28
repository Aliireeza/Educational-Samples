//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//For create and register a procfs entry
#include <linux/proc_fs.h>
//For providing read function of the entry with ease
#include <linux/seq_file.h>
//For finding the parent process ID of the module
#include <asm/current.h>
//For using task_struct
#include <linux/sched.h>
//For ioctl commands and macros
#include <linux/ioctl.h>
#include <asm/ioctl.h>
//For copy_to_user, copy_from_user, put_user
#include <asm/uaccess.h>
//For get_current_user and get_current_cred
#include <linux/cred.h>

#include "commonioctlcommands.h"


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "selfprocess"



//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Kernel module using procfs, which could be used as ioctl handler in user space to obtain some process information");
MODULE_VERSION("1.0.2");

//Creating a proc directory entry structure
static struct proc_dir_entry* our_proc_file;


//This is a simple function which turn unsigned long to string
void ltoa(unsigned long value, char *ptr, int base){
	unsigned long t = 0, res = 0;
    unsigned long tmp = value;
    int count = 0;

    if (NULL == ptr)
      return NULL;

    if (tmp == 0)
      count++;

    while(tmp > 0){
      tmp = tmp/base;
      count++;
    }

    ptr += count;
    *ptr = '\0';

    do{
      res = value - base * (t = value / base);
      if (res < 10)
        * -- ptr = '0' + res;
      else if ((res >= 10) && (res < 16))
        * --ptr = 'A' - 10 + res;
    }while ((value = t) != 0);

    return(ptr);
}


//When device recive ioctl commands this function will perform the job depending on what kind of command it recieved
long proc_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	static int err = 0, retval = 0;
	unsigned long value;
	char output[30];
	printk(KERN_INFO "IOCTLPROCFS: IOCTL Command, %d\n", cmd);

	if(_IOC_TYPE(cmd) != MAGIC || _IOC_NR(cmd) > IOC_MAXNR)
		return -ENOTTY;

	if(_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
	if(_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
	if(err)
		return -EFAULT;

	switch(cmd){
		case IOCTL_PROCESS_NAME:
			strcpy(output, current->comm);
			break;
		case IOCTL_PROCESS_PID:
			value = current->pid;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_UID:
			value = get_current_user()->uid.val;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_EUID:
			value = get_current_cred()->euid.val;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_SUID:
			value = get_current_cred()->suid.val;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_FSUID:
			value = get_current_cred()->fsuid.val;
			ltoa(value, output, 10);
			break;

		case IOCTL_PROCESS_GID:
			value = get_current_cred()->gid.val;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_EGID:
			value = get_current_cred()->egid.val;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_SGID:
			value = get_current_cred()->sgid.val;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_FSGID:
			value = get_current_cred()->fsgid.val;
			ltoa(value, output, 10);
			break;

		case IOCTL_PROCESS_PGRP:
			value = task_pgrp(current);
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_PRIORITY:
			value = current->prio;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_REAL_PRIORITY:
			value = current->rt_priority;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_PROCESSOR:
			value = current->cpu;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_UTIME:
			value = current->utime;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_STIME:
			value = current->stime;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_GTIME:
			value = current->gtime;
			ltoa(value, output, 10);
			break;
		case IOCTL_PROCESS_STARTTIME:
			value = current->start_time;
			ltoa(value, output, 10);
			break;
		default:
			printk(KERN_ALERT "IOCTLPROCFS: Invalid IOCTL Command!\n");
			return -ENOTTY;
	}
	raw_copy_to_user((int __user *) arg, output, 30);
	return retval;
}


//This function calls on demand of read request from seq_files
static int proc_show(struct seq_file *m, void *v){
	seq_printf(m, "Process Name: %s [PID: %d, PGRP: %d, Priority:%lu]\n", current->comm, current->pid, task_pgrp(current), current->prio);
	seq_printf(m, "INFO: processor(%d), Real Time Priority(%lu)\n", current->cpu, current->rt_priority);
	seq_printf(m, "Times: utime(%lu), stime(%lu), gtime(%lu), start_time(%lu)\n", current->utime, current->stime, current->gtime, current->start_time);
	seq_printf(m, "USER: UID(%d), eUID(%d), sUID(%d), fsUID(%d)\n", get_current_user()->uid.val, get_current_cred()->euid.val, get_current_cred()->suid.val, get_current_cred()->fsuid.val);
	seq_printf(m, "GROUP: GID(%d), eGID(%d), sGID(%d), fsGID(%d)\n", get_current_cred()->gid.val, get_current_cred()->egid.val, get_current_cred()->sgid.val, get_current_cred()->fsgid.val);
	return SUCCESS;
}

//This is where system functionallity triggers every time some process try to read from our proc entry
static int proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "IOCTLPROCFS: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_show, NULL);
}


//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the corresponding /proc entry
static const struct file_operations fops = {
	.open = proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.unlocked_ioctl = proc_ioctl,
};


//Your module's entry point
static int __init ioctl_procfs_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "IOCTLPROCFS: Initialization.\n");
	printk(KERN_INFO "IOCTLPROCFS: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

	our_proc_file = proc_create(DEVICE_NAME, 0644 , NULL, &fops);
	//Put an error message in kernel log if cannot create proc entry
	if(!our_proc_file){
		printk(KERN_ALERT "IOCTLPROCFS: Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}

	printk(KERN_INFO "IOCTLPROCFS: /proc/%s has been created.\n", DEVICE_NAME);
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit ioctl_procfs_exit(void){
	//Initiating module unloading procedure
	printk(KERN_INFO "IOCTLPROCFS: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Remove proc filesystem entry from system
	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "IOCTLPROCFS: /proc/%s has been removed.\n", DEVICE_NAME);

	printk(KERN_INFO "IOCTLPROCFS: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}

//Now we need to define init-module and cleanup_module aliases
module_init(ioctl_procfs_init);
module_exit(ioctl_procfs_exit);
