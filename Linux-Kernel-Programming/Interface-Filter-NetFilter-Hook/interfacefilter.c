//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>

//What do you hink this standsfor, netfilter hooks of course
#include <linux/netfilter.h>
//And we want our hook to act on some defaults attributes of IP_V4 packets
#include <linux/netfilter_ipv4.h>
//For sk_buff structure to hold sockets
#include <linux/skbuff.h>
//For net_device structure to hold our network device
#include <linux/netdevice.h>

//For finding the parent process ID of the module
#include <asm/current.h>
//For using task_struct
#include <linux/sched.h>
//For some string functions
#include <linux/string.h>
//For network namespaces
#include <net/net_namespace.h>

//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define MY_MODULE_NAME "interfacefilter"


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Net Filter Hook module which could filter all packets for a specified interface");
MODULE_VERSION("1.0.2");


int hook_interface = 2;
module_param(hook_interface, int, 0);
MODULE_PARM_DESC(hook_interface, "eth0:0, wlan:1, [wlp3s0:2], virbr0:3, enl3p0:4, lo:5");

int hook_direction = 0;
module_param(hook_direction, int, 0);
MODULE_PARM_DESC(hook_direction, "input:-1, [bidirection:0], output:1");

static char interface_name[8];
//Network namespace pointer
struct net *netns;


//This is our hook function
unsigned int in_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state){
	if(strcmp(state->in->name, interface_name) == 0){
		if(printk_ratelimit())
				printk(KERN_ALERT "INTERFACEFILTER: Packet filtered for Input interface on %s", interface_name);
		return NF_DROP;
		}
	else
		return NF_ACCEPT;
}


unsigned int out_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state){
	if(strcmp(state->out->name, interface_name) == 0){
		if(printk_ratelimit())
				printk(KERN_ALERT "INTERFACEFILTER: Packet filtered for Output interface on %s", interface_name);
		return NF_DROP;
		}
	else
		return NF_ACCEPT;
}



//Netfilter hook_operations would be the key to add this functionality to he Kernel
static struct nf_hook_ops in_nfho ={
	.hook = in_hook_func,
	.hooknum = 1,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FIRST,
};


static struct nf_hook_ops out_nfho ={
	.hook = out_hook_func,
	.hooknum = 5,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FIRST,
};



//Every Module must have an entry point which is known as init_module or
//some function that defines with module_init macros. init_module is the function
//that would call immidately after insmoding your module to set everything right
//and registering module functionality to the Kernel
static int __init interface_filter_init(void){
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "INTERFACEFILTER: Initialization.\n");
	printk(KERN_INFO "INTERFACEFILTER: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Decide on module parameters
	switch(hook_interface){
		case 0:
			strcpy(interface_name, "eth0");
			break;
		case 1:
			strcpy(interface_name, "wlan0");
			break;
		case 2:
			strcpy(interface_name, "wlp3s0");
			break;
		case 3:
			strcpy(interface_name, "virbr0");
			break;
		case 4:
			strcpy(interface_name, "enl3p0");
			break;
		default:			
			strcpy(interface_name, "lo");
	}
	
	printk(KERN_INFO "INTERFACEFILTER: You have entere interface module parameter %d so the interface name will be %s", hook_interface, interface_name);
	
	//Initializing network namespace
	netns = get_net(&init_net);
	//Now we have to just add our hook to the netfilter
	if(hook_direction < 0)
		nf_register_net_hook(netns, &in_nfho);
	if(hook_direction > 0)
		nf_register_net_hook(netns, &out_nfho);
	else{
		nf_register_net_hook(netns, &in_nfho);
		nf_register_net_hook(netns, &out_nfho);
	}
	
	printk(KERN_INFO "INTERFACEFILTER: Netfilter-Hooks have been Registered.\n");
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//This is our clean up function
static void __exit interface_filter_exit(void){
	//Remove proc filesystem entry from system
	printk(KERN_INFO "INTERFACEFILTER: Cleaning Up.\n");
	printk(KERN_INFO "INTERFACEFILTER: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);
	
	//Unregistering our hook from the system
	if(hook_direction < 0)
		nf_unregister_net_hook(netns, &in_nfho);
	if(hook_direction > 0)
		nf_unregister_net_hook(netns, &out_nfho);
	else{
		nf_unregister_net_hook(netns, &in_nfho);
		nf_unregister_net_hook(netns, &out_nfho);
	}
	
	printk(KERN_INFO "INTERFACEFILTER: Netfilter-Hooks have been UN-Registered.\n");
	
	printk(KERN_INFO "INTERFACEFILTER: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}

//Now we need to define init-module and cleanup_module aliases
module_init(interface_filter_init);
module_exit(interface_filter_exit);
