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
//For using proc filesystem
#include <linux/proc_fs.h>
//For using sequence files
#include <linux/seq_file.h>

//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "freeinterrups"

//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("Simple Interrupt Probe, which show how many interrupts has been registered the moment for some interrupt types");
MODULE_VERSION("1.0.2");

//Here are some useful variables
static struct proc_dir_entry* proc_register;
static int interrupt_timer_count, interrupt_shared_count;
static int interrupt_percpu_count, interrupt_oneshot_count;

//This is our eloquent interrut handler
static irqreturn_t sample_handler(int irq, void *dev_id){
	//check device id
	//schedule a deferred work
	//return acknowledgement
	return IRQ_HANDLED;
}

//Printing everything in order to a userspace process demand on reading our proc entry
static int procfs_show(struct seq_file *m, void *v){
	//Lets look at the calling process
	printk(KERN_INFO "FREEINTERRUPTS: Process \"%s\" with PID \"%i\" called procfs entry.\n", current->comm, current->pid);
	//Then print out the interrupt lines
	seq_printf(m, "Timer:%d\nShared:%d\nPerCPU:%d\nOneShot:%d\nTotal:%d\n", interrupt_timer_count, interrupt_shared_count, interrupt_percpu_count, interrupt_oneshot_count, NR_IRQS);
	return 0;
}


//Do the math here and call the show function when ever a userspace process demands
static int procfs_open(struct inode *inode, struct file *file){
	//First we have to probe all available shared interrupt lines
	int i;
	interrupt_timer_count=0;interrupt_percpu_count=0;
	interrupt_shared_count=0;interrupt_oneshot_count=0;
	for(i=0; i < NR_IRQS; i++){
		if(request_irq(i, sample_handler, IRQF_TIMER, DEVICE_NAME, NULL)==0){
			interrupt_timer_count++;
			free_irq(i, NULL);
			}
		if(request_irq(i, sample_handler, IRQF_SHARED | IRQF_NO_SUSPEND, DEVICE_NAME, NULL)==0){
			interrupt_shared_count++;
			free_irq(i, NULL);
			}
		if(request_irq(i, sample_handler, IRQF_PERCPU, DEVICE_NAME, NULL)==0){
			interrupt_percpu_count++;
			free_irq(i, NULL);
			}
		if(request_irq(i, sample_handler, IRQF_ONESHOT, DEVICE_NAME, NULL)==0){
			interrupt_oneshot_count++;
			free_irq(i, NULL);
			}
	}
	//Now its time to print the results on the screen
	return single_open(file, procfs_show, NULL);
}


//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the corresponding /proc entry
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = procfs_open,
	.llseek = seq_lseek,
	.read = seq_read,
	.release = single_release,
};


//Your module's entry point
static int __init freeinterrupts_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "FREEINTERRUPTS: Initialization.\n");
	printk(KERN_INFO "FREEINTERRUPTS: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//First we are going to create our proc folder
	proc_register = proc_create(DEVICE_NAME, 0644 , NULL, &fops);
	//Put an error message in kernel log if cannot create proc entry
	if(!proc_register){
		printk(KERN_ALERT "FREEINTERRUPTS: Error, not enough memory, so cannot create entry under /proc/%s.\n", DEVICE_NAME);
		return -ENOMEM;
	}
	printk(KERN_INFO "FREEINTERRUPTS: Module entry created under /proc/%s.\n", DEVICE_NAME);


	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}



//You sould clean up the mess before exiting the module
static void __exit freeinterrupts_exit(void){
	//Initiating module unloading procedure
	printk(KERN_INFO "FREEINTERRUPTS: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Remove proc filesystem entry from system
	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "FREEINTERRUPTS: Module entry removed from /proc/%s.\n", DEVICE_NAME);
	
	printk(KERN_INFO "FREEINTERRUPTS: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(freeinterrupts_init);
module_exit(freeinterrupts_exit);
