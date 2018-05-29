//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//For using interrupt functions and Tasklets
#include <linux/interrupt.h>
//Most of delay functions lie here
#include <linux/delay.h>
//For using Kernel timers
#include <linux/timer.h>
//For catching CPU number
#include <linux/smp.h>
//For jiffies and other time functions
#include <linux/jiffies.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "simpletimer"


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple module which using Kernel timers to create simple outputs");
MODULE_VERSION("1.0.2");



//We have to use struct timer_list to creat our own timer
static struct timer_list first_timer;
static struct tasklet_struct first_tasklet, second_tasklet;

static char tasklet_fucntion[30] = DEVICE_NAME;
static int timer_expire_delay = 500;
static int first_timer_counter = 0;


//These are our tasklet functions
static void first_tasklet_function(unsigned long data){
	tasklet_trylock(&first_tasklet);
	udelay(5);
	printk(KERN_INFO "SIMPLETIMER: First Tasklet Function, Itteration:%d, CPU:%d, Jiffies:%ld, Argument:%s\n", first_timer_counter, smp_processor_id(), jiffies,(char *) data);
	tasklet_schedule(&second_tasklet);
	tasklet_unlock(&first_tasklet);
}


static void second_tasklet_function(unsigned long data){
	tasklet_trylock(&second_tasklet);
	udelay(7);
	printk(KERN_INFO "SIMPLETIMER: Second Tasklet Function, Itteration:%d, CPU:%d, Jiffies:%ld, Argument:%s\n", first_timer_counter, smp_processor_id(), jiffies, (char *) data);
	tasklet_unlock(&second_tasklet);
}


//These functions will call at each timers' timeout
void first_timer_function(struct timer_list *timer){
	tasklet_schedule(&first_tasklet);
	if(first_timer_counter < 5){
		mod_timer(&first_timer, jiffies + msecs_to_jiffies(timer_expire_delay));
		first_timer_counter++;
		}
}


//Your module's entry point
static int __init simple_timer_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "SIMPLETIMER: Initialization ...\n");

	//First we have to initiate our tasklets
	tasklet_init(&first_tasklet, &first_tasklet_function, (unsigned long) &tasklet_fucntion);
	tasklet_init(&second_tasklet, &second_tasklet_function, (unsigned long) &tasklet_fucntion);
	printk(KERN_INFO "SIMPLETIMER: %s has initiated Tasklets successfully on %d CPU.\n", DEVICE_NAME, smp_processor_id());
	
	//It is now time to initiate our timer, but it will not start yet	
	first_timer.function = first_timer_function;
	first_timer.expires = jiffies + msecs_to_jiffies(timer_expire_delay);
	
	//now start the timer
	add_timer(&first_timer);
	printk(KERN_INFO "SIMPLETIMER: %s has initiated Timer successfully.\n", DEVICE_NAME);
		

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit simple_timer_exit(void){
	//First we have to release the timer resources
	del_timer(&first_timer);
	printk(KERN_INFO "SIMPLETIMER: %s has finished and released timer.\n", DEVICE_NAME);

	//Then we have to get rid of our tasklets
	tasklet_disable_nosync(&first_tasklet);
	tasklet_disable_nosync(&second_tasklet);
	tasklet_kill(&first_tasklet);
	tasklet_kill(&second_tasklet);
	printk(KERN_INFO "SIMPLETIMER: %s has killed and released tasklets on %d CPU.\n", DEVICE_NAME, smp_processor_id());
	
	printk(KERN_INFO "SIMPLETIMER: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(simple_timer_init);
module_exit(simple_timer_exit);
