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
//For using task_struct
#include <linux/sched.h>
//For macro UTS_RELEASE
#include <generated/utsrelease.h>
//for macros LINUX_KERNEL_CODE and KERNEL_VERSION
#include <linux/version.h>
//For get_current_user
#include <linux/cred.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "helloworldprocfs"


//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Hello World using /proc filesystem device driver, which could count the number of running processes on the system");
MODULE_VERSION("1.0.2");


static int ratelimitflag = 1;
module_param(ratelimitflag, int, 0);
MODULE_PARM_DESC(ratelimitflag, "This command line argument will [Enable:1] or disable:0 rate_limit function");


//Creating a proc directory entry structure
static struct proc_dir_entry* our_proc_file;
static int counter = 0;



//Now there is a re-written version of simply used for_each_process loop
void browse_processes(struct task_struct *curr_task, struct seq_file *m){
	struct task_struct *task_next;
	struct list_head *list;

	//This loop will go around the whole task_struct list in which the Kernel holds all the processes on the system
	list_for_each(list, &curr_task->children){
		task_next = list_entry(list, struct task_struct, sibling);
		//printk_ratelimit will stop producing logs when you reach the limit which idicated (and could be set) in
		//    /proc/sys/kernel/printk_ratelimit_burst

		//Also this interrupt will last for at least as equal to the time specified (and could be set) in
		//    /proc/sys/kernel/printk_ratelimit
		if(ratelimitflag){
			if(printk_ratelimit())
				printk(KERN_INFO "HELLOWORLDPROCFS: Process \"%s:%d\"\n", task_next->comm, task_next->pid);
			}
		else
			printk(KERN_INFO "HELLOWORLDPROCFS: Process \"%s:%d\"\n", task_next->comm, task_next->pid);

		//You can uncomment the line bellow if you want to see all processes info in the proc file
		//seq_printf(m, "HELLOWORLDPROCFS: Process \"%s:%d\" in %ld state\n", iTask->comm, iTask->pid, iTask->state);
	
		counter++;
		browse_processes(task_next, m);
	}  
}


//This function calls on demand of read request from seq_files
static int proc_show(struct seq_file *m, void *v){
	printk(KERN_INFO "HELLOWORLDPROCFS: Generating output for user space with seq_files.\n");
	//We are going to count processes which are currently running on the context of the Kernel
	static struct task_struct *curr_task, *init_task;
	counter = 0;
	
	//First capture the pointer to the current process
	curr_task = get_current();
	//With this loop we can find the init_process
	for(init_task = curr_task; init_task->pid != 0; init_task = init_task->parent);
	browse_processes(init_task, m);
	
	//Now print the essential hello world output
	seq_printf(m, "Hello World along side with %d processes :)\n", counter);
	return SUCCESS;
}


//This is where system functionallity triggers every time some process try to read from our proc entry
static int proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "HELLOWORLDPROCFS: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	printk(KERN_INFO "HELLOWORLDPROCFS: Open Function, USER \"UID:%i\"\n", get_current_user()->uid.val);
	return single_open(file, proc_show, NULL);
}


static int module_permission(struct inode *inode, int operation){
	printk(KERN_INFO "HELLOWORLDPROCFS: Permission Function, USER \"UID:%i\"\n", get_current_user()->uid.val);
	//Check the Operation, Read is permitted, Write is permitted only for UID 0 which indicates root user
	if(operation == 4 || (operation == 2 && get_current_user()->uid.val == 0))
		return 0;

	//Otherwise, it is not permitted
	return -EACCES;
}

//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the corresponding /proc entry
static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


//This is for older kernel versions and check write permission on files
//In newer version we could use capabilities of users which we will use in
//other modules later
static const struct inode_operations iops = {
	.permission = module_permission,
};


//Your module's entry point
static int __init helloworld_procfs_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "HELLOWORLDPROCFS: Initialization.\n");
	printk(KERN_INFO "HELLOWORLDPROCFS: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//LINUX_VERSION_CODE macro can obtain current version of Kernel in hexadecimal form
	//you can also create the same format from a known kernel version such as 2.6.10 or 4.12.0
	//by using KERNEL_VERSION macro, by comaring them it would be obvious that you can choose
	//what version of function might be use in your code
	//Another important macro here might be UTS_RELEASE which return the Kernel version in dotted-decimal-string format
	//Now depends on our Kernel version we could use a diffrent command and put specific comment in the Kernel log

	#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 10)
		printk(KERN_INFO "HELLOWORLDPROCFS: Hello OLD Kernel %s, So Use create_proc_entry Function\n", UTS_RELEASE);
		our_proc_file = create_proc_entry(DEVICE_NAME, 0644 , NULL);
		our_proc_file->proc_fops = &fops;
		our_proc_file->proc_iops = &iops;
	#else

		printk(KERN_INFO "HELLOWORLDPROCFS: Hello NEW Kernel %s, So Use proc_create Function\n", UTS_RELEASE);
		our_proc_file = proc_create(DEVICE_NAME, 0644 , NULL, &fops);

	#endif
	//Put an error message in kernel log if cannot create proc entry
	if(!our_proc_file){
		printk(KERN_ALERT "HELLOWORLDPROCFS: Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}

	printk(KERN_INFO "HELLOWORLDPROCFS: /proc/%s has been created.\n", DEVICE_NAME);
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit helloworld_procfs_exit(void){
	//Initiating module unloading procedure
	printk(KERN_INFO "HELLOWORLDPROCFS: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Remove proc filesystem entry from system
	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "HELLOWORLDPROCFS: /proc/%s has been removed.\n", DEVICE_NAME);

	printk(KERN_INFO "HELLOWORLDPROCFS: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}

//Now we need to define init-module and cleanup_module aliases
module_init(helloworld_procfs_init);
module_exit(helloworld_procfs_exit);
