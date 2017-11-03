//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//For struct file_operations
#include <linux/fs.h>
//For create and register a procfs entry
#include <linux/proc_fs.h>
//For providing read function of the entry with ease
#include <linux/seq_file.h>
//For copy_to_user, copy_from_user, put_user
#include <asm/uaccess.h>
//For finding the parent process ID of the module
#include <asm/current.h>
//For using task_struct
#include <linux/sched.h>
//For ioctl commands and macros
#include <linux/ioctl.h>
#include <asm/ioctl.h>

//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "readwritechardev"
//This is the constant that used for determination of buffer length
#define MAX_BUF_LEN 4096


//These are our ioctl definition
#define MAGIC 'C'
#define IOC_MAXNR 3
#define IOCTL_RESET _IO(MAGIC, 0)
#define IOCTL_MAX_BUFFER_SIZE _IOR(MAGIC, 1, int)
#define IOCTL_DEV_BUFFER_SIZE _IOR(MAGIC, 2, int)

//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Character Device driver with simple read and write function in its /dev entry with ioctl commands");
MODULE_VERSION("1.0.2");

//Major is for device driver major number and device open is just a flag
//to make sure that only one device can access the /dev entry at a moment
static int major, Device_Open = 0;

//Creating a proc directory entry structure
static struct proc_dir_entry* our_proc_file;

//Now we have to create a buffer for our longest message (MAX_BUF_LEN)
//and a variable to count our actual message size
static char dev_buffer[MAX_BUF_LEN];
static unsigned long dev_buffer_size = 0, i = 0;


//Each time a process try to open /dev entry in order to write to or read from it
//This function have to do something like adjusting reference count
int device_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "READWRITECHARDEV: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if (Device_Open)
		return -EBUSY;
	//Device_Open just used as a simple mutex, but it is not as safe as one, in real concurrent multitasking
	//So whenever the user try to open the entry point, we have to check that if someone else does the same thing
	//Then increase it and let the process continue
	Device_Open++;

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
	printk(KERN_INFO "READWRITECHARDEV: Read Function, Process \"%s:%i\"\n", current->comm, current->pid);
	if(ret){
		//we have finished to read, return 0, incase you use an application that try to find an EOF
		printk(KERN_INFO "READWRITECHARDEV: /dev entry read has END\n");
		ret = 0;
	}
	else{
		//Write data to the user buffer
		if(raw_copy_to_user(buffer, dev_buffer, dev_buffer_size))
			return -EFAULT;

		printk(KERN_INFO "READWRITECHARDEV: %lu bytes has read from /dev entry\n", dev_buffer_size);
		ret = dev_buffer_size;
	}

	//The function returns the number charachters which have been read
	return ret;
}


//Each time user try to echo something or otherwise write anything to the /dev entry, this function does the job
static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "READWRITECHARDEV: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);	//get buffer size
	if (length > MAX_BUF_LEN)
		dev_buffer_size = MAX_BUF_LEN;
	else
		dev_buffer_size = length;

	//Write data from the user buffer
	if(raw_copy_from_user(dev_buffer, buffer, dev_buffer_size))
		return -EFAULT;

	//The function returns the number charachters which have been written
	printk(KERN_INFO "READWRITECHARDEV: %lu bytes has wrote to /dev entry\n", dev_buffer_size);
	return dev_buffer_size;
}


//Each time you release the /dev entry after read or write somthing from and to /dev entry
//This function have to adjust the reference count and does its job
int device_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "READWRITECHARDEV: Release Function, Process \"%s:%i\"\n", current->comm, current->pid);
	//In time of device release we have to decrease the mutex like flag, Device_Open
	Device_Open--;
	//When you release the entry point, that means you have finished with the device so
	//decrese the reference count wit module_put
	module_put(THIS_MODULE);
	return SUCCESS;
}



//When device recive ioctl commands this function will perform the job depending on what kind of command it recieved
long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	static int err = 0, retval = 0;

	printk(KERN_INFO "READWRITECHARDEV: IOCTL Function, Process \"%s:%i\"\n", current->comm, current->pid);
	printk(KERN_INFO "READWRITECHARDEV: IOCTL Command, %d\n", cmd);

	if(_IOC_TYPE(cmd) != MAGIC || _IOC_NR(cmd) > IOC_MAXNR)
		return -ENOTTY;

	if(_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
	if(_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
	if(err)
		return -EFAULT;
	
	switch(cmd){
		case IOCTL_RESET:
			//Here we have to find a way to check y=user's permissions and we will do this with capability functions
			//But for now, we just assume we need to reset the buffer
			for(i=0; i< MAX_BUF_LEN; dev_buffer[i++]="");
			dev_buffer_size = 0;
			printk(KERN_INFO "READWRITECHARDEV: IOCTL_RESET signal has been received.\n");
			break;
		case IOCTL_MAX_BUFFER_SIZE:
			raw_copy_to_user((int __user *) arg, MAX_BUF_LEN, 15);
			printk(KERN_INFO "READWRITECHARDEV: IOCTL_MAX_BUFFER_SIZE signal has been received.\n");
			break;
		case IOCTL_DEV_BUFFER_SIZE:
			raw_copy_to_user((int __user *) arg, dev_buffer_size, 15);
			printk(KERN_INFO "READWRITECHARDEV: IOCTL_DEV_BUFFER_SIZE signal has been received.\n");
			break;
		default:
			printk(KERN_ALERT "READWRITECHARDEV: Invalid IOCTL Command!\n");
			return -ENOTTY;
	}
	return retval;
}


//This function calls on demand of read request from seq_files
static int proc_show(struct seq_file *m, void *v){
	seq_printf(m, "IOCTL_RESET, %lu\n", IOCTL_RESET);
	seq_printf(m, "IOCTL_MAX_BUFFER_SIZE, %lu\n", IOCTL_MAX_BUFFER_SIZE);
	seq_printf(m, "IOCTL_DEV_BUFFER_SIZE, %lu\n", IOCTL_DEV_BUFFER_SIZE);
	return SUCCESS;
}

//This is where system functionallity triggers every time some process try to read from our proc entry
static int proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "READWRITECHARDEV: Proc Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_show, NULL);
}


//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the correspond /dev and /procentry
static const struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl,
};

static const struct file_operations pops = {
	.open = proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.unlocked_ioctl = device_ioctl,
};

//Your module's entry point
static int __init readwrite_chardev_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "READWRITECHARDEV: Initialization.\n", DEVICE_NAME);
	printk(KERN_INFO "READWRITECHARDEV: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);
	
	our_proc_file = proc_create(DEVICE_NAME, 0644 , NULL, &pops);
	//Put an error message in kernel log if cannot create proc entry
	if(!our_proc_file){
		printk(KERN_ALERT "READWRITECHARDEV: Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}

	printk(KERN_INFO "READWRITECHARDEV: /proc/%s has been created.\n", DEVICE_NAME);
	
	
	//We are going to obtain a major number (with indicating 0 as the first argument)
	//and registering the charachter device at the same time.
	//Then we have to prompt to user how he could create a relative /dev entry point with this
	//major number in the kernel log
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		printk(KERN_ALERT "READWRITECHARDEV: Registration Filure, %d Major\n", major);
		return major;
		}
	//For now we have to shamefully request our user to create the device file themselves
	//in several steps ahead, we will create this file automatically and eliminate this drawback
	//which is necessary to keep our initial samples as simple as possible ;)
	printk(KERN_INFO "READWRITECHARDEV: 'mknod /dev/%s c %d 0'\n", DEVICE_NAME, major);
	printk(KERN_INFO "READWRITECHARDEV: 'chmod 777 /dev/%s'.\n", DEVICE_NAME);

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}

//You sould clean up the mess before exiting the module
static void __exit readwrite_chardev_exit(void){
	printk(KERN_INFO "READWRITECHARDEV: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Remove proc filesystem entry from system
	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "READWRITECHARDEV: /proc/%s has been removed.\n", DEVICE_NAME);
	
	//Then we have to release our character driver
	unregister_chrdev(major, DEVICE_NAME);
	printk(KERN_INFO "READWRITECHARDEV: 'rm /dev/%s'\n", DEVICE_NAME);

	printk(KERN_INFO "READWRITECHARDEV: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(readwrite_chardev_init);
module_exit(readwrite_chardev_exit);
