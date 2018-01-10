//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//For jiffies, which will give us timelapce of the system
#include <linux/jiffies.h>
//For catching CPU number
#include <linux/smp.h>
//Ofcourse for using workqueues
#include <linux/workqueue.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "workqueuesample"

//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple module which workqueues for educationa purpose");
MODULE_VERSION("1.0.2");


//Define a work
static struct workqueue_struct *our_workqueue;

//Here are some useful variables
static unsigned long before_delay, after_delay;


//This is our work function which we wanted to queue
static void our_work_function(struct work_struct *our_work){
	after_delay = jiffies;
	printk(KERN_INFO "WORKQUEUESAMPLE: Work function is running on CPU %d \n", smp_processor_id());
	printk(KERN_INFO "WORKQUEUESAMPLE: Work function scheduled on: %ld \n", before_delay);
	printk(KERN_INFO "WORKQUEUESAMPLE: Work function executed on: %ld \n", after_delay);
}


static DECLARE_DELAYED_WORK(our_work, our_work_function);


//Your module's entry point
static int __init workqueue_module_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "WORKQUEUESAMPLE: Initialization.\n");

	before_delay = jiffies;
	//Then we have to initiate a work

	our_workqueue = create_singlethread_workqueue("ourqueue");
	if(!our_workqueue){
		printk(KERN_ALERT "WORKQUEUESAMPLE: Creating the workqueue has been failed!\n");
		return -EFAULT;
	}
	printk(KERN_INFO "WORKQUEUESAMPLE: Workqueue has been created\n");
	
	//Then we are going to queue our work
	queue_delayed_work(our_workqueue, &our_work, HZ);
	printk(KERN_INFO "WORKQUEUESAMPLE: our work function and workqueue initiated on CPU %d \n", smp_processor_id());
	

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit workqueue_module_exit(void){
	//Now we have to get rid of our workqueue, so first we cancel the work,
	cancel_delayed_work(&our_work);
	if(our_workqueue)
		destroy_workqueue(our_workqueue);
	printk(KERN_INFO "WORKQUEUESAMPLE: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(workqueue_module_init);
module_exit(workqueue_module_exit);
