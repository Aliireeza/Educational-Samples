//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//For using I/O ports
#include <linux/ioport.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "iomemsmaple"


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is an I/O Memory module, which could check wether an I/O Memory Region is free at the moment or not");
MODULE_VERSION("1.0.2");


//module parameters with default value of something
static unsigned long our_mem_first = 0xe5000000, our_mem_number = 0x100;
module_param(our_mem_first, long, 0);
MODULE_PARM_DESC(our_mem_first, "The first address of I/O memory region");
module_param(our_mem_number, long, 0);
MODULE_PARM_DESC(our_mem_number, "the number of bytes in requested region");
static int our_result = 0;

//Your module's entry point
static int __init io_memory_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "IOMEMSAMPLE: Initialization.\n");

	//check_mem_region is depricated now, So everything is so simpler ;)
	//if(check_mem_region(our_mem_first, our_mem_number)){
		//This means the check_region function return non-zero value, which means an error
		//printk(KERN_INFO "IOMEMPROBE: The requested region (from %d to %d) is NOT Available at the moment\n", our_mem_first, our_mem_first + our_mem_number - 1);
	//}
	//else{
		//This means the check_region function return zero value, which means the port is available
		//Now we want to actually register this port
		if(!request_mem_region(our_mem_first, our_mem_number, DEVICE_NAME)){
			//This means the function return a NULL pointer, which indicates a faliure
			printk(KERN_ALERT "IOMEMSAMPLE: The requested region (from %X to %X) Could Not Be Registered at the moment\n", our_mem_first, our_mem_first + our_mem_number - 1);
		}
		else{
			//Ok, It has been registered successfully here
			printk(KERN_INFO "IOMEMSAMPLE: The requested region (from %X to %X) Registered to %s\n", our_mem_first, our_mem_first + our_mem_number - 1, DEVICE_NAME);
			our_result = 1;
		}
	//}

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}

//You sould clean up the mess before exiting the module
static void __exit io_memory_exit(void){
	//Initiating module unloading procedure
	//We have to release the I/O memory regions
	if(our_result){
		release_mem_region(our_mem_first, our_mem_number);
		printk(KERN_INFO "IOMEMSAMPLE: The registred region (from %X to %X) has been Freed\n", our_mem_first, our_mem_first + our_mem_number - 1);
	}

	//And, That is it
	printk(KERN_INFO "IOMEMSAMPLE: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}

//Now we need to define init-module and cleanup_module aliases
module_init(io_memory_init);
module_exit(io_memory_exit);
