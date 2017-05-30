//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//For struct file_operations
#include <linux/fs.h>
//For copy_to_user, copy_from_user, put_user
#include <asm/uaccess.h>
//For finding the parent process ID of the module
#include <asm/current.h>
//For task_struct structure
#include <linux/sched.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "helloworldchardev"
//This is the constant that used for determination of buffer length
#define BUF_LEN 4096


//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Character Device Driver which count the number of reading its /dev file");
MODULE_VERSION("1.0.2");


//Major is for device driver major number and device open is just a flag
//to make sure that only one device can access the /dev entry at a moment
static int major, Device_Open = 0;
//The first one declare a buffer which will be used for our messages and
//the second one is just a pointer to the current location in the message
static char msg[BUF_LEN], *msg_Ptr;
static int counter = 0;

//Each time a process try to open dev entry inorder to write to or read from it
//This function have to do something like adjusting reference count
static int device_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "HELLOWORLDCHARDEV: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);

	if (Device_Open)
		return -EBUSY;
	//Device_Open just used as a simple mutex, but it is not as safe as one, in real concurrent multitasking
	//So whenever the user try to open the entry point, we have to check that if someone else does the same thing
	//Then increase it and let the process continue
	Device_Open++;

	//sprintf function does not show anything to the user, instead, it copies the counter variable content
	//combined with a string, completely like printf format, into our message buffer
	if(counter == 0){
		sprintf(msg, "Hello world!\n");
		counter++;
		}
	else
		sprintf(msg, "I already told you %d times, Hello world!\n", counter++);

	msg_Ptr = msg;
	//Eachtime you open the entry point, infact you are using the device, so you have to
	//count the references to it, in order to when you want to release it, you could safely release
	//the device with reference count of zero
	//So we increase the reference count using try_module_get
	try_module_get(THIS_MODULE);
	return SUCCESS;
}


//When a user try to cat or otherwise read the /dev entry, this function does the job
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
	printk(KERN_INFO "HELLOWORLDCHARDEV: Read Function, Process \"%s:%i\"\n", current->comm, current->pid);
	//Here is the deal, now we have to manage somehow to read correct position from our message buffer
	//inorder to put_user character by character message into the user space
	int bytes_read = 0;

	//This check is for applications such as cat, because they will tryagain to either reach EOF or recieve
	//return 0 which indicated that reading is not possible anymore, so they will be terminated, otherwise
	//cat application will remain open an you will not be able to continue in your command line
	if (*msg_Ptr == 0)
		return 0;

	while (length && *msg_Ptr) {
		//This is the actul part in which a user can at last see a character
		put_user(*(msg_Ptr++), buffer++);
		//Since put_user function push single carachter to user space,
		//after each iteration we should increment both driver and user buffer
		length--;
		bytes_read++;
		}

	//The function returns read charachters count
	return bytes_read;
}


//Each time user try to echo something or otherwise write anything to the /dev entry, this function does the job
static ssize_t device_write(struct file *filp, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "HELLOWORLDCHARDEV: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);
	printk(KERN_ALERT "HELLOWORLDCHARDEV: Sorry, WRITE operation is not permitted!\n");
	//The function returns wrote charachters count, here this means nothing

	//Because each time you open file for read or write, device_open function will call
	//we have to decrement the counter value in case that last increment was due to write function
	if(counter > 0)
		counter--;

	return -EINVAL;
}


//Each time you release the /dev entry after read or write somthing from and to /dev entry
//This function have to adjust the reference count and does its job
static int device_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "HELLOWORLDCHARDEV: Release Function, Process \"%s:%i\"\n", current->comm, current->pid);
	//In time of device release we have to decrease the mutex like flag, Device_Open
	//although this simple integer flag is not a concurrency handling mechanism, it
	//could demonstrate the function of such mechanisms, we will discuss later
	Device_Open--;
	//When you release the entry point, that means you have finished with the device so
	//decrese the reference count wit module_put
	module_put(THIS_MODULE);
	return SUCCESS;
}


//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the correspond /dev entry
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};


//Your module's entry point
static int __init helloworld_chardev_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "HELLOWORLDCHARDEV: Initialization.\n", DEVICE_NAME);
	printk(KERN_INFO "HELLOWORLDCHARDEV: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);
	//We are going to obtain a major number (with indicating 0 as the first argument)
	//and registering the charachter device at the same time.
	//Then we have to prompt to user how he could create a relative /dev entry point with this
	//major number in the kernel log
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		printk(KERN_ALERT "HELLOWORLDCHARDEV: Registration Failure (%d).\n", major);
		return major;
		}

	//For now we have to shamefully request our user to create the device file themselves
	//in several steps ahead, we will create this file automatically and eliminate this drawback
	//which is necessary to keep our initial samples as simple as possible ;)
	printk(KERN_INFO "HELLOWORLDCHARDEV: 'mknod /dev/%s c %d 0'\n", DEVICE_NAME, major);
	printk(KERN_INFO "HELLOWORLDCHARDEV: 'chmod 777 /dev/%s'\n", DEVICE_NAME);

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}

//You sould clean up the mess before exiting the module
static void __exit helloworld_chardev_exit(void){
	//Initiating module unloading procedure
	printk(KERN_INFO "HELLOWORLDCHARDEV: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Unregistering the character device, releasing Major number and Minor numbers, and make /dev entry ready for remove
	unregister_chrdev(major, DEVICE_NAME);
	printk(KERN_INFO "HELLOWORLDCHARDEV: 'rm /dev/%s'\n", DEVICE_NAME);

	printk(KERN_INFO "HELLOWORLDCHARDEV: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(helloworld_chardev_init);
module_exit(helloworld_chardev_exit);
