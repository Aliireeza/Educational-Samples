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

//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define MY_MODULE_NAME "trafficfilter"


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Net Filter Hook module which could filter all input/output packets");
MODULE_VERSION("1.0.2");


//These are some module parameters and their initial default values
int hook_in_behavior = 0;
module_param(hook_in_behavior, int, 0);
MODULE_PARM_DESC(hook_in_behavior, "This is the default behavior for input traffic, [0:DROP] or 1:ACCEPT");

int hook_out_behavior = 0;
module_param(hook_out_behavior, int, 0);
MODULE_PARM_DESC(hook_out_behavior, "This is the default behavior for output traffic, [0:DROP] or 1:ACCEPT");

struct net *netns;


//This is our hook function for input traffic
unsigned int hook_in_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state){
	printk(KERN_ALERT "TRAFICFILTER: Capture an input packet\n");
	return hook_in_behavior == 1 ? NF_ACCEPT : NF_DROP;
}

//This is our hook function for output traffic
unsigned int hook_out_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state){
	printk(KERN_ALERT "TRAFICFILTER: Capture an output packet\n");
	return hook_out_behavior == 1 ? NF_ACCEPT : NF_DROP;
}



//There are 5 types of available hooks for netfilter
//NF_IP_PRE_ROUTING	1	After sanity check, before routing decisions
//NF_IP_LOCAL_IN	2	After routing decisions, if packet is destined for this host
//NF_IP_FORWARD		3	After routing decisions,If packet is destined for another interface
//NF_IP_LOCAL_OUT	4	For packets comming from local processes on their way out
//NF_IP_POST_ROUTING	5	Just before outbound packets hit the wire

//Also netfilter return codes could be categorized in five
//NF_DROP		Discard the packet
//NF_ACCEPT		Keep the packet and let it countinue
//NF_STOLEN		Forget about the packet
//NF_QUEUE		Queue packet immidiately for userspace
//NF_REPEAT		Call this hook functionfor this packet once again



//Netfilter hook_operations would be the key to add this functionality to he Kernel
static struct nf_hook_ops in_nfho ={
	.hook = hook_in_func,
	.hooknum = 1,
	.pf = PF_INET,
	.priv = NULL,
	.priority = NF_IP_PRI_FIRST,
};


static struct nf_hook_ops out_nfho ={
	.hook = hook_out_func,
	.hooknum = 5,
	.pf = PF_INET,
	.priv = NULL,
	.priority = NF_IP_PRI_FIRST,
};



//Every Module must have an entry point which is known as init_module or
//some function that defines with module_init macros. init_module is the function
//that would call immidately after insmoding your module to set everything right
//and registering module functionality to the Kernel
static int __init packet_filter_init(void){
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "TRAFFICFILTER: Initialization.\n");
	printk(KERN_INFO "TRAFFICFILTER: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Initialize the network namespace
	netns = get_net(&init_net);
	//Now we have to just add our hooks to the netfilter
	nf_register_net_hook(netns, &in_nfho);
	nf_register_net_hook(netns, &out_nfho);


	printk(KERN_INFO "TRAFFICFILTER: Netfilter-Hooks have been Registered.\n");
	
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//This is our clean up function
static void __exit packet_filter_exit(void){
	//Remove proc filesystem entry from system
	printk(KERN_INFO "TRAFFICFILTER: Cleaning Up.\n");
	printk(KERN_INFO "TRAFFICFILTER: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);
	
	//Unregistering our hooks from the system
	nf_unregister_net_hook(netns, &in_nfho);
	nf_unregister_net_hook(netns, &out_nfho);

	printk(KERN_INFO "TRAFFICFILTER: Netfilter-Hooks have been UN-Registered.\n");
	
	printk(KERN_INFO "TRAFFICFILTER: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(packet_filter_init);
module_exit(packet_filter_exit);
