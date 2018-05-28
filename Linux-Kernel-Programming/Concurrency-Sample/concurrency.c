//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//We need this header in all kernel modules
#include <linux/module.h>
//And this is needed for the macros
#include <linux/init.h>
//For struct file_operations
#include <linux/fs.h>
//For create and register a procfs entry
#include <linux/proc_fs.h>
//For copy_to_user, copy_from_user, put_user
#include <linux/uaccess.h>
//For finding the parent process ID of the module
#include <asm/current.h>
//For using task_struct
#include <linux/sched.h>
//For using completions
#include <linux/completion.h>
//For using spinlocks
#include <linux/spinlock.h>
//For using semaphores and mutexes
#include <linux/semaphore.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "concurrency"
//This is the constant that used for determination of buffer length
#define MAX_BUF_LEN 4096


//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer and it's functionality
MODULE_AUTHOR("Aliireeza Teymoorian");
MODULE_DESCRIPTION("This module provide Read/Write functions on a Character device and a Proc file with concurrency issues in mind");


//It is for device driver major number
static int major;
//Creating a proc directory entry structure
static struct proc_dir_entry* our_proc_file;
//Now we have to create a buffer for our longest message (MAX_BUF_LEN)
//and a variable to count our actual message size for both our
//character device and procfs module
static char procfs_buffer[MAX_BUF_LEN], dev_buffer[MAX_BUF_LEN];
static unsigned long procfs_buffer_size = 0, dev_buffer_size = 0;
//Concurrency strucs are defined here
static struct semaphore sem;
static struct completion comp;
static spinlock_t lock;


//Each time a process try to open our proc entry inorder to write to/read from it
//This function have to do something like adjusting reference count and does its job in concurrency related issuses
int procfs_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "CONCURRENCY: Proc Filesystem, Open Function, Process \"%s:%d\" \n", current->comm, current->pid);
	//Try to catch our semaphore before read or write
	down(&sem);
	//Eachtime you open the entry point, infact you are using the device, so you have to
	//count the references to it, in order to when you want to release it, you could safely release
	//the device with reference count of zero. so we increase the reference count using try_module_get
	try_module_get(THIS_MODULE);
	return SUCCESS;
}


//When a user try to cat or otherwise read  from the /proc entry, this function will do the job
static ssize_t procfs_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
	printk(KERN_INFO "CONCURRENCY: Proc Filesystem, Read Function, Process \"%s:%d\" \n", current->comm, current->pid);
	//Get the lock and keep it until finishing the read process
	spin_lock(&lock);
	//Now continue to read
	static int ret = 0;
	if(ret){
		//we have finished to read, return 0
		printk(KERN_INFO "CONCURRENCY: Proc Filesystem, Read END\n");
		ret = 0;
	}
	else{
		//otherwise, fill the buffer with the current message in procfs_buffer, then return the buffer size
		if(raw_copy_to_user(buffer, procfs_buffer, procfs_buffer_size))
			return -EFAULT;

		printk(KERN_INFO "CONCURRENCY: Proc Filesystem, Read %lu bytes\n", procfs_buffer_size);
		ret = procfs_buffer_size;
	}
	//It is time to release the lock
	spin_unlock(&lock);
	//The function returns read charachters count
	return ret;
}


//Each time user try to echo something or otherwise write anything to the /proc entry, this function will do the job
static ssize_t procfs_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "CONCURRENCY: Proc Filesystem, Write Function, Process \"%s:%d\" \n", current->comm, current->pid);
	//Get the lock and keep it until finishing the write process
	spin_lock(&lock);
	//Now continue to write
	//Get the buffer size and keep it less than our MAX_BUF_LEN
	if(length > MAX_BUF_LEN)
		procfs_buffer_size = MAX_BUF_LEN;
	else
		procfs_buffer_size = length;

	//Write data from user, which is in the buffer to our procfs_buffer, with the procfs_buffer_size caculated above
	if(raw_copy_from_user(procfs_buffer, buffer, procfs_buffer_size))
		return -EFAULT;

	printk(KERN_INFO "READWRITEPROCFS: Proc Filesystem, Write %lu bytes\n", procfs_buffer_size);
	//It is time to release the lock
	spin_unlock(&lock);
	//The function returns wrote characters count
	return procfs_buffer_size;
}


//Each time you release the /proc entry after write or read somthing to or from  the /proc entry
//This function have to adjust the reference count and does its job in concurrency related issuses
int procfs_close(struct inode *inode, struct file *file){
	printk(KERN_INFO "CONCURRENCY: Proc Filesystem, Release Function, Process \"%s:%d\" \n", current->comm, current->pid);
	//Up the semaphore here after release the device
	up(&sem);
	//When you release the entry point, that means you have finished with the device so
	//decrese the reference count wit module_put
	module_put(THIS_MODULE);
	return SUCCESS;
}





//Each time a process try to open /dev entry in order to write to or read from it
//This function have to do something like adjusting reference count and some other stuffs
int device_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "CONCURRENCY: Character Device, Open Function, Process \"%s:%d\" \n", current->comm, current->pid);
	//Try to catch our semaphore before read or write
	down(&sem);
	//Eachtime you open the entry point, infact you are using the device, so you have to
	//count the references to it, in order to when you want to release it, you could safely release
	//the device with reference count of zero
	//So we increase the reference count using try_module_get
	try_module_get(THIS_MODULE);
	return SUCCESS;
}


//When a user try to cat or otherwise read from the /dev entry, this function will do the job
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
	printk(KERN_INFO "CONCURRENCY: Character Device, Read Function, Process \"%s:%d\" \n", current->comm, current->pid);
	//Now wait for a write process first
	wait_for_completion(&comp);
	//Now somebody wrote something in our /dev entry, so we could allow user process to continue reading
	static int ret = 0;
	if(ret){
		//we have finished to read, return 0
		printk(KERN_INFO "CONCURRENCY: Character Device, Read END\n");
		ret = 0;
	}
	else{
		//otherwise, fill the buffer with the current message in procfs_buffer, then return the buffer size
		if(raw_copy_to_user(buffer, dev_buffer, dev_buffer_size))
			return -EFAULT;


		printk(KERN_INFO "CONCURRENCY: Character Device, Read %lu bytes\n", dev_buffer_size);
		ret = dev_buffer_size;
	}

	//Now we have to re initialize our completion, becuse it is a one-shot device
	init_completion(&comp);
	//The function returns read charachters count
	return ret;
}


//Each time user try to echo something or otherwise write anything to the /dev entry, this function does the job
static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "CONCURRENCY: Character Device, Write Function, Process \"%s:%d\" \n", current->comm, current->pid);
	//get the buffer size and keep it less than our MAX_BUF_LEN
	if (length > MAX_BUF_LEN)
		dev_buffer_size = MAX_BUF_LEN;
	else
		dev_buffer_size = length;

	//Write data from user, which is in the buffer to our procfs_buffer, with the procfs_buffer_size caculated above
	if (raw_copy_from_user(dev_buffer, buffer, dev_buffer_size))
		return -EFAULT;

	//Now we could release the reader process by completing our completion
	complete(&comp);
	//The function returns wrote charachters count
	printk(KERN_INFO "CONCURRENCY: Character Device, Write %lu bytes\n", dev_buffer_size);
	return dev_buffer_size;
}


//Each time you release the /dev entry after read or write somthing from or to /dev entry
//This function have to adjust the reference count and does its other jobs
int device_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "CONCURRENCY: Character Device, Release Function, Process \"%s:%d\" \n", current->comm, current->pid);
	//Up the semaphore here after release the device
	up(&sem);
	//When you release the entry point, that means you have finished with the device so
	//decrese the reference count wit module_put
	module_put(THIS_MODULE);
	return SUCCESS;
}


//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity

//devfops has ben defined in order to respond to userspace access demand to the correspond /dev entry
static struct file_operations devfops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

//procfops has been defined in order to respond to userspace access demand to the correspond /proc entry
static const struct file_operations procfops = {
	.owner = THIS_MODULE,
	.read = procfs_read,
	.write = procfs_write,
	.open = procfs_open,
	.release = procfs_close,
};


//Your module's entry point
static int __init concurrency_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "CONCURRENCY: Initialization of %s Module\n", DEVICE_NAME);
	printk(KERN_INFO "CONCURRENCY: Init Module, Process \"%s:%d\" \n", current->comm, current->pid);

	//We are going to obtain a major number (with indicating 0 as the first argument)
	//and registering the charachter device at the same time.
	//Then we have to prompt to user how he could create a relative /dev entry point with this
	//major number in the kernel log
	major = register_chrdev(0, DEVICE_NAME, &devfops);
	if(major < 0){
		printk(KERN_ALERT "CONCURRENCY: Character Device, Registration Failed, %d Major\n", major);
		//exit with major as module error code in case of registration failure
		return major;
		}

	printk(KERN_INFO "CONCURRENCY: Character Device, 'mknod /dev/%s c %d 0'\n", DEVICE_NAME, major);
	printk(KERN_INFO "CONCURRENCY: Character Device, 'chmod 777 /dev/%s'\n", DEVICE_NAME);

	our_proc_file = proc_create(DEVICE_NAME, 0644 , NULL, &procfops);
	//Put an error message in kernel log if cannot create proc entry
	if(!our_proc_file){
		printk(KERN_ALERT "CONCURRENCY: Proc Filesystem, Registration Failed.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}

	printk(KERN_INFO "CONCURRENCY: Proc Filesystem, /proc/%s has been created.\n", DEVICE_NAME);

	//Now we want to allocate our concurrency tools
	//First, initializing our semaphores, with initial value of 4 for whole module functionality
	sema_init(&sem, 4);
	//Second our completion for our character device read and write operations
	init_completion(&comp);
	//Third our spinlock for our procfs entry read and write functions
	spin_lock_init(&lock);

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit concurrency_exit(void){
	int retval;
	printk(KERN_INFO "CONCURRENCY: Unregisteration Process\n");
	printk(KERN_INFO "CONCURRENCY: Cleanup Module, Process \"%s:%d\" \n", current->comm, current->pid);

	//Remove device /dev entry from system
	unregister_chrdev(major, DEVICE_NAME);
	printk(KERN_INFO "CONCURRENCY: Character Device, 'rm /dev/%s'.\n", DEVICE_NAME);

	//Remove proc filesystem entry from system
	remove_proc_entry(DEVICE_NAME, NULL);

	printk(KERN_INFO "CONCURRENCY: Proc Filesystem, /proc/%s has been removed.\n", DEVICE_NAME);
	//Now we are going to release resources of our concurrency tools
	//First, get rid of our semaphores

	//Then, our completion with useless return value, stupid!
	complete_and_exit(&comp, retval);
	//And finally, our spinlock will release

	printk(KERN_INFO "CONCURRENCY: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(concurrency_init);
module_exit(concurrency_exit);

