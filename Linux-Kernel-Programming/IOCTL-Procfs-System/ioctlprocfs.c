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
//For getting system information
#include <linux/utsname.h>
//For ioctl commands and macros
#include <linux/ioctl.h>
#include <asm/ioctl.h>
//For copy_to_user, copy_from_user, put_user
#include <asm/uaccess.h>

//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "information"


//These are our ioctl definition
#define MAGIC 'T'
#define IOC_MAXNR 6
#define IOCTL_SYSNAME _IOR(MAGIC, 0, char)
#define IOCTL_NODENAME _IOR(MAGIC, 1, char)
#define IOCTL_RELEASE _IOR(MAGIC, 2, char)
#define IOCTL_VERSION _IOR(MAGIC, 3, char)
#define IOCTL_MACHINE _IOR(MAGIC, 4, char)
#define IOCTL_DOMAINNAME _IOR(MAGIC, 5, char)

//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Kernel module using procfs, which could be used as ioctl handler in user space to obtain some system information");
MODULE_VERSION("1.0.2");

//Creating a proc directory entry structure
static struct proc_dir_entry* our_proc_file;


//When device recive ioctl commands this function will perform the job depending on what kind of command it recieved
long proc_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	static int err = 0, retval = 0;

	printk(KERN_INFO "IOCTLPROCFS: IOCTL Function, Process \"%s:%i\"\n", current->comm, current->pid);
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
		case IOCTL_SYSNAME:
			copy_to_user((int __user *) arg, utsname()->sysname, 30);
			break;
		case IOCTL_NODENAME:
			copy_to_user((int __user *) arg, utsname()->nodename, 30);
			break;
		case IOCTL_RELEASE:
			copy_to_user((int __user *) arg, utsname()->release, 30);
			break;
		case IOCTL_VERSION:
			copy_to_user((int __user *) arg, utsname()->version, 30);
			break;
		case IOCTL_MACHINE:
			copy_to_user((int __user *) arg, utsname()->machine, 30);
			break;
		case IOCTL_DOMAINNAME:
			copy_to_user((int __user *) arg, utsname()->domainname, 30);
			break;
		default:
			printk(KERN_ALERT "IOCTLPROCFS: Invalid IOCTL Command!\n");
			return -ENOTTY;
	}
	return retval;
}


//This function calls on demand of read request from seq_files
static int proc_show(struct seq_file *m, void *v){
	seq_printf(m, "IOCTL_SYSNAME, %lu\n", IOCTL_SYSNAME);
	seq_printf(m, "IOCTL_NODENAME, %lu\n", IOCTL_NODENAME);
	seq_printf(m, "IOCTL_RELEASE, %lu\n", IOCTL_RELEASE);
	seq_printf(m, "IOCTL_VERSION, %lu\n", IOCTL_VERSION);
	seq_printf(m, "IOCTL_MACHINE, %lu\n", IOCTL_MACHINE);
	seq_printf(m, "IOCTL_DOMAINNAME, %lu\n", IOCTL_DOMAINNAME);
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
static int __init capability_procfs_init(void){
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
static void __exit capability_procfs_exit(void){
	//Initiating module unloading procedure
	printk(KERN_INFO "IOCTLPROCFS: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Remove proc filesystem entry from system
	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "IOCTLPROCFS: /proc/%s has been removed.\n", DEVICE_NAME);

	printk(KERN_INFO "IOCTLPROCFS: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}

//Now we need to define init-module and cleanup_module aliases
module_init(capability_procfs_init);
module_exit(capability_procfs_exit);
