//We are going to do some kernel programming
#include <linux/kernel.h>
//Obviously we are creating a module
#include <linux/module.h>
//Just for using macros here
#include <linux/init.h>
//For obtaining PID and process name
#include <linux/sched.h>

//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple module based on Linux Kernel notifier chains to monitor modules activities");
MODULE_VERSION("1.0.2");

static int error;


static int module_notify(struct notifier_block *self, unsigned long action, void *data) {
	//Who called the notifier_block?
 	printk(KERN_INFO "SIMPLEMODULENOFIFIER: Notifier function, Process(%s:%i)\n", current->comm, current->pid);

	//Know which module has triggered this callback function
	struct module *target_module = data;
	
	
	//Decide on different action
	switch(action){ 
		case MODULE_STATE_COMING: 
			printk(KERN_INFO "SIMPLEMODULENOFIFIER: Module \"%s\" has been loaded.\n", target_module->name);
			break;
		case MODULE_STATE_LIVE: 
			printk(KERN_INFO "SIMPLEMODULENOFIFIER: Module \"%s\" has been set completely.\n", target_module->name);
			break;
		case MODULE_STATE_GOING: 
			printk(KERN_INFO "SIMPLEMODULENOFIFIER: Module \"%s\" has been unloaded.\n", target_module->name);
			break;
		case MODULE_STATE_UNFORMED: 
			printk(KERN_INFO "SIMPLEMODULENOFIFIER: Module \"%s\" is still unformed.\n", target_module->name);
			break;
		default:
			printk(KERN_INFO "SIMPLEMODULENOFIFIER: Module \"%s\" has experienced an unexpected situation.\n", target_module->name);
		        return NOTIFY_DONE;
		} 
		
	return NOTIFY_OK; 
} 


//Using notifiers for identifying actions on loadable Kernel modules
static struct notifier_block module_nb = { 
	.notifier_call = module_notify, //This fuction will call whenever the notifier triggers
};


//This would be our module start up point
static int __init module_notifier_init(void){
	printk(KERN_INFO "SIMPLEMODULENOFIFIER: Init module, Process(%s:%i)\n", current->comm, current->pid);
	//Registering a Notifier
	error = register_module_notifier(&module_nb);
	if(error){
		printk(KERN_ALERT "SIMPLEMODULENOFIFIER: Error in registering module notifier\n");
		return error;
		}
	printk(KERN_INFO "SIMPLEMODULENOFIFIER: Module notifier registered successfully\n");

	return error;
}


//Now, due to end this module, its time to clean up the mess
static void __exit module_notifier_exit(void){
	printk(KERN_INFO "SIMPLEMODULENOFIFIER: Init module, Process(%s:%i)\n", current->comm, current->pid);
	//Unregistering module notifier
	unregister_module_notifier(&module_nb);

        printk(KERN_INFO "SIMPLEMODULENOFIFIER: Module notifier deregistered\n");
}


//Definition of init_module and cleanup_modules with user-defined functions
module_init(module_notifier_init);
module_exit(module_notifier_exit);
