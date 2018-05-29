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


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "processstatus"


//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple ProcFS module, which could count the number of running processes with various states on the system");
MODULE_VERSION("1.0.2");


//Creating a proc directory entry structure
static struct proc_dir_entry* our_proc_file;
static int counter = 0;
static int task_running = 0, task_interruptible = 0, task_unintrruptible = 0, task_stopped = 0;
static int task_traced = 0, task_dead = 0, task_zombie = 0, task_unknown = 0;

static void process_state(struct task_struct *task){
	if(task){
		switch(task->state){
      			case TASK_RUNNING:
				task_running++;
				break;
      			case TASK_INTERRUPTIBLE:
				task_interruptible++;
				break;
      			case TASK_UNINTERRUPTIBLE:
				task_unintrruptible++;
				break;
			case __TASK_STOPPED:
				task_stopped++;
				break;
			case __TASK_TRACED:
				task_traced++;
				break;
			case EXIT_DEAD:
				task_dead++;
				break;
			case EXIT_ZOMBIE:
				task_zombie++;
				break;
			default:
				task_unknown++;
			}
		}
}


//Now there is a re-written version of simply used for_each_process loop
static void browse_processes(struct task_struct *curr_task, struct seq_file *m){
	struct task_struct *task_next;
	struct list_head *list;

	//This loop will go around the whole task_struct list in which the Kernel holds all the processes on the system
	list_for_each(list, &curr_task->children){
		task_next = list_entry(list, struct task_struct, sibling);
		process_state(task_next);
		counter++;
		browse_processes(task_next, m);
	}  
}


//This function calls on demand of read request from seq_files
static int proc_show(struct seq_file *m, void *v){
	printk(KERN_INFO "PROCESSSTATUS: Generating output for user space with seq_files.\n");
	//We are going to count processes which are currently running on the context of the Kernel
	static struct task_struct *curr_task, *init_task;
	task_running = 0;
	task_interruptible = 0;
	task_unintrruptible = 0;
	task_stopped = 0;
	task_traced = 0;
	task_dead = 0;
	task_zombie = 0;
	task_unknown = 0;
	counter = 0;
	
	//First capture the pointer to the current process
	curr_task = get_current();
	//With this loop we can find the init_process
	for(init_task = curr_task; init_task->pid != 0; init_task = init_task->parent);
	browse_processes(init_task, m);
	
	//Now print the essential output
	seq_printf(m, "Total: %d\nRunning: %d\nInterruptible: %d\nUninterruptible: %d\nStopped: %d\nTraced: %d\nDead: %d\nZombie: %d\nUnknown: %d\n",
		counter, task_running, task_interruptible, task_unintrruptible, task_stopped,
		task_traced, task_dead, task_zombie, task_unknown);
	return SUCCESS;
}


//This is where system functionallity triggers every time some process try to read from our proc entry
static int proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "PROCESSSTATUS: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	printk(KERN_INFO "PROCESSSTATUS: Open Function, USER \"UID:%i\"\n", get_current_user()->uid.val);
	return single_open(file, proc_show, NULL);
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


//Your module's entry point
static int __init processstatus_procfs_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "PROCESSSTATUS: Initialization.\n");
	printk(KERN_INFO "PROCESSSTATUS: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);


	our_proc_file = proc_create(DEVICE_NAME, 0644 , NULL, &fops);
	//Put an error message in kernel log if cannot create proc entry
	if(!our_proc_file){
		printk(KERN_ALERT "PROCESSSTATUS: Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}

	printk(KERN_INFO "PROCESSSTATUS: /proc/%s has been created.\n", DEVICE_NAME);
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit processstatus_procfs_exit(void){
	//Initiating module unloading procedure
	printk(KERN_INFO "PROCESSSTATUS: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Remove proc filesystem entry from system
	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "PROCESSSTATUS: /proc/%s has been removed.\n", DEVICE_NAME);

	printk(KERN_INFO "PROCESSSTATUS: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}

//Now we need to define init-module and cleanup_module aliases
module_init(processstatus_procfs_init);
module_exit(processstatus_procfs_exit);
