//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//For create and register a procfs entry
#include <linux/proc_fs.h>
//For copy_to_user, copy_from_user, put_user
#include <asm/uaccess.h>
//For finding the parent process ID of the module
#include <asm/current.h>
//For "struct task_struct"
#include <linux/sched.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "readwriteprocfs"
//This is the constant that used for determination of buffer length
#define MAX_BUF_LEN 4096


//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple ProcFS module with simple read and write function in its /proc entry");
MODULE_VERSION("1.0.2");


//Creating a proc directory entry structure
static struct proc_dir_entry* our_proc_file;
//Now we have to create a buffer for our longest message (MAX_BUF_LEN)
//and a variable to count our actual message size
static char procfs_buffer[MAX_BUF_LEN];
static unsigned long procfs_buffer_size = 0;


//Each time a process try to open proc entry inorder to write to or read from it
//This function have to do something like adjusting reference count
int procfs_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "READWRITEPROCFS: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	//Eachtime you open the entry point, infact you are using the device, so you have to
	//count the references to it, in order to when you want to release it, you could safely release
	//the device with reference count of zero
	//So we increase the reference count using try_module_get
	try_module_get(THIS_MODULE);
	return SUCCESS;
}


//When a user try to cat or otherwise read the /proc entry, this function does the job
static ssize_t procfs_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
	static int ret = 0;
	printk(KERN_INFO "READWRITEPROCFS: Read Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if(ret){
		/* we have finished to read, return 0 */
		printk(KERN_INFO "READWRITEPROCFS: Read END\n");
		ret = 0;
	}
	else{
		//Write data to the user buffer
		if(raw_copy_to_user(buffer, procfs_buffer, procfs_buffer_size))
			return -EFAULT;

		printk(KERN_INFO "READWRITEPROCFS: Read %lu bytes\n", procfs_buffer_size);
		ret = procfs_buffer_size;
	}

	//The function returns the number charachters which have been read
	return ret;
}


//Each time user try to echo something or otherwise write anything to the /proc entry, this function does the job
static ssize_t procfs_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "READWRITEPROCFS: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);
	//get buffer size
	if(length > MAX_BUF_LEN)
		procfs_buffer_size = MAX_BUF_LEN;
	else
		procfs_buffer_size = length;
	//Write data from the user buffer
	if(raw_copy_from_user(procfs_buffer, buffer, procfs_buffer_size))
		return -EFAULT;

	//The function returns the number charachters which have been written
	printk(KERN_INFO "READWRITEPROCFS: Write %lu bytes\n", procfs_buffer_size);
	return procfs_buffer_size;
}


//Each time you release the /proc entry after read or write somthing from and to /proc entry
//This function have to adjust the reference count and does its job
int procfs_close(struct inode *inode, struct file *file){
	printk(KERN_INFO "READWRITEPROCFS: Release Function, Process \"%s:%i\"\n", current->comm, current->pid);
	//When you release the entry point, that means you have finished with the device so
	//decrese the reference count wit module_put
	module_put(THIS_MODULE);
	return SUCCESS;
}


//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the correspond /proc entry
static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = procfs_read,
	.write = procfs_write,
	.open = procfs_open,
	.release = procfs_close,
};


//Your module's entry point
static int __init readwrite_procfs_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "READWRITEPROCFS: Initialization.\n");
	printk(KERN_INFO "READWRITEPROCFS: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

	our_proc_file = proc_create(DEVICE_NAME, 0644 , NULL, &fops);
	//Put an error message in kernel log if cannot create proc entry
	if(!our_proc_file){
		printk(KERN_ALERT "READWRITEPROCFS: Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}

	printk(KERN_INFO "READWRITEPROCFS: /proc/%s has been created.\n", DEVICE_NAME);
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit readwrite_procfs_exit(void){
	//Initiating module unloading procedure
	printk(KERN_INFO "READWRITEPROCFS: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Remove proc filesystem entry from system
	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "READWRITEPROCFS: /proc/%s has been removed.\n", DEVICE_NAME);

	printk(KERN_INFO "READWRITEPROCFS: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}

//Now we need to define init-module and cleanup_module aliases
module_init(readwrite_procfs_init);
module_exit(readwrite_procfs_exit);
