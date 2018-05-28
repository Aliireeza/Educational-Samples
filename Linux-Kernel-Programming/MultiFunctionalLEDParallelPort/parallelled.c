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
//For ioctl commands and macros
#include <linux/ioctl.h>
//For raw_copy_to_user, raw_copy_from_user, put_user
#include <asm/uaccess.h>
//For using I/O ports
#include <linux/ioport.h>
//For using interrupt functions and Tasklets
#include <linux/interrupt.h>
//Most of delay functions lie here
#include <linux/delay.h>
//For Capability Macros
#include <linux/capability.h>
//For standard string functions of c language
#include <linux/string.h>
//For counting jiffies and HZ
#include <linux/jiffies.h>
//This is for working with sysfs entries and Linux device model
#include <linux/sysfs.h>
//For creating/deleting of a kobject and relevent manipulation functions
#include <linux/kobject.h>
//For using Kernel timers
#include <linux/timer.h>
//For I/O read and write functions such as inb and outb_p
#include <asm/io.h>


//This header is our ioctl commands definition
#include "commonioctlcommands.h"


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DRIVER_NAME "parallelled"


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This module uses parallel port to create an array of blinking LEDs");
MODULE_VERSION("1.0.4");


//*****************************************************************
//Some of variable that are necessary for the driver, some of them are defined
//as module parameters, which could be set at the insmod time, their names
//are quite self documentary
static int parallel_mode = 0;
module_param(parallel_mode, int, 0);
MODULE_PARM_DESC(parallel_mode, "Single Blink:0 [Default], All Blink:1, Counter:2");

static int parallel_delay = 1000;
module_param(parallel_delay, int, 0);
MODULE_PARM_DESC(parallel_delay, "Delay between toggling LEDs, Default:1000ms, Min:100ms, Max: 10000ms");

static int parallel_step = 500;
module_param(parallel_step, int, 0);
MODULE_PARM_DESC(parallel_step, "Change blinking speed in each round, Default:500ms, Min:100ms, Max:1000ms");

static int parallel_value = 1;

static int parallel_base = 0x378;
module_param(parallel_base, int, 0);
MODULE_PARM_DESC(parallel_base, "Parallel port base address, Default:0x378");

static int parallel_ports = 3;

static int probe_mode = 0;
module_param(probe_mode, int, 0);
MODULE_PARM_DESC(probe_mode, "Kernel Probing:0 [Default], Hand Probing:1");

static int parallel_irq = -1;
static int timer_expire_delay;


//This is the pointer to procfs entry associated data structure
static struct proc_dir_entry *proc_file;
//This is our kobject which could actually create some entry points in the sysfs
static struct kobject *parallel_kobj;
//This is the data structure that we use for kernel timers
static struct timer_list parallel_timer;


//*****************************************************************
//This is the actual interrupt handler, change the delay value
irqreturn_t parallel_interrupt_handler(int irq, void *dev_id){
	if(timer_expire_delay <= 10000)
		timer_expire_delay += parallel_step;
	else
		timer_expire_delay = parallel_delay;
	return IRQ_HANDLED;
}


//This is the timer function which produce the desire output for
//the parallel port depend on the parallel_mode value
void parallel_timer_funtion(struct timer_list *timer){
	parallel_value = inb(parallel_base);
	rmb();
	
	switch(parallel_mode){
		case 0:
			if(parallel_value >= 128 || parallel_value == 0)
				parallel_value = 1;
			else
				parallel_value *= 2;
			break;
		case 1:
			if(parallel_value >= 127)
				parallel_value = 0;
			else
				parallel_value = 127;
			break;
		default:
			if(parallel_value <= 127)
				parallel_value ++;
			else
				parallel_value = 0;
		}
		
	outb(parallel_value, parallel_base);
	wmb();
	if(printk_ratelimit())
			printk(KERN_INFO "PARALLELVALUE: %d\n", parallel_value);
	mod_timer(&parallel_timer, jiffies + timer_expire_delay);
}


//*****************************************************************
//The sysfs_show method is responsible to generate a relevant output
//depend on which of the sysfs attributes that user wants to read from
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer){
	if(strcmp(attr->attr.name, "mode") == 0)
		return sprintf(buffer, "%d\n", parallel_mode);
	else if(strcmp(attr->attr.name, "delay") == 0)
		return sprintf(buffer, "%d\n", parallel_delay);
	else if(strcmp(attr->attr.name, "step") == 0)
		return sprintf(buffer, "%d\n", parallel_step);
	else if(strcmp(attr->attr.name, "probe") == 0)
		return sprintf(buffer, "%d\n", probe_mode);
	else if(strcmp(attr->attr.name, "port") == 0)
		return sprintf(buffer, "%d\n", parallel_base);
	else if(strcmp(attr->attr.name, "interrupt") == 0)
		return sprintf(buffer, "%d\n", parallel_irq);
	else
		return sprintf(buffer, "%d\n", parallel_value);
	return strlen(buffer);
}


//The sysfs_show method is responsible to receive an input and set the relevant
//variable depend on which of the sysfs attributes that user wants to write into
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count){
	if(strcmp(attr->attr.name, "mode") == 0)
		sscanf(buffer, "%d\n", &parallel_mode);
	else if(strcmp(attr->attr.name, "delay") == 0)
		sscanf(buffer, "%d\n", &parallel_delay);
	else if(strcmp(attr->attr.name, "step") == 0)
		sscanf(buffer, "%d\n", &parallel_step);
	else{
		printk(KERN_ALERT "PARALLELLED: NO Write permission in /sys/kernel/%s\n", attr->attr.name);
		return -EPERM;
		}
	return count;
}


//We need to initialize each and every attribute by definig it's name, permissions, and read and write related functions
static struct kobj_attribute parallel_mode_attribute = __ATTR(mode, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute parallel_delay_attribute = __ATTR(delay, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute parallel_step_attribute = __ATTR(step, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute parallel_value_attribute = __ATTR(value, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute parallel_probe_attribute = __ATTR(probe, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute parallel_port_attribute = __ATTR(port, 0664, sysfs_show, sysfs_store);
static struct kobj_attribute parallel_irq_attribute = __ATTR(interrupt, 0664, sysfs_show, sysfs_store);


//This is a array of attibutes to shape our device structure in sysfs
static struct attribute *parallel_attrs[] = {
	&parallel_mode_attribute.attr,
	&parallel_delay_attribute.attr,
	&parallel_step_attribute.attr,
	&parallel_value_attribute.attr,
	&parallel_probe_attribute.attr,
	&parallel_port_attribute.attr,
	&parallel_irq_attribute.attr,
	NULL,
};


//An attribute group will put all of the attributes directly in the kobject directory
static struct attribute_group parallel_attr_group = {
	.attrs = parallel_attrs,
};


//*****************************************************************
//When device recive ioctl commands this function will perform the job depending on what kind of command it recieved
long proc_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	static int error = 0;
	static char buffer[30];

	if(_IOC_TYPE(cmd) != MAGIC || _IOC_NR(cmd) > IOC_MAXNR)
		return -ENOTTY;

	if(_IOC_DIR(cmd) & _IOC_READ)
		error = !access_ok(VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
	if(_IOC_DIR(cmd) & _IOC_WRITE)
		error = !access_ok(VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
	if(error)
		return -EFAULT;
	
	switch(cmd){
		case IOCTL_PARALLEL_RESET:
			printk(KERN_INFO "PARALLELLED: Reset IOCTL Command\n");
			if(!capable(CAP_SYS_ADMIN))
				return -EPERM;
			//Do Something to reset the whole device
			break;
		case IOCTL_TIMER_TOGGLE:
			printk(KERN_INFO "PARALLELLED: Toggle Timer IOCTL Command\n");
			//Do Something to toggle the timer
			break;
			
		//Handle IOCTL commands that wants to read some settings	
		case IOCTL_PROBE_READ:
			printk(KERN_INFO "PARALLELLED: Probe Mode Read IOCTL Command\n");
			sprintf(buffer, "%d\n", probe_mode);
			raw_copy_to_user((int __user *) arg, buffer, 30);
			break;
		case IOCTL_DELAY_READ:
			printk(KERN_INFO "PARALLELLED: Timer Interval Read IOCTL Command\n");
			sprintf(buffer, "%d\n", parallel_delay);
			raw_copy_to_user((int __user *) arg, buffer, 30);
			break;
		case IOCTL_STEP_READ:
			printk(KERN_INFO "PARALLELLED: Timer Step Read IOCTL Command\n");
			sprintf(buffer, "%d\n", parallel_step);
			raw_copy_to_user((int __user *) arg, buffer, 30);
			break;
		case IOCTL_MODE_READ:
			printk(KERN_INFO "PARALLELLED: Parallel Mode Read IOCTL Command\n");
			sprintf(buffer, "%d\n", parallel_mode);
			raw_copy_to_user((int __user *) arg, buffer, 30);
			break;
		case IOCTL_VALUE_READ:
			printk(KERN_INFO "PARALLELLED: Port Value Read IOCTL Command\n");
			sprintf(buffer, "%d\n", parallel_value);
			raw_copy_to_user((int __user *) arg, buffer, 30);
			break;
		case IOCTL_IRQ_READ:
			printk(KERN_INFO "PARALLELLED: Parallel IRQ Read IOCTL Command\n");
			sprintf(buffer, "%d\n", parallel_irq);
			raw_copy_to_user((int __user *) arg, buffer, 30);
			break;
		case IOCTL_PORT_READ:
			printk(KERN_INFO "PARALLELLED: Port Address Read IOCTL Command\n");
			sprintf(buffer, "%d\n", parallel_base);
			raw_copy_to_user((int __user *) arg, buffer, 30);
			break;
			
		//Handle IOCTL commands that wants to change some settings	
		case IOCTL_DELAY_WRITE:
			printk(KERN_INFO "PARALLELLED: Timer Delay Write IOCTL Command\n");
			raw_copy_from_user(buffer, (int __user *) arg, 30);
			sscanf(buffer, "%d\n", &parallel_delay);
			break;	
		case IOCTL_STEP_WRITE:
			printk(KERN_INFO "PARALLELLED: Timer Step Write IOCTL Command\n");
			raw_copy_from_user(buffer, (int __user *) arg, 30);
			sscanf(buffer, "%d\n", &parallel_step);
			break;	
		case IOCTL_MODE_WRITE:
			printk(KERN_INFO "PARALLELLED: Parallel Mode Write IOCTL Command\n");
			raw_copy_from_user(buffer, (int __user *) arg, 30);
			sscanf(buffer, "%d\n", &parallel_mode);
			break;	
	default:
		printk(KERN_ALERT "PARALLELLED: Invalid IOCTL Cmd (%u)\n", cmd);
		return -EFAULT;
	}
	return SUCCESS;
}


//This function calls on demand of read request from seq_files
static int proc_show(struct seq_file *m, void *v){
	seq_printf(m, "MODE: %d", parallel_mode);
	if(parallel_mode == 0)
		seq_printf(m, " [Single LED Mode]\n");
	else if (parallel_mode == 1)
		seq_printf(m, " [Whole LED Mode]\n");
	else
		seq_printf(m, " [Counter Mode]\n");
	
	seq_printf(m, "Delay: %d [Step: %d]\n", parallel_delay, parallel_step);
	seq_printf(m, "Ports: %X to %X [Value: %d]", parallel_base , parallel_base + parallel_ports - 1, parallel_value);
	seq_printf(m, "Probing Mode: %s [irq: %d]\n", probe_mode == 0? "Kernel Assissted": "Do-it-Yourself", parallel_irq);
	return SUCCESS;
}


//This is where system functionallity triggers every time some process try to read from our proc entry
static int proc_open(struct inode *inode, struct file *file){
	return single_open(file, proc_show, NULL);
}


//This is where system functionallity triggers every time some process try to write into our proc entry
static ssize_t proc_write(struct file *file, const char *buffer, size_t length, loff_t * off){
	//The thing is logically this driver could not use any input by the user
	//So, simply ban this operation by write a log message
	printk(KERN_ALERT "PARALLELLED: ProcFS Write Permission Denied\n");
	return -EFAULT;
}


//This data structure holds th function pointers for the procfs entry's functionality
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the corresponding /proc entry
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = proc_open,		//This would be responsible for reading the file's content
	.write = proc_write,		//In case that user wants to write anything to the driver
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.unlocked_ioctl = proc_ioctl,	//To handle possible IOCTL commands for the parallel port
};


//*****************************************************************
//This is a simple and temporary interrupt handler to use just for the probe
irqreturn_t probing_interrupt_handler(int irq, void *dev_id){
	//Found the possible relevant IRQ number
	if(parallel_irq == 0) parallel_irq = irq;
	//Ambiguous condition in conflict with the first condition
	if(parallel_irq != irq) parallel_irq = -irq;
	//Nevertheless, the interrupt handler did its job
	return IRQ_HANDLED;
}


//Direct probing by hand, more complicated than Kernel assissted but more felexible
void parallel_direct_probing(void){
	int trials[] = {3, 5, 7, 9, 0};
	int tried[]  = {0, 0, 0, 0, 0};
	int i, count = 0;

	for (i=0; trials[i]; i++)
		tried[i] = request_irq(trials[i], probing_interrupt_handler, IRQF_TRIGGER_NONE, DRIVER_NAME, NULL);

	do{
		parallel_irq = 0;		//Reset irq number
		outb_p(0x10,parallel_base + 2);	//Enable interrupts on parrallel port
		outb_p(0x00,parallel_base);	//Erase data bits
		udelay(5);
		outb_p(0xFF,parallel_base);	//Toggle them, so we can capture an interrupt
		outb_p(0x00,parallel_base + 2);	//Disable interrupts on parrallel port
		//Give it some times
		udelay(5);
		//the value has been set by the handler
		if(parallel_irq == 0){
			printk(KERN_INFO "PARALLELED: No irq reported by probe, in %d try.\n", count);
			parallel_irq = -1;
		}
		//If more than one line has been activated, the result is
		//negative. We should service the interrupt (but the lpt port
		//doesn't need it) and loop over again. Do it at most 5 times
	}while(parallel_irq <=0 && count++ < 5);

	//End of loop, uninstall the handler
	for(i=0; trials[i]; i++)
		if(tried[i] == 0)
			free_irq(trials[i], NULL);

	if (parallel_irq < 0)
		printk(KERN_INFO "PARALLELED: Direct probe failed %i times, giving up!\n", count);
}


//This method of probing is used Kernel functions to monitor the IRQ line
void parallel_kernel_probing(void){
	int count = 0;
	do{
		unsigned long mask;

		mask = probe_irq_on();			//Save the current interrupt state of the system
		outb_p(0x10,parallel_base + 2);		//Enable interrupts on parrallel port
		outb_p(0x00,parallel_base);		//Erase data bits
		udelay(5);
		outb_p(0xFF,parallel_base);		//Toggle them, so we can capture an interrupt
		outb_p(0x00,parallel_base + 2);		//Disable interrupts on parrallel port
		parallel_irq = probe_irq_off(mask);	//Compare the ne state with the saved one, and show the result

		if(parallel_irq == 0){
			printk(KERN_INFO "PARALLELCHARDEV: No irq reported by probe, in %d try.\n", count);
			parallel_irq = -1;
			}
		//If more than one line has been activated, the result is
		//negative. We should service the interrupt (but the lpt port
		//doesn't need it) and loop over again. Do it at most 5 times
	}while(parallel_irq < 0 && count++ < 5);
	//If not find
	if(parallel_irq < 0)
		printk("PARALLELED: Kernel probe failed %i times, giving up!\n", count);
}


//*****************************************************************
//The init_module is the entry point for a Kernel module
//This is where we register facilities and data structures
static int __init parallel_led_init(void){
	//First, registering parallel ports
	if(!request_region(parallel_base, parallel_ports, DRIVER_NAME)){
		printk(KERN_ALERT "PARALLELLED: Ports %X to %X Could Not Be Registered!\n", parallel_base, parallel_base + parallel_ports - 1);
		goto port_error;
	}
	
	//Second, probing and registering the irq number
	//The method to do this is depend on the moule parameter; probe_mode
	if(probe_mode == 0)
		parallel_kernel_probing();
	else
		parallel_direct_probing();
	
	//Third, If we could obtain the IRQ number then we could register the interrupt handler
	if(parallel_irq >= 0){
		if(request_irq(parallel_irq, parallel_interrupt_handler, IRQF_NO_SUSPEND, DRIVER_NAME, NULL)){
			printk(KERN_ALERT "PARALLELLED: Interrupt Handler Registeration on %d IRQ Failure!\n", parallel_irq);
			goto interrupt_error;
		}
	}
	else{
		printk(KERN_ALERT "PARALLELLED: %s Interrupt Probing Failure!\n", probe_mode == 0? "Kernel Assissted" : "Do-it-Yourself");
		goto interrupt_error;
	}
	
	//Fourth, set the parallel port to initial zero state
	outb(0x00, parallel_base);
	wmb();
	udelay(7);
	
	//Fifth, registering timer and other tools
	timer_expire_delay = parallel_delay;
	
	parallel_timer.function = parallel_timer_funtion;
	parallel_timer.expires = jiffies + timer_expire_delay;
	add_timer(&parallel_timer);
	
	//Sixth, creating procfs interface
	proc_file = proc_create(DRIVER_NAME, 0644, NULL, &fops);
	if(!proc_file){
		printk(KERN_ALERT "PARALLELLED: ProcFS [ /proc/%s ]Registration Failure!\n", DRIVER_NAME);
		goto proc_error;	
	}
	
	//Lastly, creating sysfs interface and add the attributes
	parallel_kobj = kobject_create_and_add(DRIVER_NAME, kernel_kobj);
	if (!parallel_kobj){
		printk(KERN_ALERT "PARALLELLED: KOBJECT Registration Failure!\n");
		goto sysfs_error;
	}
	
	if (sysfs_create_group(parallel_kobj, &parallel_attr_group)){
		printk(KERN_ALERT "PARALLELLED: SysFS Attribute Creation Failure!\n");
		kobject_put(parallel_kobj);
		goto sysfs_error;
	}
	
	return SUCCESS;
	
	//This is where we could undone what we have done in case of any error
	//Depending of which state is the source of the error, before exiting
	sysfs_error:
		remove_proc_entry(DRIVER_NAME, NULL);
	proc_error:
		del_timer(&parallel_timer);
		free_irq(parallel_irq, NULL);
	interrupt_error:
		release_region(parallel_base, parallel_ports);
	port_error:
		return -EFAULT;
}


//Module Cleanup is the exit point of a Kernel module
//The goal is to clean the slate and undo everything had happened in the
//init module fucntion
static void __exit parallel_led_exit(void){
	//Remove the kobject and its attributes
	kobject_put(parallel_kobj);
	//Remove the procfs entry and its functionality
	remove_proc_entry(DRIVER_NAME, NULL);
	//Stop and remove the timer
	del_timer(&parallel_timer);
	//Free irq and unregister the interrupt handler
	free_irq(parallel_irq, NULL);
	//Now there is no need for the parallel port
	//Free the registered port region
	release_region(parallel_base, parallel_ports);
}


//These macros provide aliases for init_module and cleanup_module functions
module_init(parallel_led_init);
module_exit(parallel_led_exit);
