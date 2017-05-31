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
//For getting system information
#include <linux/utsname.h>
//For finding the parent process ID of the module
#include <asm/current.h>
//For using task_struct
#include <linux/sched.h>
//For "do_gettimeofday" or "current_kernel_time" or "getnstimeofday" functions
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
#define MY_MODULE_NAME "symbolicsysfs"


//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple module using Linux device model in sysfs to reveal some information of your system and use symbolic links in device models");
MODULE_VERSION("1.0.2");


//These command line arguments are defined for calculation of local time from GMT
//Default value is +3:30 which is used to obtain TEHRAN's local time
static int gmt_hour = 3;
module_param(gmt_hour, int, 0);
MODULE_PARM_DESC(gmt_hour, "This command line argument will differ local time hours from GMT time, default is +3 hours for Tehran, Iran");


static int gmt_minute = 30;
module_param(gmt_minute, int, 0);
MODULE_PARM_DESC(gmt_minute, "This command line argument will differ local time minutes from GMT time, default is +30 minutes for Tehran, Iran");


//This will use to create a nice output
static char gmt_sign = '+';



//This is our kobject which could actually create some entry points in the sysfs
static struct kobject *system_kobj;
static struct kobject *clock_kobj;
static struct kobject *times_kobj;



static ssize_t clock_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	printk(KERN_INFO "SYMBOLICSYSFS: Clock Show Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);

	//We have to decide on which attribute show wich content to the userspace
	if(strcmp(attr->attr.name, "gmt") == 0){
		//This is how we are going to get these times
		//First, we use current_kernel_time to obtain current time of system
		//which will be used to calculate times for GMT value
		struct timespec my_timeofday_gmt = current_kernel_time();

		//These variables will use to create a more human readable output
		int hour_gmt, minute_gmt, second_gmt;

		//Now we only use the second fragment of timespec structs
		second_gmt = (int) my_timeofday_gmt.tv_sec;
		hour_gmt = (second_gmt / 3600) % 24;
		minute_gmt = (second_gmt / 60) % 60;
		second_gmt %= 60;


		return sprintf(buf, "%d:%d:%d\n", hour_gmt, minute_gmt, second_gmt);
		}
	else if (strcmp(attr->attr.name, "local") == 0){
		//This is how we are going to get these times
		//First, we use current_kernel_time to obtain current time of system
		//which will be used to calculate times for GMT value
		struct timespec my_timeofday_gmt = current_kernel_time();
		//Then for absolutely no reason at all we will use a different way
		//to obtain exactly the same value from getnstimeofday function
		//which will be used in calculation of local time
		struct timespec my_timeofday_loc;
			getnstimeofday(&my_timeofday_loc);

		//These variables will use to create a more human readable output
		int hour_loc, minute_loc, second_loc;

		//Now we only use the second fragment of timespec structs
		second_loc = (int) my_timeofday_loc.tv_sec;
		hour_loc = ((second_loc / 3600) % 24 + gmt_hour) % 24;
		minute_loc = (second_loc / 60) % 60 + gmt_minute;

		//And this is just a simple trick to adding local difference to the real value
		if(minute_loc>=60){
			hour_loc = (hour_loc + 1) % 24;
			minute_loc %= 60;
			}
		second_loc %= 60;

		return sprintf(buf, "%d:%d:%d\n", hour_loc, minute_loc, second_loc);
		}
	else if (strcmp(attr->attr.name, "second") == 0)
		return sprintf(buf, "%lu\n", jiffies/HZ);
	else if (strcmp(attr->attr.name, "timediff") == 0)
			return sprintf(buf, "%c%d:%d\n", gmt_sign, gmt_hour, gmt_minute);
	else
		printk(KERN_INFO  "SYMBOLICSYSFS: I don't know what you are doing, but it seems you are not doing it right!\n");
	return NULL;
}


static ssize_t clock_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
	printk(KERN_INFO "SYMBOLICSYSFS: Clock Store Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);

	if(strcmp(attr->attr.name, "gmt") == 0)
		printk(KERN_ALERT "SYMBOLICSYSFS: You have no Permission to write in GMT Time attribute.\n");
	else if (strcmp(attr->attr.name, "local") == 0)
		printk(KERN_ALERT "SYMBOLICSYSFS: You have no Permission to write in LOCAL Time attribute.\n");
	else if (strcmp(attr->attr.name, "second") == 0)
		printk(KERN_ALERT "SYMBOLICSYSFS: You have no Permission to write in Jiffies attribute.\n");
	else if (strcmp(attr->attr.name, "timediff") == 0)
			printk(KERN_ALERT "SYMBOLICSYSFS: You have no Permission to write in TimeDiff attribute.\n");
	else
		printk(KERN_ALERT "SYMBOLICSYSFS: I don't know what you are doing, but it seems you are not permitted!\n");
	return count;
}


static ssize_t system_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	printk(KERN_INFO "SYMBOLICSYSFS: System Show Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);

	//We have to decide on which attribute show wich content to the userspace
	if(strcmp(attr->attr.name, "sysname") == 0)
		return sprintf(buf, "%s\n", utsname()->sysname);
	else if (strcmp(attr->attr.name, "nodename") == 0)
		return sprintf(buf, "%s\n", utsname()->nodename);
	else if (strcmp(attr->attr.name, "release") == 0)
		return sprintf(buf, "%s\n", utsname()->release);
	else if (strcmp(attr->attr.name, "version") == 0)
		return sprintf(buf, "%s\n", utsname()->version);
	else if (strcmp(attr->attr.name, "machine") == 0)
		return sprintf(buf, "%s\n", utsname()->machine);
	else if (strcmp(attr->attr.name, "domainname") == 0)
		return sprintf(buf, "%s\n", utsname()->domainname);
	else
		printk(KERN_INFO  "SYMBOLICSYSFS: I don't know what you are doing, but it seems you are not doing it right!\n");
	return NULL;
}


static ssize_t system_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
	printk(KERN_INFO "SYMBOLICSYSFS: System Store Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);
	printk(KERN_ALERT "SYMBOLICSYSFS: You have no Permission to write in %s attribute.\n", attr->attr.name);
	return count;
}


static ssize_t times_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	printk(KERN_INFO "SYMBOLICSYSFS: Times Show Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);

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
	cputime_to_timespec(user, &calc_user);
	cputime_to_timespec(nice, &calc_nice);
	cputime_to_timespec(system, &calc_system);
	cputime_to_timespec(idle, &calc_idle);
	cputime_to_timespec(iowait, &calc_iowait);
	cputime_to_timespec(irq, &calc_irq);
	cputime_to_timespec(softirq, &calc_softirq);
	cputime_to_timespec(steal, &calc_steal);
	cputime_to_timespec(guest, &calc_guest);
	cputime_to_timespec(guest_nice, &calc_guest_nice);


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
		printk(KERN_INFO  "SYMBOLICSYSFS: I don't know what you are doing, but it seems you are not doing it right!\n");
	return NULL;
}


static ssize_t times_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
	printk(KERN_INFO "SYMBOLICSYSFS: Times Store Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);
	printk(KERN_ALERT "SYMBOLICSYSFS: You have no Permission to write in %s attribute.\n", attr->attr.name);
	return count;
}


//We need to initialize each and every attribute by definig it's name, permissions, and read and write related functions
static struct kobj_attribute sysname_attribute = __ATTR(sysname, 0664, system_show, system_store);
static struct kobj_attribute nodename_attribute = __ATTR(nodename, 0664, system_show, system_store);
static struct kobj_attribute release_attribute = __ATTR(release, 0664, system_show, system_store);
static struct kobj_attribute version_attribute = __ATTR(version, 0664, system_show, system_store);
static struct kobj_attribute machine_attribute = __ATTR(machine, 0664, system_show, system_store);
static struct kobj_attribute domainname_attribute = __ATTR(domainname, 0664, system_show, system_store);


static struct kobj_attribute gmttime_attribute = __ATTR(gmt, 0664, clock_show, clock_store);
static struct kobj_attribute localtime_attribute = __ATTR(local, 0664, clock_show, clock_store);
static struct kobj_attribute jiffies_attribute = __ATTR(second, 0664, clock_show, clock_store);
static struct kobj_attribute timediff_attribute = __ATTR(timediff, 0664, clock_show, clock_store);


static struct kobj_attribute uptime_attribute = __ATTR(uptime, 0664, times_show, times_store);
static struct kobj_attribute user_attribute = __ATTR(user, 0664, times_show, times_store);
static struct kobj_attribute nice_attribute = __ATTR(nice, 0664, times_show, times_store);
static struct kobj_attribute system_attribute = __ATTR(system, 0664, times_show, times_store);
static struct kobj_attribute idle_attribute = __ATTR(idle, 0664, times_show, times_store);
static struct kobj_attribute iowait_attribute = __ATTR(iowait, 0664, times_show, times_store);
static struct kobj_attribute irq_attribute = __ATTR(irq, 0664, times_show, times_store);
static struct kobj_attribute softirq_attribute = __ATTR(softirq, 0664, times_show, times_store);
static struct kobj_attribute steal_attribute = __ATTR(steal, 0664, times_show, times_store);
static struct kobj_attribute guest_attribute = __ATTR(guest, 0664, times_show, times_store);
static struct kobj_attribute guest_nice_attribute = __ATTR(guestnice, 0664, times_show, times_store);

//This is a array of attibutes to shape our device structure in sysfs
static struct attribute *times_attrs[] = {
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


static struct attribute *system_attrs[] = {
	&sysname_attribute.attr,
	&nodename_attribute.attr,
	&release_attribute.attr,
	&version_attribute.attr,
	&machine_attribute.attr,
	&domainname_attribute.attr,
	NULL,	//To terminate the list of attributes
};


static struct attribute *clock_attrs[] = {
	&gmttime_attribute.attr,
	&localtime_attribute.attr,
	&jiffies_attribute.attr,
	&timediff_attribute.attr,
	NULL,	//To terminate the list of attributes
};

//An unnamed attribute group will put all of the attributes directly in the kobject directory
static struct attribute_group times_attr_group = {
	.attrs = times_attrs,
};

static struct attribute_group system_attr_group = {
	.attrs = system_attrs,
};

static struct attribute_group clock_attr_group = {
	.attrs = clock_attrs,
};


//This is our clean up function
static void __exit symbolic_sysfs_exit(void){
	//Here we have to undone whatever we have initiated in init module in the first place
	printk(KERN_INFO "SYMBOLICSYSFS: Cleanup Module\"%s:%i\"\n", current->comm, current->pid);

	//First we have to remove liks, below commands will do this
	sysfs_remove_link(system_kobj, "localtime");
	sysfs_remove_link(system_kobj, "systemtimes");

	sysfs_remove_link(clock_kobj, "systeminfo");
	sysfs_remove_link(clock_kobj, "systemtimes");
	
	sysfs_remove_link(times_kobj, "systeminfo");
	sysfs_remove_link(times_kobj, "localtime");
	
	//We have to release our kobject and delete all files in it, it will do with this single line of code
	if(system_kobj){
		kobject_put(system_kobj);
		printk(KERN_INFO "SYMBOLICSYSFS: /sys/kernel/systeminfo and all it's attributes has been removed.\n");
		}
	if(clock_kobj){
		kobject_put(clock_kobj);
		printk(KERN_INFO "SYMBOLICSYSFS: /sys/kernel/localtime and all it's attributes has been removed.\n");
		}
	if(times_kobj){
		kobject_put(times_kobj);
		printk(KERN_INFO "SYMBOLICSYSFS: /sys/kernel/systemtimes and all it's attributes has been removed.\n");
		}
	

	printk(KERN_INFO "SYMBOLICSYSFS: Good Bye World ;)\n", MY_MODULE_NAME);
	//Our module has removed successfully from the system
}



//Every Module must have an entry point which is known as init_module or
//some function that defines with module_init macros. init_module is the function
//that would call immidately after insmoding your module to set everything right
//and registering module functionality to the Kernel
static int __init symbolic_sysfs_init(void){
	int retval;

	//First, we print out some information about the process in the uer space wich has called our process
	printk(KERN_INFO "SYMBOLICSYSFS: Initialization\n");
	printk(KERN_INFO "SYMBOLICSYSFS: Init Module, \"%s:%i\"\n", current->comm, current->pid);

	//Now we have to register our kobject directory in the sysfs under kernel class
	system_kobj = kobject_create_and_add("systeminfo", kernel_kobj);
	if (!system_kobj){
		printk(KERN_ALERT "SYMBOLICSYSFS: SYSTEM KOBJECT Registration Failure.\n");
		//to clean up everthing we have done yet, the easiest way is to call cleanup_module function
		symbolic_sysfs_exit();
		//Because of this fact that sysfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}
	printk(KERN_INFO "SYMBOLICSYSFS: SYSTEM KOBJECT has been created successfully.\n");
	
	clock_kobj = kobject_create_and_add("localtime", kernel_kobj);
	if (!clock_kobj){
		printk(KERN_ALERT "SYMBOLICSYSFS: CLOCK KOBJECT Registration Failure.\n");
		//to clean up everthing we have done yet, the easiest way is to call cleanup_module function
		symbolic_sysfs_exit();
		//Because of this fact that sysfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}
	printk(KERN_INFO "SYMBOLICSYSFS: CLOCK KOBJECT has been created successfully.\n");
	
	times_kobj = kobject_create_and_add("systemtimes", kernel_kobj);
	if (!times_kobj){
		printk(KERN_ALERT "SYMBOLICSYSFS: TIMES KOBJECT Registration Failure.\n");
		//to clean up everthing we have done yet, the easiest way is to call cleanup_module function
		symbolic_sysfs_exit();
		//Because of this fact that sysfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}
	printk(KERN_INFO "SYMBOLICSYSFS: TIMES KOBJECT has been created successfully.\n");
	
	
	//Then create files associated with this kobject
	retval = sysfs_create_group(system_kobj, &system_attr_group);
	if (retval){
		printk(KERN_ALERT "SYMBOLICSYSFS: Creating system attribute groupe has been failed.\n");
		//Because kobject creation actually increased the reference cont of it, we kno in case of failure need to reduce it by one
		kobject_put(system_kobj);
		//to clean up everthing we have done yet, the easiest way is to call cleanup_module function
		symbolic_sysfs_exit();
		return retval;
	}
	printk(KERN_INFO "SYMBOLICSYSFS: System KOBJECT and its attributes are accessible through /sys/kernel/systeminformation/.\n");

	retval = sysfs_create_group(clock_kobj, &clock_attr_group);
	if (retval){
		printk(KERN_ALERT "SYMBOLICSYSFS: Creating clock attribute groupe has been failed.\n");
		//Because kobject creation actually increased the reference cont of it, we kno in case of failure need to reduce it by one
		kobject_put(clock_kobj);
		//to clean up everthing we have done yet, the easiest way is to call cleanup_module function
		symbolic_sysfs_exit();
		return retval;
	}
	printk(KERN_INFO "SYMBOLICSYSFS: Clock KOBJECT and its attributes are accessible through /sys/kernel/localtime/.\n");


	retval = sysfs_create_group(times_kobj, &times_attr_group);
	if (retval){
		printk(KERN_ALERT "SYMBOLICSYSFS: Creating times attribute groupe has been failed.\n");
		//Because kobject creation actually increased the reference cont of it, we kno in case of failure need to reduce it by one
		kobject_put(times_kobj);
		//to clean up everthing we have done yet, the easiest way is to call cleanup_module function
		symbolic_sysfs_exit();
		return retval;
	}
	printk(KERN_INFO "SYMBOLICSYSFS: Clock KOBJECT and its attributes are accessible through /sys/kernel/systemtimes/.\n");
	
	
	if(sysfs_create_link(system_kobj, clock_kobj, "localtime"))
		printk(KERN_ALERT "SYMBOLICSYSFS: Symbolic link creation error Clock--->System\n");
	printk(KERN_INFO "SYMBOLICSYSFS: Symbolic Link Created Clock--->System");

	if(sysfs_create_link(system_kobj, times_kobj, "systemtimes"))
		printk(KERN_ALERT "SYMBOLICSYSFS: Symbolic link creation error Times--->System\n");
	printk(KERN_INFO "SYMBOLICSYSFS: Symbolic Link Created Times--->System");


	if(sysfs_create_link(clock_kobj, system_kobj, "systeminfo"))
		printk(KERN_ALERT "SYMBOLICSYSFS: Symbolic link creation error System--->Clock\n");
	printk(KERN_INFO "SYMBOLICSYSFS: Symbolic Link Created System--->Clock");
	
	if(sysfs_create_link(clock_kobj, times_kobj, "systemtimes"))
		printk(KERN_ALERT "SYMBOLICSYSFS: Symbolic link creation error Times--->Clock\n");
	printk(KERN_INFO "SYMBOLICSYSFS: Symbolic Link Created Times--->Clock");


	if(sysfs_create_link(times_kobj, system_kobj, "systeminfo"))
		printk(KERN_ALERT "SYMBOLICSYSFS: Symbolic link creation error System--->Times\n");
	printk(KERN_INFO "SYMBOLICSYSFS: Symbolic Link Created System--->Times");

	if(sysfs_create_link(times_kobj, clock_kobj, "localtime"))
		printk(KERN_ALERT "SYMBOLICSYSFS: Symbolic link creation error Clock--->Times\n");
	printk(KERN_INFO "SYMBOLICSYSFS: Symbolic Link Created Clock--->Times");



	//It is a simple calculation of our output sign
	if(gmt_hour > 0)
		gmt_sign = '+';
	else if(gmt_hour < 0)
		gmt_sign = '-';
	else
		if(gmt_minute >= 0)
			gmt_sign = '+';
		else
			gmt_sign = '-';


	//Now we can promt users to use it
	printk(KERN_INFO "SYMBOLICSYSFS: Module %s is ready to go ;)\n", MY_MODULE_NAME);

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}




//Now we need to define init-module and cleanup_module aliases
module_init(symbolic_sysfs_init);
module_exit(symbolic_sysfs_exit);
