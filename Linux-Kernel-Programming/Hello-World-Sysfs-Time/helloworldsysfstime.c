//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//This is for working with sysfs entries and Linux device model
#include <linux/sysfs.h>
//For creating/deleting of a kobject and relevent manipulation functions
#include <linux/kobject.h>
//For finding the parent process ID of the module
#include <asm/current.h>
//For using task_struct
#include <linux/sched.h>
//For using diffrent time related functions
#include <linux/time.h>
//For obtaining CPU status
#include <linux/kernel_stat.h>
//For counting jiffies and HZ
#include <linux/jiffies.h>
//For using cputime_to_timespec function
#include <linux/sched/cputime.h>
//For ktime_get_ts function to obtain uptime
#include <linux/timekeeping.h>

//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define MY_MODULE_NAME "systemtimes"

//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Hello World using Linux device model in sysfs to reveal some information of your cup times");
MODULE_VERSION("1.0.2");

//This is our kobject which could actually create some entry points in the sysfs
static struct kobject *our_kobj;


static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	printk(KERN_INFO "HELLOWORLDSYSFS: Show Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);

	//These are our primary variables to obtain CPU times in jiffies
	u64 user = 0, nice = 0, system = 0, idle = 0, iowait =0;
	u64 irq = 0 , softirq = 0, steal = 0, guest = 0, guest_nice = 0;
	//It is just a counter
	int i;
	//These are our output variables to convert times from primary variables to human readable seconds
	struct timespec calc_user, calc_nice, calc_system, calc_idle, calc_iowait;
	struct timespec calc_irq, calc_softirq, calc_steal, calc_guest, calc_guest_nice;

	struct timespec calc_uptime;

	ktime_get_ts(&calc_uptime);
	
	//Now we are going to calculate times for each CPUs
	for_each_possible_cpu(i){
		user += kcpustat_cpu(i).cpustat[CPUTIME_USER];
	 	nice += kcpustat_cpu(i).cpustat[CPUTIME_NICE];
	 	system += kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM];
	 	idle += kcpustat_cpu(i).cpustat[CPUTIME_IDLE];
	 	iowait += kcpustat_cpu(i).cpustat[CPUTIME_IOWAIT];
	 	irq += kcpustat_cpu(i).cpustat[CPUTIME_IRQ];
	 	softirq += kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ];
	 	steal += kcpustat_cpu(i).cpustat[CPUTIME_STEAL];
	 	guest += kcpustat_cpu(i).cpustat[CPUTIME_GUEST];
	 	guest_nice += kcpustat_cpu(i).cpustat[CPUTIME_GUEST_NICE];
		}

	//Then converting the results to timespec structs
	calc_user = ktime_to_timespec(user);
	calc_nice = ktime_to_timespec(nice);
	calc_system = ktime_to_timespec(system);
	calc_idle = ktime_to_timespec(idle);
	calc_iowait = ktime_to_timespec(iowait);
	calc_irq = ktime_to_timespec(irq);
	calc_softirq = ktime_to_timespec(softirq);
	calc_steal = ktime_to_timespec(steal);
	calc_guest = ktime_to_timespec(guest);
	calc_guest_nice = ktime_to_timespec(guest_nice);


	//We have to decide on which attribute show wich content to the userspace
	if(strcmp(attr->attr.name, "uptime") == 0)
		return sprintf(buf, "%lu.%02lu\n", calc_uptime.tv_sec, (calc_uptime.tv_nsec / (NSEC_PER_SEC / 100)));
	else if(strcmp(attr->attr.name, "user") == 0)
		return sprintf(buf, "%lu.%02lu\n", calc_user.tv_sec, (calc_user.tv_nsec / (NSEC_PER_SEC / 100)));
	else if(strcmp(attr->attr.name, "ince") == 0)
		return sprintf(buf, "%lu.%02lu\n", calc_nice.tv_sec, (calc_nice.tv_nsec / (NSEC_PER_SEC / 100)));
	else if(strcmp(attr->attr.name, "system") == 0)
		return sprintf(buf, "%lu.%02lu\n", calc_system.tv_sec, (calc_system.tv_nsec / (NSEC_PER_SEC / 100)));
	else if(strcmp(attr->attr.name, "idle") == 0)
		return sprintf(buf, "%lu.%02lu\n", calc_idle.tv_sec, (calc_idle.tv_nsec / (NSEC_PER_SEC / 100)));
	else if(strcmp(attr->attr.name, "iowait") == 0)
		return sprintf(buf, "%lu.%02lu\n", calc_iowait.tv_sec, (calc_iowait.tv_nsec / (NSEC_PER_SEC / 100)));
	else if(strcmp(attr->attr.name, "irq") == 0)
		return sprintf(buf, "%lu.%02lu\n", calc_irq.tv_sec, (calc_irq.tv_nsec / (NSEC_PER_SEC / 100)));
	else if(strcmp(attr->attr.name, "softirq") == 0)
		return sprintf(buf, "%lu.%02lu\n", calc_softirq.tv_sec, (calc_softirq.tv_nsec / (NSEC_PER_SEC / 100)));
	else if(strcmp(attr->attr.name, "steal") == 0)
		return sprintf(buf, "%lu.%02lu\n", calc_steal.tv_sec, (calc_steal.tv_nsec / (NSEC_PER_SEC / 100)));
	else if(strcmp(attr->attr.name, "guest") == 0)
		return sprintf(buf, "%lu.%02lu\n", calc_guest.tv_sec, (calc_guest.tv_nsec / (NSEC_PER_SEC / 100)));
	else if(strcmp(attr->attr.name, "guestnice") == 0)
		return sprintf(buf, "%lu.%02lu\n", calc_guest_nice.tv_sec, (calc_guest_nice.tv_nsec / (NSEC_PER_SEC / 100)));
	else
		printk(KERN_INFO  "HELLOWORLDSYSFS: I don't know what you are doing, but it seems you are not doing it right!\n");
	return NULL;
}


static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
	printk(KERN_INFO "HELLOWORLDSYSFS: Store Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);
	printk(KERN_ALERT "HELLOWORLDSYSFS: You have no Permission to write in %s attribute.\n", attr->attr.name);
	return count;
}


//We need to initialize each and every attribute by definig it's name, permissions, and read and write related functions
static struct kobj_attribute uptime_attribute = __ATTR(uptime, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute user_attribute = __ATTR(user, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute nice_attribute = __ATTR(nice, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute system_attribute = __ATTR(system, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute idle_attribute = __ATTR(idle, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute iowait_attribute = __ATTR(iowait, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute irq_attribute = __ATTR(irq, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute softirq_attribute = __ATTR(softirq, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute steal_attribute = __ATTR(steal, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute guest_attribute = __ATTR(guest, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute guest_nice_attribute = __ATTR(guestnice, 0664, sysfs_show, sysfs_store);

//This is a array of attibutes to shape our device structure in sysfs
static struct attribute *attrs[] = {
	&uptime_attribute.attr,
	&user_attribute.attr,
	&nice_attribute.attr,
	&system_attribute.attr,
	&idle_attribute.attr,
	&iowait_attribute.attr,
	&irq_attribute.attr,
	&softirq_attribute.attr,
	&steal_attribute.attr,
	&guest_attribute.attr,
	&guest_nice_attribute.attr,
	NULL,	//To terminate the list of attributes
};

//An unnamed attribute group will put all of the attributes directly in the kobject directory
static struct attribute_group attr_group = {
	.attrs = attrs,
};


//Every Module must have an entry point which is known as init_module or
//some function that defines with module_init macros. init_module is the function
//that would call immidately after insmoding your module to set everything right
//and registering module functionality to the Kernel
static int __init hello_world_init(void){
	int retval;

	//First, we print out some information about the process in the uer space wich has called our process
	printk(KERN_INFO "HELLOWORLDSYSFS: Initialization\n");
	printk(KERN_INFO "HELLOWORLDSYSFS: Init Module, \"%s:%i\"\n", current->comm, current->pid);

	//Now we have to register our kobject directory in the sysfs under kernel class
	our_kobj = kobject_create_and_add(MY_MODULE_NAME, kernel_kobj);
	if (!our_kobj){
		printk(KERN_ALERT "HELLOWORLDSYSFS: KOBJECT Registration Failure.\n");
		//Because of this fact that sysfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}

	//Then create files associated with this kobject
	retval = sysfs_create_group(our_kobj, &attr_group);
	if (retval){
		printk(KERN_ALERT "HELLOWORLDSYSFS: Creating attribute groupe has been failed.\n");
		//Because kobject creation actually increased the reference cont of it, we kno in case of failure need to reduce it by one
		kobject_put(our_kobj);
	}

	//Now we can promt users how to use it
	//Creating output with combination of these three step in a single line
	//We are using seq_printf to simply put everything in the userspace
	printk(KERN_INFO "HELLOWORLDSYSFS: You can obtain some information by reading files under /sys/kernel/%s\n", MY_MODULE_NAME);
	printk(KERN_INFO "HELLOWORLDSYSFS: Module %s is ready to go ;)\n", MY_MODULE_NAME);

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//This is our clean up function
static void __exit hello_world_exit(void){
	//Here we have to undone whatever we have initiated in init module in the first place
	printk(KERN_INFO "HELLOWORLDSYSFS: Cleanup Module\"%s:%i\"\n", current->comm, current->pid);

	//We have to release our kobject and delete all files in it, it will do with this single line of code
	kobject_put(our_kobj);
	printk(KERN_INFO "HELLOWORLDSYSFS: /sys/kernel/%s and all its attributes has been removed.\n", MY_MODULE_NAME);

	printk(KERN_INFO "HELLOWORLDSYSFS: Good Bye World ;)\n", MY_MODULE_NAME);
	//Our module has removed successfully from the system
}


//Now we need to define init-module and cleanup_module aliases
module_init(hello_world_init);
module_exit(hello_world_exit);
