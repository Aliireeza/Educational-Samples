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

//This will be our module name
#define MY_MODULE_NAME "helloworld"


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple HelloWorld module, with the aim of creating an introduction to kernel module programming");
MODULE_VERSION("1.0.2");

static char *hellostring = "World";
module_param(hellostring, charp, 0000);
MODULE_PARM_DESC(hellostring, "A string value to print your name in kernel space");

//Every Module must have an entry point which is known as init_module or
//some function that defines with module_init macros. init_module is the function
//that would call immidately after insmoding your module to set everything right
//and registering module functionality to the Kernel
static int __init hello_world_init(void){
	//These messages will not show to the user but instead they will apear in the kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command

	//KERN_EMERG 		Emergency condition, system is probably dead
	//KERN_ALERT 		Some problem has occurred, immediate attention is needed
	//KERN_CRIT 		A critical condition
	//KERN_ERR 		An error has occurred
	//KERN_WARNING 		A warning
	//KERN_NOTICE 		Normal message to take note of
	//KERN_INFO 		Some information
	//KERN_DEBUG		Debug information related to the program

	printk(KERN_ALERT "HELLOWORLD: Hello %s!\n", hellostring);
	printk(KERN_INFO "HELLOWORLD: Module \"%s\" Loading,  By Process \"%s:%i\"\n", MY_MODULE_NAME, current->comm, current->pid);

	//The init_module should return a value to the rest of The Kernel, that asure
	//other modules and system calls about the successfull registration of its functionality
	return 0;
}


//Every Module must have an exit point which is known as cleanup_module or
//some function that defines with module_exit macros. cleanup_module is the function
//that would call immidately after rmmoding your module to undone every settings
//that you made in init_module and clean the slate
static void __exit hello_world_exit(void){
	printk(KERN_INFO "HELLOWORLD: Now we have to undone everything after rmmoding %s module\n", MY_MODULE_NAME);
	printk(KERN_INFO "HELLOWORLD: Module \"%s\" Unloading, By Process \"%s:%i\"\n", MY_MODULE_NAME, current->comm, current->pid);
	printk(KERN_ALERT "HELLOWORLD: GoodBye %s.\n", hellostring);
	//The cleanup_module function doesn't need to return any value to the rest of the Kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(hello_world_init);
module_exit(hello_world_exit);
