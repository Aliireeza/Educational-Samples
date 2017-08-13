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
//For struct file_operations
#include <linux/fs.h>
//For cdev struct and functions
#include <linux/cdev.h>
//For device create and structs and other functions
#include <linux/device.h>
//For copy_to_user, copy_from_user, put_user
#include <asm/uaccess.h>
//For finding the parent process ID of the module
#include <asm/current.h>
//For using task_struct
#include <linux/sched.h>
//For ioctl commands and macros
#include <linux/ioctl.h>
#include <asm/ioctl.h>
//For evaluating capabilities
#include <linux/capability.h>
//For using spinlocks
#include <linux/spinlock.h>
//For blocking I/O
#include <linux/wait.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "advancedchardev"
//This is the constant that used for determination of buffer length
#define MAX_BUF_LEN 4096

//These are our ioctl definition
#define MAGIC 'T'
#define IOC_MAXNR 4
#define ADVANCED_CHARDEV_IOCTL_RESET	_IO(MAGIC, 0)
#define ADVANCED_CHARDEV_IOCTL_READ	_IOR(MAGIC, 1, int)
#define ADVANCED_CHARDEV_IOCTL_WRITE	_IOW(MAGIC, 2, char)
#define ADVANCED_CHARDEV_IOCTL_ENCRYPT	_IO(MAGIC, 3)



//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer and it's functionality
MODULE_AUTHOR("Aliireeza Teymoorian");
MODULE_DESCRIPTION("This is an advance Character Device driver with ioctl, concurrency, capabilities, and auto dev file creation");


//Major is for device driver major number and device open is just a flag
//to make sure that only one device can access the /dev entry at a moment
//Minor is just a range base for obtaining a range of minor number
// associated with major number
static int major, minor = 0, device_open_flag = 0;
static int minor_base = 0, minor_count = 1;
static atomic_t device_open_atomic = ATOMIC_INIT(1);

//Now we have to create some data structures related to our charachter device
static dev_t our_device;
static struct class *our_device_class;
static struct cdev our_cdev;

//Now we have to create a buffer for our longest message (MAX_BUF_LEN)
//and a variable to count our actual message sizeK
static char dev_buffer[MAX_BUF_LEN];
static unsigned long dev_buffer_size = 0;
static char encryption_key = MAGIC;

//Creating a proc directory entry structure
static struct proc_dir_entry* our_proc_file;

//Concurrency structs are defined here
static spinlock_t device_file_spinlock;
static wait_queue_head_t our_queue;



//This function calls on demand of read request from seq_files
static int proc_show(struct seq_file *m, void *v){
	seq_printf(m, "ADVANCED_CHARDEV_IOCTL_RESET, %u\n", ADVANCED_CHARDEV_IOCTL_RESET);
	seq_printf(m, "ADVANCED_CHARDEV_IOCTL_READ, %lu\n", ADVANCED_CHARDEV_IOCTL_READ);
	seq_printf(m, "ADVANCED_CHARDEV_IOCTL_WRITE %lu\n", ADVANCED_CHARDEV_IOCTL_WRITE);
	seq_printf(m, "ADVANCED_CHARDEV_IOCTL_ENCRYPT, %u\n", ADVANCED_CHARDEV_IOCTL_ENCRYPT);
	return 0;
}

//This is where system functionallity triggers every time some process try to read from our proc entry
static int proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "ADVANCEDCHARDEV: ProcFS Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_show, NULL);
}


//When device recive ioctl commands this function will perform the job depending on what kind of command it recieved
long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	static int err = 0, i = 0, retval = 0, capability_status;

	printk(KERN_INFO "ADVANCEDCHARDEV: IOCTL Function, Process \"%s:%i\"\n", current->comm, current->pid);
	printk(KERN_INFO "ADVANCEDCHARDEV: IOCTL Command, %d\n", cmd);

	if(_IOC_TYPE(cmd) != MAGIC || _IOC_NR(cmd) > IOC_MAXNR)
		return -ENOTTY;

	if(_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
	if(_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
	if(err)
		return -EFAULT;

	switch(cmd){
		case ADVANCED_CHARDEV_IOCTL_RESET:
			printk(KERN_INFO "ADVANCEDCHARDEV: IOCTL Command, ADVANCED_CHARDEV_IOCTL_RESET\n");
			if(!capable(CAP_SYS_ADMIN))
				return -EPERM;
			for(i=0; i<MAX_BUF_LEN ;i++)
				dev_buffer[i] = NULL;
			break;

		case ADVANCED_CHARDEV_IOCTL_READ:
			printk(KERN_INFO "ADVANCEDCHARDEV: IOCTL Command, ADVANCED_CHARDEV_IOCTL_READ\n");
			if(capable(CAP_DAC_OVERRIDE))
				capability_status = 0;
			else if(capable(CAP_NET_ADMIN))
				capability_status = 1;
			else if(capable(CAP_SYS_MODULE))
				capability_status = 2;
			else if(capable(CAP_SYS_RAWIO))
				capability_status = 3;
			else if(capable(CAP_SYS_ADMIN))
				capability_status = 4;
			else if(capable(CAP_SYS_TTY_CONFIG))
				capability_status = 5;
			else
				capability_status = 6;
			retval = __put_user(capability_status, (int __user *) arg);
			break;

		case ADVANCED_CHARDEV_IOCTL_WRITE:
			printk(KERN_INFO "ADVANCEDCHARDEV: IOCTL Command, ADVANCED_CHARDEV_IOCTL_WRITE\n");
			retval = __get_user(encryption_key, (char __user *) arg);
			break;

		case ADVANCED_CHARDEV_IOCTL_ENCRYPT:
			printk(KERN_INFO "ADVANCEDCHARDEV: IOCTL Command, ADVANCED_CHARDEV_IOCTL_ENCRYPT\n");
			if(!capable(CAP_SYS_ADMIN))
				return -EPERM;
			for(i=0; i<dev_buffer_size-1 && dev_buffer != '\0';i++)
				dev_buffer[i] = (dev_buffer[i] + encryption_key) % 128;
			break;
		default:
			return -ENOTTY;
	}


	return retval;
}


//Each time a process try to open /dev entry in order to write to or read from it
//This function have to do something like adjusting reference count
int device_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "ADVANCEDCHARDEV: DEV File Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	//device_open just used as a simple mutex, but it is not as safe as one, in real concurrent multitasking
	//So whenever the user try to open the entry point, we have to check that if someone else does the same thing
	//Then increase it and let the process continue

	//This is a more secure way, to use atomic variables
	spin_lock(&device_file_spinlock);
	if(device_open_flag == 1 && !atomic_dec_and_test(&device_open_atomic)){
		spin_unlock(&device_file_spinlock);
		if(file->f_flags & O_NONBLOCK)
			return -EBUSY;

		if(wait_event_interruptible(our_queue, (device_open_flag == 1))){
			printk(KERN_ALERT "ADVANCEDCHARDEV: Open Function put process \"%s:%i\" in Block mode\n", current->comm, current->pid);
			return -ERESTARTSYS;
		}
		spin_lock(&device_file_spinlock);
	}
	spin_unlock(&device_file_spinlock);

	spin_lock(&device_file_spinlock);
	device_open_flag++;
	spin_unlock(&device_file_spinlock);
	atomic_inc(&device_open_atomic);
	//Eachtime you open the entry point, infact you are using the device, so you have to
	//count the references to it, in order to when you want to release it, you could safely release
	//the device with reference count of zero
	//So we increase the reference count using try_module_get
	try_module_get(THIS_MODULE);
	return SUCCESS;
}


//When a user try to cat or otherwise read the /dev entry, this function does the job
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
	static int ret = 0;
	printk(KERN_INFO "ADVANCEDCHARDEV: DEV File Read Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if (ret) {
		/* we have finished to read, return 0 */
		printk(KERN_INFO "ADVANCEDCHARDEV: /dev entry read has END\n");
		ret = 0;
	}
	else{
		/* fill the buffer, return the buffer size */
		if ( copy_to_user(buffer, dev_buffer, dev_buffer_size) )
			return -EFAULT;

		printk(KERN_INFO "ADVANCEDCHARDEV: %lu bytes has read from /dev entry\n", dev_buffer_size);
		ret = dev_buffer_size;
	}

	//The function returns read charachters count
	return ret;
}


//Each time user try to echo something or otherwise write anything to the /dev entry, this function does the job
static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "ADVANCEDCHARDEV: DEV File Write Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if (length > MAX_BUF_LEN)
		dev_buffer_size = MAX_BUF_LEN;
	else
		dev_buffer_size = length;

	//write data to the buffer
	if (copy_from_user(dev_buffer, buffer, dev_buffer_size))
		return -EFAULT;

	//The function returns wrote charachters count
	printk(KERN_INFO "ADVANCEDCHARDEV: %lu bytes has wrote to /dev entry\n", dev_buffer_size);
	return dev_buffer_size;
}


//Each time you release the /dev entry after read or write somthing from and to /dev entry
//This function have to adjust the reference count and does its job
int device_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "ADVANCEDCHARDEV: DEV File Release Function, Process \"%s:%i\"\n", current->comm, current->pid);
	//In time of device release we have to decrease the mutex like flag, Device_Open
	spin_lock(&device_file_spinlock);
	device_open_flag--;
	spin_unlock(&device_file_spinlock);
	atomic_inc(&device_open_atomic);
	wake_up_interruptible(&our_queue);
	//When you release the entry point, that means you have finished with the device so
	//decrese the reference count wit module_put
	module_put(THIS_MODULE);
	return SUCCESS;
}


//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the correspond /dev entry
static struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release
};

static const struct file_operations proc_fops = {
	.open = proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


//Your module's entry point
static int __init advanced_chardev_init(void){
	//These mesages will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "ADVANCEDCHARDEV: Initialization.\n");
	printk(KERN_INFO "ADVANCEDCHARDEV: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);
	//We are going to obtain a major number and a range of minor numbers
	//and registering the charachter device and creating its dev entry.
	//This procedure will be accomplished in four steps
	//First, We need to get from system major and minor numbers
	if(alloc_chrdev_region(&our_device, minor_base, minor_count, DEVICE_NAME)<0){
		printk(KERN_ALERT "ADVANCEDCHARDEV: Device cannot obtain major and minor numbers\n");
		return -EFAULT;
	}
	//Now we will use these tow macros to obtain device information
	major = MAJOR(our_device);
	minor = MINOR(our_device);
	printk(KERN_INFO "ADVANCEDCHARDEV: Device has obtained Major: %d and Minor: %d successfully.\n", major, minor);

	//Second, we have to create a class for our driver
	our_device_class = class_create(THIS_MODULE, DEVICE_NAME);
	if(!our_device_class){
		printk(KERN_ALERT "ADVANCEDCHARDEV: Device could not create associated class.\n");
		unregister_chrdev_region(minor_base, minor_count);
		printk(KERN_INFO "ADVANCEDCHARDEV: Device unregistred Major: %d and Minor: %d\n", major, minor);
		return -EFAULT;
	}

	printk(KERN_INFO "ADVANCEDCHARDEV: Device has created associated class successfully.\n");

	//Third, We have to create our device and device file
	if(!device_create(our_device_class, NULL, our_device, NULL, DEVICE_NAME)){
		printk(KERN_ALERT "ADVANCEDCHARDEV: Device could not create associated device file.\n");
		class_destroy(our_device_class);
		printk(KERN_INFO "ADVANCEDCHARDEV: Device unregistred associated class.\n");
		unregister_chrdev_region(minor_base, minor_count);
		printk(KERN_INFO "ADVANCEDCHARDEV: Device unregistred Major: %d and Minor: %d\n", major, minor);
		return -EFAULT;
	}

	printk(KERN_INFO "ADVANCEDCHARDEV: Device file has been created in /dev/%s\n", DEVICE_NAME);

	//Fourth, We have to initiate our device to its file_operations struct
	cdev_init(&our_cdev, &dev_fops);
	if(cdev_add(&our_cdev, our_device, minor_count) < 0){
		printk(KERN_ALERT "ADVANCEDCHARDEV: Device could not initialize associated device.\n");
		device_destroy(our_device_class, our_device);
		printk(KERN_INFO "ADVANCEDCHARDEV: Device removed associated /dev entry.\n");
		class_destroy(our_device_class);
		printk(KERN_INFO "ADVANCEDCHARDEV: Device unregistred associated class.\n");
		unregister_chrdev_region(minor_base, minor_count);
		printk(KERN_INFO "ADVANCEDCHARDEV: Device unregistred Major: %d and Minor: %d\n", major, minor);
		return -EFAULT;
	}

	our_proc_file = proc_create(DEVICE_NAME, 0644 , NULL, &proc_fops);
	//Put an error message in kernel log if cannot create proc entry
	if(!our_proc_file){
		printk(KERN_ALERT "ADVANCEDCHARDEV: ProcFS registration failure.\n");
		cdev_del(&our_cdev);
		printk(KERN_INFO "ADVANCEDCHARDEV: Device released associated file operations.\n");
		device_destroy(our_device_class, our_device);
		printk(KERN_INFO "ADVANCEDCHARDEV: Device removed associated /dev entry.\n");
		class_destroy(our_device_class);
		printk(KERN_INFO "ADVANCEDCHARDEV: Device unregistred associated class.\n");
		unregister_chrdev_region(minor_base, minor_count);
		printk(KERN_INFO "ADVANCEDCHARDEV: Device unregistred Major: %d and Minor: %d\n", major, minor);
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}
	printk(KERN_INFO "ADVANCEDCHARDEV: /proc/%s has been created.\n", DEVICE_NAME);

	spin_lock_init(&device_file_spinlock);
	spin_unlock(&device_file_spinlock);
	init_waitqueue_head(&our_queue);
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit advanced_chardev_exit(void){
	printk(KERN_INFO "ADVANCEDCHARDEV: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//To get rid of our character device we have to do a four-step procedure as well
	cdev_del(&our_cdev);
	printk(KERN_INFO "ADVANCEDCHARDEV: Device released associated file operations.\n");
	device_destroy(our_device_class, our_device);
	printk(KERN_INFO "ADVANCEDCHARDEV: Device removed associated /dev entry.\n");
	class_destroy(our_device_class);
	printk(KERN_INFO "ADVANCEDCHARDEV: Device unregistred associated class.\n");
	unregister_chrdev_region(minor_base, minor_count);
	printk(KERN_INFO "ADVANCEDCHARDEV: Device unregistred Major: %d and Minor: %d\n", major, minor);

	//Remove procfs entry
	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "ADVANCEDCHARDEV: /proc/%s has been removed.\n", DEVICE_NAME);

	printk(KERN_INFO "ADVANCEDCHARDEV: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(advanced_chardev_init);
module_exit(advanced_chardev_exit);

