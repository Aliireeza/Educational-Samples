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
//For copy_to_user, copy_from_user, put_user
#include <asm/uaccess.h>

//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "processtree"
//This is the constant that used for determination of buffer length
#define MAX_BUF_LEN 4096

//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple procfs module, which could manipulate the running processes on the system");
MODULE_VERSION("1.0.2");


//Creating two proc directory entry structure
static struct proc_dir_entry *task_proc_file, *tree_proc_file;
static struct proc_dir_entry *next_proc_file, *perv_proc_file;
static struct proc_dir_entry *curr_proc_file, *parent_proc_file;

//Now we have to create a buffer for our longest message (MAX_BUF_LEN)
//and a variable to count our actual message size
static char procfs_buffer[MAX_BUF_LEN];
static unsigned long procfs_buffer_size = 0;

//Here we store target pid and name
static int our_task_pid = -1;
static char our_task_comm[30] = "";
static struct task_struct *our_task;

//These command line arguments are defined for define how this module works
static int work_type = 1;
module_param(work_type, int, 0);
MODULE_PARM_DESC(work_type, "Using PID:1 [Default], Using Process Name:0");


static void process_state(struct task_struct *task, char *state){
	if(task){
		switch(task->state){
			case TASK_RUNNING:
				strcpy(state, "RUNNING");
				break;
			case TASK_INTERRUPTIBLE:
				strcpy(state, "INTERRUPTIBLE");
				break;
			case TASK_UNINTERRUPTIBLE:
				strcpy(state, "UN-INTERRUPTIBLE");
				break;
			case __TASK_STOPPED:
				strcpy(state, "STOPPED");
				break;
			case __TASK_TRACED:
				strcpy(state, "TRACED");
				break;
			case EXIT_DEAD:
				strcpy(state, "DEAD");
				break;
			case EXIT_ZOMBIE:
				strcpy(state, "ZOMBIE");
				break;
			default:
				strcpy(state, "UNKNOWN");
			}
		}
}


//This function will print out a process information
static void print_process_info(struct seq_file *m, struct task_struct *task){
	struct list_head *child_list;
	char state[20];
	struct task_struct *parent, *child;
	
	if(task){
		process_state(task, state);
		//Now we are going to print information about this task
		seq_printf(m, "T:%s:%d:%s\n", task->comm, task->pid, state);
		//The, Its parent
		parent = task->parent;
		if(parent){
			process_state(parent, state);
			seq_printf(m, "P:%s:%d:%s\n\n", parent->comm, parent->pid, state);
			}
		else
			seq_printf(m, "Unknown Parent\n\n");

      		//Finally, print information of each child
      		list_for_each(child_list, &task->children){
      			child = list_entry(child_list, struct task_struct, sibling);
      			process_state(child, state);
			seq_printf(m, "C:%s:%d:%s\n", child->comm, child->pid, state);
		}
	}
	//Capturing error, if for any reason you could not obtain next descriptor
  	else{
    		seq_printf(m, "Error in opening the task_struct\n");
  	}
}


//Now there is a re-written version of simply used for_each_process loop
static void browse_processes(struct seq_file *m, struct task_struct *task){
	struct task_struct *task_next;
	struct list_head *list;
	//This loop will go around the whole task_struct list in which the Kernel holds all the processes on the system
	list_for_each(list, &task->children){
		task_next = list_entry(list, struct task_struct, sibling);
		print_process_info(m, task_next);
		browse_processes(m, task_next);
	}
}


static struct task_struct * find_task(struct task_struct *task, int pid , char *comm){
	struct task_struct *task_next, *found_task;
	struct list_head *list;
	int FOUND_FLAG = 0;
	
	list_for_each(list, &task->children){
		task_next = list_entry(list, struct task_struct, sibling);
		if(task_next->pid == pid || strcmp(task_next->comm, comm) == 0){
			FOUND_FLAG = 1;
			found_task = task_next;
			break;
		}
		find_task(task_next, pid, comm);
	}
		
	if(FOUND_FLAG == 1)
		return found_task;
	else
		return NULL;
}


//This function will go around the whole task_struct list in which the Kernel holds all the processes on the system
static struct task_struct * search_for_each_task(int pid, char *comm){
	struct task_struct *curr_task, *init_task;
		
	//First capture the pointer to the current process
	curr_task = get_current();
	//With this loop we can find the init_process
	for(init_task = curr_task; init_task->pid != 0; init_task = init_task->parent);
	
	//Now we have to go through whole tasks from the begining to find the target task_struct
	return find_task(init_task, pid, comm);
}


static struct task_struct * next_task(struct task_struct *task){
	//We need to find the next task manually
	return task;
}


static struct task_struct * perv_task(struct task_struct *task){
	//We need to find the previous task manually
	return task;
}


//This function will recieve user input
//Each time user try to echo something or otherwise write anything to the /proc entry, this function does the job
static ssize_t task_proc_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	int j = 0;
	printk(KERN_INFO "PROCESSTREE: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);
	//get buffer size
	if (length > MAX_BUF_LEN)
		procfs_buffer_size = MAX_BUF_LEN;
	else
		procfs_buffer_size = length;
	//write data to the buffer
	if (raw_copy_from_user(procfs_buffer, buffer, procfs_buffer_size))
		return -EFAULT;

	//Assigning input data to target process
	if(work_type == 1){ //This means working with pid
		our_task_pid = 0;
		//Now we have to convert string input to some integer
		for(j=0;j<procfs_buffer_size-1 && procfs_buffer[j] != '\0';j++)
			our_task_pid = our_task_pid * 10 + (int)(procfs_buffer[j] - 48);
		printk(KERN_INFO "PROCESSTREE: Input PID is %d\n", our_task_pid);
		}
	else{ //This means working with process name
		strncpy(our_task_comm, procfs_buffer, procfs_buffer_size-1);
		our_task_pid = -2;
		}

	//The function returns wrote charachters count
	printk(KERN_INFO "PROCESSTREE: Write %lu bytes\n", procfs_buffer_size);
	return procfs_buffer_size;
}


//This function calls on demand of read request from seq_files
static int task_proc_show(struct seq_file *m, void *v){
	//We are going to collect info about some processes which are currently running on the context of the Kernel
	struct task_struct *task;

	//IF this function calls without any input for the first time, or something went wrong in write function
	if(our_task_pid == -1){
		//Set target info to current process
		our_task_pid = current->pid;
		strcpy(our_task_comm, current->comm);
	}
	
	printk(KERN_INFO "PROCESSTREE: Looking For PID: %d, Name: %s\n", our_task_pid, our_task_comm);
	//If this function calls without any argument not for the first time or On the other hand we try to find the task
	if(our_task_pid == current->pid || strcmp(our_task_comm, current->comm) == 0)
		task = get_current();
	else
		task = search_for_each_task(our_task_pid, our_task_comm);


	//In case of being unable to find correct target process
	if(!task){
		printk(KERN_ALERT "PROCESSTREE: PID %d Has Not Being Found\n", our_task_pid);
		our_task_pid = -1;
		strcpy(our_task_comm, "");
		return -EFAULT;
		}
	else{
		printk(KERN_INFO "PROCESSTREE: PID %d Has Being Found\n", task->pid);
		print_process_info(m, task);
		our_task_pid = -1;
		strcpy(our_task_comm, "");
		return SUCCESS;
	}
}


static int tree_proc_show(struct seq_file *m, void *v){
	struct task_struct *curr_task, *init_task;
	
	//First capture the pointer to the current process
	curr_task = get_current();
	//With this loop we can find the init_process
	for(init_task = curr_task; init_task->pid != 0; init_task = init_task->parent);
	browse_processes(m, init_task);
	return SUCCESS;
}


static int perv_proc_show(struct seq_file *m, void *v){
	our_task = perv_task(our_task);
	print_process_info(m, our_task);
	return SUCCESS;
}

static int next_proc_show(struct seq_file *m, void *v){
	our_task = next_task(our_task);
	print_process_info(m, our_task);
	return SUCCESS;
}

static int parent_proc_show(struct seq_file *m, void *v){
	our_task = our_task->parent;
	print_process_info(m, our_task);
	return SUCCESS;
}

static int current_proc_show(struct seq_file *m, void *v){
	our_task = get_current();
	print_process_info(m, our_task);
	return SUCCESS;
}


//This is where system functionallity triggers every time some process try to read from our proc entry
static int tree_proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "PROCESSTREE: Tree Seqfile Read, Process \"%s:%d\" \n", current->comm, current->pid);
	return single_open(file, tree_proc_show, NULL);
}

static int task_proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "PROCESSTREE: Tree Seqfile Read, Process \"%s:%d\" \n", current->comm, current->pid);
	return single_open(file, task_proc_show, NULL);
}

static int parent_proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "PROCESSTREE: Parent Seqfile Read, Process \"%s:%d\" \n", current->comm, current->pid);
	return single_open(file, parent_proc_show, NULL);
}

static int current_proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "PROCESSTREE: Current Seqfile Read, Process \"%s:%d\" \n", current->comm, current->pid);
	return single_open(file, current_proc_show, NULL);
}

static int next_proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "PROCESSTREE: Next Seqfile Read, Process \"%s:%d\" \n", current->comm, current->pid);
	return single_open(file, next_proc_show, NULL);
}

static int perv_proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "PROCESSTREE: Perv Seqfile Read, Process \"%s:%d\" \n", current->comm, current->pid);
	return single_open(file, perv_proc_show, NULL);
}

//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the correspond /proc entry
static const struct file_operations tree_fops = {
	.open = tree_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations task_fops = {
	.open = task_proc_open,
	.read = seq_read,
	.write = task_proc_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations curr_fops = {
	.open = current_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations next_fops = {
	.open = next_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations perv_fops = {
	.open = perv_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const struct file_operations parent_fops = {
	.open = parent_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


//You sould clean up the mess before exiting the module
static void __exit processtree_exit(void){
	//Remove proc filesystem entry from system
	printk(KERN_INFO "PROCESSTREE: Cleanup Module, Process \"%s:%d\" \n", current->comm, current->pid);

	if(tree_proc_file)
		remove_proc_entry("processtree", NULL);
	if(task_proc_file)
		remove_proc_entry("processinfo", NULL);
	if(curr_proc_file)
		remove_proc_entry("processcurrent", NULL);
	if(parent_proc_file)
		remove_proc_entry("processparent", NULL);
	if(next_proc_file)
		remove_proc_entry("processnext", NULL);
	if(perv_proc_file)
		remove_proc_entry("processperv", NULL);
		
	printk(KERN_INFO "PROCESSTREE: ProcFS files have been removed.\n");
	printk(KERN_INFO "PROCESSTREE: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Your module's entry point
static int __init processtree_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "PROCESSTREE: Initialization.\n");
	our_task = get_current();
	printk(KERN_INFO "PROCESSTREE: Init Module, Process \"%s:%d\" \n", current->comm, current->pid);

	//We have to register our procfs entries one by one
	tree_proc_file = proc_create("processtree", 0644 , NULL, &tree_fops);
	task_proc_file = proc_create("processinfo", 0644 , NULL, &task_fops);
	next_proc_file = proc_create("processnext", 0644 , NULL, &next_fops);
	perv_proc_file = proc_create("processperv", 0644 , NULL, &perv_fops);
	parent_proc_file = proc_create("processparent", 0644 , NULL, &parent_fops);
	curr_proc_file = proc_create("processcurrent", 0644 , NULL, &curr_fops);
	
	//Put an error message in kernel log if cannot create proc entry
	if(!task_proc_file | !tree_proc_file | !curr_proc_file | !parent_proc_file | !next_proc_file | !perv_proc_file){
		printk(KERN_ALERT "PROCESSTREE: ProcFS Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		processtree_exit();
		return -ENOMEM;
	}
	printk(KERN_INFO "PROCESSTREE: ProcFS files have been created.\n");

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//Now we need to define init-module and cleanup_module aliases
module_init(processtree_init);
module_exit(processtree_exit);
