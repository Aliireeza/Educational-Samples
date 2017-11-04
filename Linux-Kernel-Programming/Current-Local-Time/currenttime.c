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
//For jiffies, which will give us timelapce of the system
#include <linux/jiffies.h>
//For finding the parent process ID of the module
#include <asm/current.h>
//For "struct task_struct"
#include <linux/sched.h>
//For "do_gettimeofday" or "current_kernel_time" or "getnstimeofday" functions
#include <linux/time.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "currenttime"


//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple ProcFS module which returns GMT and Local time");
MODULE_VERSION("1.0.2");


//These command line arguments are defined for calculation of local time from GMT
//Default value is +3:30 which is used to obtain TEHRAN's local time
static int gmt_hour = 3;
module_param(gmt_hour, int, 0);
MODULE_PARM_DESC(gmt_hour, "This command line argument will differ local time hours from GMT time, default is +3 hours for Tehran, Iran");


static int gmt_minute = 30;
module_param(gmt_minute, int, 0);
MODULE_PARM_DESC(gmt_minute, "This command line argument will differ local time minutes from GMT time, default is +30 minutes for Tehran, Iran");


//This will use to create a nice output
static char gmt_sign = '+';


//Creating a proc directory entry structure
static struct proc_dir_entry* our_proc_file;


//This function calls on demand of read request from seq_files
static int proc_show(struct seq_file *m, void *v){
	//This is how we are going to get these times
	//First, we use current_kernel_time to obtain current time of system
	//which will be used to calculate times for GMT value
	struct timespec my_timeofday_gmt = current_kernel_time();
	//Then for absolutely no reason at all we will use a different way
	//to obtain exactly the same value from getnstimeofday function
	//which will be used in calculation of local time
	struct timespec my_timeofday_loc;
	getnstimeofday(&my_timeofday_loc);

	//These variables will use to create a more human readable output
	int hour_gmt, minute_gmt, second_gmt;
	int hour_loc, minute_loc, second_loc;

	//Now we only use the second fragment of timespec structs
	second_gmt = (int) my_timeofday_gmt.tv_sec;
	hour_gmt = (second_gmt / 3600) % 24;
	minute_gmt = (second_gmt / 60) % 60;
	second_gmt %= 60;

	second_loc = (int) my_timeofday_loc.tv_sec;
	hour_loc = ((second_loc / 3600) % 24 + gmt_hour) % 24;
	minute_loc = (second_loc / 60) % 60 + gmt_minute;

	//And this is just a simple trick to adding local difference to the real value
	if(minute_loc>=60){
		hour_loc = (hour_loc + 1) % 24;
		minute_loc %= 60;
		}
	second_loc %= 60;

	//Now we create a sharp simple output
	seq_printf(m, "GMT %d:%d:%d\tLocal Time %d:%d:%d (GMT %c%d:%d)\n",hour_gmt, minute_gmt, second_gmt, hour_loc, minute_loc, second_loc, gmt_sign, gmt_hour, gmt_minute);
	return SUCCESS;
}


//This is where system functionallity triggers every time some process try to read from our proc entry
static int proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "CURRENTTIME: Seqfile Read, Process \"%s:%d\" \n", current->comm, current->pid);
	return single_open(file, proc_show, NULL);
}


//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the correspond /proc entry
static const struct file_operations fops = {
	.open = proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


//Your module's entry point
static int __init currenttime_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "CURRENTTIME: Initialization.\n");
	printk(KERN_INFO "CURRENTTIME: Init Module, Process \"%s:%d\" \n", current->comm, current->pid);
	our_proc_file = proc_create(DEVICE_NAME, 0644 , NULL, &fops);
	//Put an error message in kernel log if cannot create proc entry
	if(!our_proc_file){
		printk(KERN_ALERT "CURRENTTIME: Registration Failure.\n", DEVICE_NAME);
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}

	//It is a simple calculation of our output sign
	if(gmt_hour > 0)
		gmt_sign = '+';
	else if(gmt_hour < 0)
		gmt_sign = '-';
	else
		if(gmt_minute >= 0)
			gmt_sign = '+';
		else
			gmt_sign = '-';
			

	printk(KERN_INFO "CURRENTTIME: /proc/%s has been created.\n", DEVICE_NAME);
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit currenttime_exit(void){
	//Remove proc filesystem entry from system
	printk(KERN_INFO "CURRENTTIME: Cleanup Module, Process \"%s:%d\" \n", current->comm, current->pid);

	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "CURRENTTIME: /proc/%s has been removed.\n", DEVICE_NAME);

	printk(KERN_INFO "CURRENTTIME: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(currenttime_init);
module_exit(currenttime_exit);
