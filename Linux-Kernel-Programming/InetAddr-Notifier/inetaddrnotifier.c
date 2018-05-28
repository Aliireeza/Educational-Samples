//We are going to do some kernel programming
#include <linux/kernel.h>
//Obviously we are creating a module
#include <linux/module.h>
//Just for using macros here
#include <linux/init.h>
//This contains some macros for notifier chain
#include <linux/notifier.h>
//For registering a net device notifier
#include <linux/inetdevice.h>

//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple module based on Linux Kernel notifier chains to monitor net address changes");
MODULE_VERSION("1.0.2");


static int inet_address_notify(struct notifier_block *self, unsigned long action, void *param){
	//Decide on different action
	char action_string[30];
	switch(action){
		case NETDEV_UP:
			strcpy(action_string, "Net Device UP");
			break;
		case NETDEV_DOWN:
			strcpy(action_string, "Net Device DOWN");
			break;
		case NETDEV_REBOOT:
			strcpy(action_string, "Net Device REBOOT");
			break;
		case NETDEV_CHANGE:
			strcpy(action_string, "Net Device STATE CHANGE");
			break;
		case NETDEV_REGISTER:
			strcpy(action_string, "Net Device REGISTER");
			break;
		case NETDEV_UNREGISTER:
			strcpy(action_string, "Net Device UNREGISTER");
			break;	
		default:
			strcpy(action_string, "UNKOWN");
	}
	printk(KERN_INFO "SIMPLENETNOFIFIER: inetaddr change, NOTIFIER(%ld:%s).\n", action, action_string);

	return NOTIFY_OK; 
} 



//Using notifiers for identifying actions on usb
static struct notifier_block inet_address_nb = { 
	.notifier_call = inet_address_notify, //This fuction will call whenever the notifier triggers
};


//This would be our module start up point
static int __init net_notifier_init(void){

	//Registering a Notifier
	int error = 0;
	error = register_inetaddr_notifier(&inet_address_nb);
	if(error){
		printk(KERN_ALERT "SIMPLENETNOFIFIER: Notifier registeration failure.\n");
		return error;
		}
	printk(KERN_INFO "SIMPLENETNOFIFIER: Notifier registered successfully.\n");

	return 0;
}


//Now, due to end this module, its time to clean up the mess
static void __exit net_notifier_exit(void){
	//Unregistering driver and notifier
	unregister_inetaddr_notifier(&inet_address_nb);

	printk(KERN_INFO "SIMPLENETNOFIFIER: Module deregistered.\n");
}


//Definition of init_module and cleanup_modules with user-defined functions
module_init(net_notifier_init);
module_exit(net_notifier_exit);
