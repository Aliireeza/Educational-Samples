//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//For jiffies, which will give us timelapce of the system
#include <linux/jiffies.h>
//For using Tasklets
#include <linux/interrupt.h>
//For catching CPU number
#include <linux/smp.h>
//To use strcpy just like standard c library
#include <linux/string.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "taskletsample"


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple module which using tasklet for educational purpose");
MODULE_VERSION("1.0.2");


//Now we will define our tasklet_struct variable for future use
static struct tasklet_struct our_tasklet;

//Here are some useful variables
static char our_tasklet_argument[20] = DEVICE_NAME, string_argument[20];
static unsigned long before_delay, after_delay;
static int i, j;


//This is a simple tasklet function
static void our_tasklet_function(unsigned long data){
	//First, you need to be sure some other process on the current cpu won't run tasklet function concurrently
	tasklet_trylock(&our_tasklet);
	//Then, simple create some useless output
	strcpy(string_argument, data);
	after_delay = jiffies;
	i = smp_processor_id();
	printk(KERN_INFO "TASKLETSAMPLE: Tasklet function of %s is running on CPU %d \n", string_argument, i);
	printk(KERN_INFO "TASKLETSAMPLE: Tasklet function scheduled on: %ld \n", before_delay);
	printk(KERN_INFO "TASKLETSAMPLE: Tasklet function executed on: %ld \n", after_delay);
	//So, let it go now
	tasklet_unlock(&our_tasklet);
}


//Your module's entry point
static int __init tasklet_module_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "TASKLETSAMPLE: Initialization.\n");

	//First we capture the current jiffies, before using the tasklet
	before_delay = jiffies;

	//Capturing CPU number of the current processor
	j = smp_processor_id();

	//Now we have to initiate our tasklet and we use address of our tasklet argument to pass some text
	tasklet_init(&our_tasklet, &our_tasklet_function, (unsigned long) &our_tasklet_argument);
	printk(KERN_INFO "TASKLETSAMPLE: Tasklet initiated on CPU %d \n", j);

	//Now its time to tell the Kernel to put our tasklet in its run queue
	//There is three different functions here, which we can choose between them

	//First, void tasklet_schedule(struct tasklet_struct *tasklet), with normal priority
	//Second, void tasklet_hi_schedule(struct tasklet_struct *tasklet), with hi priority
	//Third, void tasklet_hi_schedule_first(struct tasklet_struct *tasklet), out of queue

	//We use the first one for reaching maximum delay possible
	tasklet_schedule(&our_tasklet);

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit tasklet_module_exit(void){
	//Now we need to get rid of our tasklet, and there is a disable then kill sequence essential
	//unless you want to wait a long time, cause single kill function might take times to release it
	tasklet_disable_nosync(&our_tasklet);
	tasklet_kill(&our_tasklet);

	//Now, it is finished
	printk(KERN_INFO "TASKLETSAMPLE: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(tasklet_module_init);
module_exit(tasklet_module_exit);
