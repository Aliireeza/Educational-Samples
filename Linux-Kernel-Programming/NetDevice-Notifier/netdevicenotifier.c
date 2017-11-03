//We are going to do some kernel programming
#include <linux/kernel.h>
//Obviously we are creating a module
#include <linux/module.h>
//Just for using macros here
#include <linux/init.h>
//This contains some macros for notifier chain
#include <linux/notifier.h>
//For registering a net device notifier
#include <linux/netdevice.h>

//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple module based on Linux Kernel notifier chains to monitor net device activities");
MODULE_VERSION("1.0.2");


static int net_device_notify(struct notifier_block *self, unsigned long action, void *dev) {
	//Who called the notifier_block?
 	printk(KERN_INFO "SIMPLENETNOFIFIER: Device Notifier function, Process(%s:%i)\n", current->comm, current->pid);

	//Decide on different action
	switch(action){ 
		case NETDEV_CHANGE: 
			printk(KERN_INFO "SIMPLENETNOFIFIER: Net Device Change, NOTIFIER.\n");
			break;
		case NETDEV_CHANGEMTU: 
			printk(KERN_INFO "SIMPLENETNOFIFIER: Net Device MTU Changed, NOTIFIER.\n");
			break;
		case NETDEV_CHANGEADDR: 
			printk(KERN_INFO "SIMPLENETNOFIFIER: Net Device Address Changed, NOTIFIER.\n");
			break;
		case NETDEV_CHANGENAME: 
			printk(KERN_INFO "SIMPLENETNOFIFIER: Net Device Name Changed, NOTIFIER.\n");
			break;	

		default:
				return NOTIFY_DONE; 	
			} 
		
	return NOTIFY_OK; 
} 



//Using notifiers for identifying actions on usb
static struct notifier_block net_device_nb = { 
	.notifier_call = net_device_notify, //This fuction will call whenever the notifier triggers
};


//This would be our module start up point
static int __init net_notifier_init(void){

	//Registering a Notifier
	int error = 0;
	error = register_netdevice_notifier(&net_device_nb);
	if(error){
		printk(KERN_ALERT "SIMPLENETNOFIFIER: Notifier registeration failure.\n");
		return error;
		}
	printk(KERN_INFO "SIMPLENETNOFIFIER: Notifier registered successfully.\n");

	return error;
}


//Now, due to end this module, its time to clean up the mess
static void __exit net_notifier_exit(void){
	//Unregistering driver and notifier
	unregister_netdevice_notifier(&net_device_nb);

	printk(KERN_INFO "SIMPLENETNOFIFIER: Module deregistered.\n");
}


//Definition of init_module and cleanup_modules with user-defined functions
module_init(net_notifier_init);
module_exit(net_notifier_exit);
