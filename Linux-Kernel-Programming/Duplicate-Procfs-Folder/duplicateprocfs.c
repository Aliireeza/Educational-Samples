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
//For getting system information
#include <linux/utsname.h>
//For using diffrent time related functions
#include <linux/time.h>
//For obtaining CPU status
#include <linux/kernel_stat.h>
//For counting jiffies and HZ
#include <linux/jiffies.h>
//For using cputime_to_timespec function
#include <asm/cputime.h>
//For ktime_get_ts function to obtain uptime
#include <linux/timekeeping.h>

//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "duplicateprocfs"


//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Hello World using /proc filesystem device driver, which could give you some system information with multiple procfs files that organised inside a proc folder");
MODULE_VERSION("1.0.2");

//Define a command line argument or module parameter to determine the state of output
static char *human = "human";
module_param(human, charp, 0000);
MODULE_PARM_DESC(human, "choose format of the uptime output by one of these two values: [human] or seconds");

//Creating a proc directory entry structure
static struct proc_dir_entry* our_first_file;
static struct proc_dir_entry* our_second_file;
static struct proc_dir_entry* our_proc_folder;


//These functions will call on demand of read request from seq_files
static int first_proc_show(struct seq_file *m, void *v){
	printk(KERN_INFO "DUPLICATEPROCFS: Generating output for user space with seq_files.\n");
	//We are going to count processes which are currently running on the context of the Kernel
	static struct timespec calc_uptime;
	static struct timespec calc_idle;
	int i;
	cputime_t calc_idletime = 0;

	ktime_get_ts(&calc_uptime);

	for_each_possible_cpu(i)
		calc_idletime += kcpustat_cpu(i).cpustat[CPUTIME_IDLE];
	cputime_to_timespec(calc_idletime, &calc_idle);
	
	if(strcmp(human, "seconds") == 0)
		//Just printout with seqfiles the original value of uptime and idle time
		seq_printf(m, "%lu.%02lu %lu.%02lu\n",
			(unsigned long) calc_uptime.tv_sec,
			(calc_uptime.tv_nsec / (NSEC_PER_SEC / 100)),
			(unsigned long) calc_idle.tv_sec,
			(calc_idle.tv_nsec / (NSEC_PER_SEC / 100)));
	else if(strcmp(human, "human") == 0){
		//First, Calculating proximately passed time sience the system boot with jiffies
		unsigned long second, minute, hour, day;

		//Turning uptime to human-readable form
		second = (unsigned long) calc_uptime.tv_sec;

		//Time measuring units for human being
		day = second / (60 * 60 * 24);
		second = second % (60 * 60 * 24);
		hour = second / (60 * 60);
		second = second % (60 * 60);
		minute = second / 60;
		second = second % 60;

		//Creating output with using seq_printf to simply put everything in the userspace
		seq_printf(m, "%ld Days, %ld Hours, %ld Minutes, %ld Seconds\t -- \t",  day, hour, minute, second);
		

		//Turning idle time to human-readable form
		second = (unsigned long) calc_idle.tv_sec;

		//Time measuring units for human being
		day = second / (60 * 60 * 24);
		second = second % (60 * 60 * 24);
		hour = second / (60 * 60);
		second = second % (60 * 60);
		minute = second / 60;
		second = second % 60;

		//Creating output with using seq_printf to simply put everything in the userspace
		seq_printf(m, "%ld Days, %ld Hours, %ld Minutes, %ld Seconds\n",  day, hour, minute, second);		
		}
	else
		seq_printf(m, "Invalid Command Line Argument\n");
	return SUCCESS;
}

static int second_proc_show(struct seq_file *m, void *v){
	seq_printf(m, "%s %s %s %s %s %s\n", utsname()->sysname, utsname()->nodename, utsname()->domainname, utsname()->release, utsname()->version, utsname()->machine);
	return SUCCESS;
}

//This is where system functionallity triggers every time some process try to read from our proc entries
static int first_proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "DUPLICATEPROCFS: First File, Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, first_proc_show, NULL);
}

static int second_proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "DUPLICATEPROCFS: Second File, Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, second_proc_show, NULL);
}

//Now we have to figure out what to do if the user want to write inside each proc file
//Each time user try to echo something or otherwise write anything to the /dev entry, this function does the job
static ssize_t procfile_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	printk(KERN_INFO "DUPLICATEPROCFS: Write Function, Process \"%s:%i\"\n", current->comm, current->pid);

	//Since we use the same write function to answer write request in the userspace on both proc files
	//we have to distinguish which file have been used, so we ought to obtain the file name first
	char *tmp;
	char *pathname;

	tmp = (char *) __get_free_page(GFP_KERNEL);
	if(!tmp){
			printk(KERN_ERR "DUPLICATEPROCFS: Sorry, we couldent allocate a temporary memory to obtain filename");
	    return -ENOMEM;
	}

	//d_path function use to translate pointer address of the f_path and return a string value
	pathname = d_path(&file->f_path, tmp, PAGE_SIZE);
	if(IS_ERR(pathname)){
			printk(KERN_ERR "DUPLICATEPROCFS: Sorry, some error has occured during filename translation: %ld", PTR_ERR(pathname));
	    free_page((unsigned long)tmp);
	    return PTR_ERR(pathname);
	}

	//And finally we can create a properiate Kernel log depend on the filename
	printk(KERN_ALERT "DUPLICATEPROCFS: Sorry, WRITE operation (on %s)is not permitted!\n", pathname);

	//Now we will free the temporary file, which has no use anymore
	free_page((unsigned long)tmp);
	
	//The function returns wrote charachters count, here this means nothing
	return -EINVAL;
}



//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the corresponding /proc entry
static const struct file_operations fops = {
	.open = first_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = procfile_write,
};

static const struct file_operations spof = {
	.open = second_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = procfile_write,
};


//Your module's entry point
static int __init duplicate_procfs_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "DUPLICATEPROCFS: Initialization.\n");
	printk(KERN_INFO "DUPLICATEPROCFS: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//First we are going to create our proc folder
	our_proc_folder = proc_mkdir(DEVICE_NAME, NULL);
	if(!our_proc_folder){
		printk(KERN_ALERT "DUPLICATEPROCFS: Folder Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}
	printk(KERN_INFO "DUPLICATEPROCFS: /proc/%s/ has been created.\n", DEVICE_NAME);

	//After that we use that folder to create our proc files
	our_first_file = proc_create("uptime", 0644 , our_proc_folder, &fops);
	//Put an error message in kernel log if cannot create proc entry
	if(!our_first_file){
		printk(KERN_ALERT "DUPLICATEPROCFS: First File Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}
	printk(KERN_INFO "DUPLICATEPROCFS: /proc/%s/uptime has been created.\n", DEVICE_NAME);


	our_second_file = proc_create("system", 0644 , our_proc_folder, &spof);
	//Put an error message in kernel log if cannot create proc entry
	if(!our_second_file){
		printk(KERN_ALERT "DUPLICATEPROCFS: Second File Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}
	printk(KERN_INFO "DUPLICATEPROCFS: /proc/%s/system has been created.\n", DEVICE_NAME);


	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit duplicate_procfs_exit(void){
	//Initiating module unloading procedure
	printk(KERN_INFO "DUPLICATEPROCFS: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Remove proc filesystem entry from system
	//First we shoud delete our file and empty the folder
	remove_proc_entry("uptime", our_proc_folder);
	printk(KERN_INFO "DUPLICATEPROCFS: /proc/%s/uptime has been removed.\n", DEVICE_NAME);
	remove_proc_entry("system", our_proc_folder);
	printk(KERN_INFO "DUPLICATEPROCFS: /proc/%s/system has been removed.\n", DEVICE_NAME);

	//At last we can remove the emptied folder from the system
	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "DUPLICATEPROCFS: /proc/%s/ has been removed.\n", DEVICE_NAME);

	printk(KERN_INFO "DUPLICATEPROCFS: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}

//Now we need to define init-module and cleanup_module aliases
module_init(duplicate_procfs_init);
module_exit(duplicate_procfs_exit);
