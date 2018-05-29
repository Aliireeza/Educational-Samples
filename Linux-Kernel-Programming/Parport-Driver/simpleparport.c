//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//For struct file_operations
//For struct file_operations
#include <linux/fs.h>
//For cdev struct and functions
#include <linux/cdev.h>
//For device create and structs and other functions
#include <linux/device.h>
//For communicating with userspace
#include <linux/uaccess.h>
//For parport_driver struct
#include <linux/parport.h>
//For parport functionality in IBM clones
#include <linux/parport_pc.h>
//For ioctl commands and macros
#include <linux/ioctl.h>
//For evaluating capabilities
#include <linux/capability.h>
//For using spinlocks
#include <linux/spinlock.h>
//For blocking I/O
#include <linux/wait.h>

#include "commonioctlcommands.h"

//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "simpleparport"
//Static buffer length
#define MAX_BUF_LEN 4096


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple parallel port driver which uses parport interface");
MODULE_VERSION("1.0.2");

static int minor_base = 0, minor_count = 1;
static atomic_t device_open = ATOMIC_INIT(1);

static struct pardevice *parport_device;
static struct cdev parport_chardev;
static struct class *parport_class;
static dev_t device_number;
static spinlock_t lock;
static wait_queue_head_t queue;

static char string_buffer[MAX_BUF_LEN];
static unsigned long buffer_size = 0;

//*****************************************************************************
long parport_device_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	static int error = 0;

	if(_IOC_TYPE(cmd) != MAGIC || _IOC_NR(cmd) > IOC_MAXNR)
		return -ENOTTY;

	if(_IOC_DIR(cmd) & _IOC_READ)
		error = !access_ok(VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
	if(_IOC_DIR(cmd) & _IOC_WRITE)
		error = !access_ok(VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
	if(error)
		return -EFAULT;
		
	switch(cmd){
		case IOCTL_PARALLEL_RESET:
			printk(KERN_INFO "PARALLELLED: Reset IOCTL Command\n");
			if(!capable(CAP_SYS_ADMIN))
				return -EPERM;
			//Do Something to reset the whole device
			break;
		default:
			printk(KERN_ALERT "PARALLELLED: Invalid IOCTL Cmd (%u)\n", cmd);
			return -EFAULT;
		}
	return SUCCESS;
}


int parport_device_open(struct inode *inode, struct file *file){
	//First we want to be sure that this port used by only one process at a time
	//There for we using atomic device open flag and wait queue for providing
	//blocking I/O
	if(atomic_read(&device_open) == 1){
		if(file->f_flags & O_NONBLOCK)
			return -EBUSY;
		else
			if(wait_event_interruptible(queue, atomic_read(&device_open) == 1)){
				printk(KERN_INFO "PARPORT: Driver put process \"%s:%i\" in block mode\n", current->comm, current->pid);
				return -ERESTARTSYS;
			}
		}
	atomic_inc(&device_open);
	try_module_get(THIS_MODULE);
	return SUCCESS;
}


static ssize_t parport_device_read(struct file *filp, char *buffer, size_t length, loff_t * offset){
	//First we have to read data from parallel port
	static int retval = 0;
	spin_lock(&lock);
	parport_claim_or_block(parport_device);
	//parport_read_data(parport_device->port, string_buffer);
	parport_release(parport_device);	
	spin_unlock(&lock);
	//Then send the data to the userspace
	if(retval)
		retval = 0;
	else{
		//Fill the buffer, return the buffer size
		if(raw_copy_to_user(buffer, string_buffer, buffer_size))
			return -EFAULT;

		printk(KERN_INFO "PARPORT: %lu bytes has read from /dev entry\n", buffer_size);
		retval = buffer_size;
	}
	return retval;
}


static ssize_t parport_device_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	//First we have to receive data from the userspace
	if (length > MAX_BUF_LEN)
		buffer_size = MAX_BUF_LEN;
	else
		buffer_size = length;

	//write data to the buffer
	if(raw_copy_from_user(string_buffer, buffer, buffer_size))
		return -EFAULT;

	//The function returns wrote charachters count
	printk(KERN_INFO "PARPORT: %lu bytes has wrote to /dev entry\n", buffer_size);
	
	//Then send the data to the parallel port
	spin_lock(&lock);
	parport_claim_or_block(parport_device);
	parport_write_data(parport_device->port, string_buffer);
	parport_release(parport_device);
	spin_unlock(&lock);
	return buffer_size;
}


int parport_device_release(struct inode *inode, struct file *file){
	//Reset the device open flag and wake up the queue
	atomic_dec(&device_open);
	wake_up_interruptible(&queue);
	module_put(THIS_MODULE);
	return SUCCESS;
}


static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = parport_device_open,
	.read = parport_device_read,
	.write = parport_device_write,
	.release = parport_device_release,
	.unlocked_ioctl = parport_device_ioctl,
};


//*****************************************************************************
static int parport_preempt(void *handler){
	return 1;
}


static void parport_attach(struct parport *port){
	parport_device = parport_register_device(port, DEVICE_NAME, parport_preempt, NULL, NULL, 0, NULL);
	if(!parport_device)
		printk(KERN_ALERT "PARPORT: Parport Device Registration Failure!\n");
}


static void parport_detach(struct parport *port){
	parport_unregister_device(parport_device);
}



static struct parport_driver driver = {
	.name = "parport",
	.attach = parport_attach,
	.detach = parport_detach,
};


//*****************************************************************************
//Your module's entry point
static int __init parport_init(void){
	int retval = 0;
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "PARPORT: Initialization ...\n");
	retval = alloc_chrdev_region(&device_number, minor_base, minor_count, DEVICE_NAME);
	if(retval < 0){
		printk(KERN_ALERT "PARPORT: Chardev region registration failure!\n");
		return retval;
		}
	
	parport_class = class_create(THIS_MODULE, DEVICE_NAME);
	if(IS_ERR(parport_class)){
		printk(KERN_ALERT "PARPORT: Device class registration failure!\n");
		unregister_chrdev_region(device_number, 1);
		return -EFAULT;
		}		
		
	cdev_init(&parport_chardev, &fops);
	retval = cdev_add(&parport_chardev, device_number, 1);
	if(retval){
		printk(KERN_ALERT "PARPORT: Character device registration failure!\n");
		unregister_chrdev_region(device_number, 1);
		class_destroy(parport_class);
		return retval;
	}
	
	if(!device_create(parport_class, NULL, device_number, NULL, DEVICE_NAME)){
		printk(KERN_ALERT "PARPORT: Device creation failure!\n");
		unregister_chrdev_region(device_number, 1);
		class_destroy(parport_class);
		return -ENODEV;
	}
	
	retval = parport_register_driver(&driver);
	if(retval){
		printk(KERN_ALERT "PARPORT: Parport driver registration failure!\n");
		unregister_chrdev_region(device_number, 1);
		device_destroy(parport_class, device_number);
		class_destroy(parport_class);
		return retval;
	}
	
	spin_lock_init(&lock);
	spin_unlock(&lock);
	init_waitqueue_head(&queue);	
	printk(KERN_INFO "PARPORT: Parport driver register successfully\n");
	
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit parport_exit(void){
	parport_unregister_driver(&driver);
	unregister_chrdev_region(device_number, 1);
	device_destroy(parport_class, device_number);
	class_destroy(parport_class);
	printk(KERN_INFO "PARPORT: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(parport_init);
module_exit(parport_exit);
