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
//Obviously for using timers
#include <linux/timer.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "fibonaccitimer"


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple ProcFS module which use a timer with a fibonacci like growing timeout");
MODULE_VERSION("1.0.2");


//Creating a proc directory entry structure
static struct proc_dir_entry* our_proc_file;
//We have to use struct timer_list to creat our own timer
static struct timer_list mytimer;


//These variables will use to create our fibbonaci delays and restore jiffies values
static unsigned long after_delay_time[10];
static int delay_a = HZ, delay_b = HZ, delay_c;
static int i = 0;


//This function will call at each timer's timeout
void fibonaccitimer(unsigned long data){
	if(i<=19){
		after_delay_time[i++] = jiffies;
		delay_c = delay_a + delay_b;
		delay_a = delay_b;
		delay_b = delay_c;
		mod_timer(&mytimer, jiffies + delay_b - delay_a);
	}
}


//This function calls on demand of read request from seq_files
static int proc_show(struct seq_file *m, void *v){
	//Now it is time for the rest of the world to understand the glorious of our output
	static int j=0;
	for(j=0; j<i; j++)
		seq_printf(m, "%lu\n", after_delay_time[j]);
	return SUCCESS;
}

//This is where system functionallity triggers every time some process try to read from our proc entry
static int proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "FIBONACCITIMER: Seqfile Read, Process \"%s:%d\" \n", current->comm, current->pid);
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
static int __init fibonacci_timer_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "FIBONACCITIMER: Initialization.\n");
	printk(KERN_INFO "FIBONACCITIMER: Init Module, Process \"%s:%d\" \n", current->comm, current->pid);

	our_proc_file = proc_create(DEVICE_NAME, 0644 , NULL, &fops);
	//Put an error message in kernel log if cannot create proc entry
	if(!our_proc_file){
		printk(KERN_ALERT "FIBONACCITIMER: Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}
	printk(KERN_INFO "FIBONACCITIMER: /proc/%s has been created.\n", DEVICE_NAME);

	//It is now time to initiate our timer, but it will not start yet
	init_timer(&mytimer);
	mytimer.function = fibonaccitimer;
	mytimer.data = 1;
	mytimer.expires = jiffies + delay_b;

	//now start the timer
	add_timer(&mytimer);

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit fibonacci_timer_exit(void){
	//Remove proc filesystem entry from system
	printk(KERN_INFO "FIBONACCITIMER: Cleanup Module, Process \"%s:%d\" \n", current->comm, current->pid);

	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "FIBONACCITIMER: /proc/%s has been removed.\n", DEVICE_NAME);

	//Now we have to release the timer resources
	del_timer(&mytimer);

	printk(KERN_INFO "FIBONACCITIMER: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(fibonacci_timer_init);
module_exit(fibonacci_timer_exit);
