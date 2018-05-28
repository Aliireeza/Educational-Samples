//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//For emergency_restart function
#include <linux/reboot.h>

//It is always good to have a meaningful constant as a return code
#define SUCCESS 0

//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is the most useless Kernel module ever, insmod it and your system will halt ;)");
MODULE_VERSION("1.0.0");



//Your module's entry point
static int __init halt_module_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "REBOOTMODULE: Initialization.\n");

	//Now this is the correct time for an emergency halt ;)
	machine_halt();
	//you could also call machine_power_off()
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit halt_module_exit(void){
	//This function will never call ;)
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}

//Now we need to define init-module and cleanup_module aliases
module_init(halt_module_init);
module_exit(halt_module_exit);
