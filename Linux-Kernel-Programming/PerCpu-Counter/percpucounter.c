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
//For finding the parent process ID of the module
#include <asm/current.h>
//For "struct task_struct"
#include <linux/sched.h>
//For using string header in kernel space
#include <linux/string.h>
//For catching CPU number
#include <linux/smp.h>
//For PER-CPU variables and functions
#include <linux/percpu.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "percpucounter"


//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple procfs module which uses per-cpu variable to count their own  functions calls on each cpu.");
MODULE_VERSION("1.0.2");


//Creating a proc directory entry structure
static struct proc_dir_entry *statistics_proc_file, *percpu_proc_file;
//Now we define our PER-CPU variable
DEFINE_PER_CPU(int, count);
//Capturing Last PER-CPU variable number which changed
static int last;


//These functions calls on demand of read request from seq_files
//This function used to show some statistics about PER-CPU variables
static int proc_statistics_show(struct seq_file *m, void *v){
	static int i, total = 0;
	//Here we try to generate a report for output, showing each CPU's copy of a PER_CPU variable
	for_each_possible_cpu(i){
		total += per_cpu(count, i);
		seq_printf(m, "%d CPU\t%d PER-CPU\t%c\n", i, per_cpu(count, i), (i == last? '+': ' '));
	}
	//And this is a sum of all PER-CPU variables
	seq_printf(m,"Counter Total Rusults: %d\n", total);

	//Now we create a sharp simple output
	return SUCCESS;
}

//This function is used to see the current CPU counter
static int proc_percpu_show(struct seq_file *m, void *v){
	//Here, we simply use two macros to work with this CPU copy of PER-CPU variable
	get_cpu_var(count)++;
	put_cpu_var(count);
	//Then we remember the last processor which acted on PER-CPU variables
	last = smp_processor_id();
	printk(KERN_INFO "PERCPUCOUNTER: PER-CPU Variable Incremented on CPU %d\n", last);
	//And create a single line output
	seq_printf(m, "%d PER-CPU, %d CPU\n", get_cpu_var(count), last);
	//Now we create a sharp simple output
	return SUCCESS;
}


//This is where system functionallity triggers every time some process try to read from our proc entry
static int proc_statistics_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "PERCPUCOUNTER: Statictics Seqfile Read, Process \"%s:%d\"\n", current->comm, current->pid);
	return single_open(file, proc_statistics_show, NULL);
}

static int proc_percpu_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "PERCPUCOUNTER: Per-CPU Seqfile Read, Process \"%s:%d\" \n", current->comm, current->pid);
	return single_open(file, proc_percpu_show, NULL);
}


//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the correspond /proc entry
static const struct file_operations statistics_fops = {
	.open = proc_statistics_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations percpu_fops = {
	.open = proc_percpu_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


//Your module's entry point
static int __init percpucounter_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "PERCPUCOUNTER: Initialization.\n");
	printk(KERN_INFO "PERCPUCOUNTER: Init Module, Process \"%s:%d\" \n", current->comm, current->pid);
	//It is time to register two procfs entry and their functionallity with the kernel
	statistics_proc_file = proc_create("percpustatistics", 0644 , NULL, &statistics_fops);
	//Put an error message in kernel log if cannot create proc entry
	if(!statistics_proc_file){
		printk(KERN_ALERT "PERCPUCOUNTER: Per-CPU Statistics Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}
	printk(KERN_INFO "PERCPUCOUNTER: /proc/percpustatistics has been created.\n");

	percpu_proc_file = proc_create("percpucounter", 0644 , NULL, &percpu_fops);
	//Put an error message in kernel log if cannot create proc entry
	if(!percpu_proc_file){
		printk(KERN_ALERT "PERCPUCOUNTER: Per-CPU Counter Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}
	printk(KERN_INFO "PERCPUCOUNTER: /proc/percpucounter has been created.\n");

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit percpucounter_exit(void){
	//Remove proc filesystem entry from system
  	printk(KERN_INFO "PERCPUCOUNTER: Cleanup Module, Process \"%s:%d\" \n", current->comm, current->pid);

	remove_proc_entry("percpustatistics", NULL);
	printk(KERN_INFO "PERCPUCOUNTER: /proc/percpustatistics has been removed.\n");

	remove_proc_entry("percpucounter", NULL);
	printk(KERN_INFO "PERCPUCOUNTER: /proc/percpucounter has been removed.\n");

	printk(KERN_INFO "PERCPUCOUNTER: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(percpucounter_init);
module_exit(percpucounter_exit);
