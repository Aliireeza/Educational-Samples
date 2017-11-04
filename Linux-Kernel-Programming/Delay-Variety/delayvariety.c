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
//Most of delay functions lie here
#include <linux/delay.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "delayvariety"
//This constant will use to create our delays in jiffies
#define MYDELAY 1000


//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple ProcFS module which shows diffrent delay functions in real world operations");
MODULE_VERSION("1.0.2");


//We have a command line argument with default value of 1
//Which will be used to distinguish between what method of delay must use
static int delaymode = 1;
module_param(delaymode, int, 0);
MODULE_PARM_DESC(delaymode, "1: Busy Waiting, 2: Yielding Processor, 3: Timeout, 4: Short Delays, 5: Long Sleep");


//Creating a proc directory entry structure
static struct proc_dir_entry* our_proc_file;


//This function calls on demand of read request from seq_files
static int proc_show(struct seq_file *m, void *v){
	//These variables will use to determine the time lapse between before and after each type of delays
	unsigned long after_delay_time[5], before_delay_time[5];
	//This is just a counter
	int i;

	//Now, depend on the value of delaymode command line argument we will use different method of delays
	switch(delaymode){
		//Busy Waiting
		case 1:
			for(i=0; i<=4; i++){
				before_delay_time[i] = jiffies;
				while(time_before(jiffies, before_delay_time[i] + MYDELAY))
					cpu_relax();
				after_delay_time[i] = jiffies;
				}
			break;
		//Yeilding The Processor
		case 2:
			for(i=0; i<=4; i++){
				before_delay_time[i] = jiffies;
				while(time_before(jiffies, before_delay_time[i] + MYDELAY))
					schedule();
				after_delay_time[i] = jiffies;
				}
			break;
		//Timeout
		case 3:
			for(i=0; i<=4; i++){
				before_delay_time[i] = jiffies;
				schedule_timeout(MYDELAY);
				after_delay_time[i] = jiffies;
				}
			break;
		//Short Delays
		case 4:
			for(i=0; i<=4; i++){
				before_delay_time[i] = jiffies;
				udelay(MYDELAY);
				after_delay_time[i] = jiffies;
				}
			break;
		//Long Sleep
		case 5:
			for(i=0; i<=4; i++){
				before_delay_time[i] = jiffies;
				msleep(MYDELAY / HZ * 1000);
				after_delay_time[i] = jiffies;
				}
			break;
		//Not Valid Value, but could use to mesure time lapses between simple commands
		default:
			for(i=0; i<=4; i++){
				before_delay_time[i] = jiffies;
				after_delay_time[i] = jiffies;
			}
	}

	//Now it is time for the rest of the world to understand the magnificence of our output
	seq_printf(m, "%d\n%lu %lu\n%lu %lu\n%lu %lu\n%lu %lu\n%lu %lu\n",
		delaymode,
		before_delay_time[0], after_delay_time[0],
		before_delay_time[1], after_delay_time[1],
		before_delay_time[2], after_delay_time[2],
		before_delay_time[3], after_delay_time[3],
		before_delay_time[4], after_delay_time[4]);
	return SUCCESS;
}


//This is where system functionallity triggers every time some process try to read from our proc entry
static int proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "DELAYVARIETY: Seqfile Read, Process \"%s:%d\" \n", current->comm, current->pid);
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
static int __init delayvariety_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "DELAYVARIETY: Initialization.\n");
	printk(KERN_INFO "DELAYVARIETY: Init Module, Process \"%s:%d\" \n", current->comm, current->pid);

	our_proc_file = proc_create(DEVICE_NAME, 0644 , NULL, &fops);
	//Put an error message in kernel log if cannot create proc entry
	if(!our_proc_file){
		printk(KERN_ALERT "DELAYVARIETY: Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}

	printk(KERN_INFO "DELAYVARIETY: /proc/%s has been created.\n", DEVICE_NAME);
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit delayvariety_exit(void){
	//Remove proc filesystem entry from system
	printk(KERN_INFO "DELAYVARIETY: Cleanup Module, Process \"%s:%d\" \n", current->comm, current->pid);

	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "DELAYVARIETY: /proc/%s has been removed.\n", DEVICE_NAME);

	printk(KERN_INFO "DELAYVARIETY: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(delayvariety_init);
module_exit(delayvariety_exit);
