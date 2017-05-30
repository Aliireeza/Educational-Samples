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

//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define MY_MODULE_NAME "systeminformation"

//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Hello World using Linux device model in sysfs to reveal some information of your system");
MODULE_VERSION("1.0.2");

//This is our kobject which could actually create some entry points in the sysfs
static struct kobject *our_kobj;


static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	printk(KERN_INFO "HELLOWORLDSYSFS: Show Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);

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
		printk(KERN_INFO  "HELLOWORLDSYSFS: I don't know what you are doing, but it seems you are not doing it right!\n");
	return NULL;
}


static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
	printk(KERN_INFO "HELLOWORLDSYSFS: Store Function, %s Attribute,  Process \"%s:%i\"\n",  attr->attr.name, current->comm, current->pid);
	printk(KERN_ALERT "HELLOWORLDSYSFS: You have no Permission to write in %s attribute.\n", attr->attr.name);
	return count;
}


//We need to initialize each and every attribute by definig it's name, permissions, and read and write related functions
static struct kobj_attribute sysname_attribute = __ATTR(sysname, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute nodename_attribute = __ATTR(nodename, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute release_attribute = __ATTR(release, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute version_attribute = __ATTR(version, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute machine_attribute = __ATTR(machine, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute domainname_attribute = __ATTR(domainname, 0664, sysfs_show, sysfs_store);

//This is a array of attibutes to shape our device structure in sysfs
static struct attribute *attrs[] = {
	&sysname_attribute.attr,
	&nodename_attribute.attr,
	&release_attribute.attr,
	&version_attribute.attr,
	&machine_attribute.attr,
	&domainname_attribute.attr,
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
	printk(KERN_INFO "HELLOWORLDSYSFS: You can obtain the following information by reading files under /sys/kernel/%s\n", MY_MODULE_NAME);
	printk(KERN_INFO "HELLOWORLDSYSFS: System Name: \"%s\"\n", utsname()->sysname);
	printk(KERN_INFO "HELLOWORLDSYSFS: Node Name: \"%s\"\n", utsname()->nodename);
	printk(KERN_INFO "HELLOWORLDSYSFS: Release: \"%s\"\n", utsname()->release);
	printk(KERN_INFO "HELLOWORLDSYSFS: Version: \"%s\"\n", utsname()->version);
	printk(KERN_INFO "HELLOWORLDSYSFS: Machine: \"%s\"\n", utsname()->machine);
	printk(KERN_INFO "HELLOWORLDSYSFS: Domain Name: \"%s\"\n", utsname()->domainname);

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
