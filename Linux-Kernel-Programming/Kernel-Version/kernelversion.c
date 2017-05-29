//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for some macros
#include <linux/init.h>
//For finding the parent process ID of the module
#include <asm/current.h>
//For "struct task_struct" that contains current process
#include <linux/sched.h>
//For macro UTS_RELEASE
#include <generated/utsrelease.h>
//for macros LINUX_KERNEL_CODE and KERNEL_VERSION
#include <linux/version.h>

//This will be our module name
#define MY_MODULE_NAME "kernelversion"


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Kernel module, that can distinguish between various Kernel versions");
MODULE_VERSION("1.0.2");


//Every Module must have an entry point which is known as init_module or
//some function that defines with module_init macros. init_module is the function
//that would call immidately after insmoding your module to set everything right
//and registering module functionality to the Kernel
static int __init kernel_version_init(void){
	//LINUX_VERSION_CODE macro can obtain current version of Kernel in hexadecimal form
	//you can also create the same format from a known kernel version such as 2.6.10 or 4.12.0
	//by using KERNEL_VERSION macro, by comaring them it would be obvious that you can choose
	//what version of function might be use in your code
	//Another important macro here might be UTS_RELEASE which return the Kernel version in dotted-decimal-string format

	printk(KERN_INFO "KERNELVERSION: Module \"%s\" Loading, By Process \"%s:%i\"\n", MY_MODULE_NAME, current->comm, current->pid);

	//Now depends on our Kernel version we could log a diffrent comment in the Kernel log
	#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 10)
		printk(KERN_INFO "KERNELVERSION: Hello OLD Kernel %s\n", UTS_RELEASE);
	#elif  LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)
		printk(KERN_INFO "KERNELVERSION: Hello NEW Kernel %s\n", UTS_RELEASE);
	#else
		printk(KERN_INFO "KERNELVERSION: Hello Moderate Kernel %s\n", UTS_RELEASE);
	#endif

	//The init_module should return a value to the rest of The Kernel, that asure
	//other modules and system calls about the successfull registration of its functionality
	return 0;
}


//Every Module must have an exit point which is known as cleanup_module or
//some function that defines with module_exit macros. cleanup_module is the function
//that would call immidately after rmmoding your module to undone every settings
//that you made in init_module and clean the slate
static void __exit kernel_version_exit(void){
	printk(KERN_INFO "KERNELVERSION: Module \"%s\" Unloading, By Process \"%s:%i\"\n", MY_MODULE_NAME, current->comm, current->pid);
	printk(KERN_INFO "KERNELVERSION: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the Kernel
}

//Now we need to define init-module and cleanup_module aliases
module_init(kernel_version_init);
module_exit(kernel_version_exit);
