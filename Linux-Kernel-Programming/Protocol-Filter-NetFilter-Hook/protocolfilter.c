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

//For iphdr, IP header structure
#include <linux/ip.h>
//For ip_hdrlen function
#include <net/ip.h>

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
#define MY_MODULE_NAME "protocolfilter"


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Net Filter Hook module which could filter all packets for a specified protocol");
MODULE_VERSION("1.0.2");


int hook_protocol = 0;
module_param(hook_protocol, int, 0);
MODULE_PARM_DESC(hook_protocol, "[UDP:0], TCP:1, IP:2, ICMP:3, L2TP:4, OSPF:5");


//Create a structure to hold an IP header
struct iphdr *ip_header;
struct net *netns;


//This is our hook function
unsigned int hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state){
	
	ip_header = (struct iphdr *) skb_network_header(skb);
	
	switch(hook_protocol){
		case 0:
			if(ip_header->protocol == IPPROTO_UDP){
				printk(KERN_INFO "PROTOCOLFILTER: UDP Packet Dropped\n");
				return NF_DROP;
			}
			break;
		case 1:
			if(ip_header->protocol == IPPROTO_TCP){
				printk(KERN_INFO "PROTOCOLFILTER: TCP Packet Dropped\n");
				return NF_DROP;
			}
			break;
		case 2:
			printk(KERN_INFO "PROTOCOLFILTER: IP Packet Dropped\n");
			return NF_DROP;
			break;
		case 3:
			if(ip_header->protocol == IPPROTO_ICMP){
				printk(KERN_INFO "PROTOCOLFILTER: ICMP Packet Dropped\n");
				return NF_DROP;
			}
			break;
		case 4:
			if(ip_header->protocol == 115){
				printk(KERN_INFO "PROTOCOLFILTER: L2TP Packet Dropped\n");
				return NF_DROP;
			}
			break;
		default:
			if(ip_header->protocol == 89){
				printk(KERN_INFO "PROTOCOLFILTER: OSPF Packet Dropped\n");
				return NF_DROP;
			}
		}
	return NF_ACCEPT;
}



//Netfilter hook_operations would be the key to add this functionality to he Kernel
static struct nf_hook_ops nfho ={
	.hook = hook_func,
	.hooknum = 1,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FIRST,
};


//Every Module must have an entry point which is known as init_module or
//some function that defines with module_init macros. init_module is the function
//that would call immidately after insmoding your module to set everything right
//and registering module functionality to the Kernel
static int __init protocol_filter_init(void){
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "PROTOCOLFILTER: Initialization.\n");
	printk(KERN_INFO "PROTOCOLFILTER: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Initializing network namespace
	netns = get_net(&init_net);
	//Now we have to just add our hook to the netfilter
	nf_register_net_hook(netns, &nfho);
	
	printk(KERN_INFO "PROTOCOLFILTER: Netfilter-Hook has been Registered.\n");
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//This is our clean up function
static void __exit protocol_filter_exit(void){
	//Remove proc filesystem entry from system
	printk(KERN_INFO "PROTOCOLFILTER: Cleaning Up.\n");
	printk(KERN_INFO "PROTOCOLFILTER: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);
	
	//Unregistering our hook from the system
	nf_unregister_net_hook(netns, &nfho);
	printk(KERN_INFO "PROTOCOLFILTER: Netfilter-Hook has been UN-Registered.\n");
	
	printk(KERN_INFO "PROTOCOLFILTER: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}

//Now we need to define init-module and cleanup_module aliases
module_init(protocol_filter_init);
module_exit(protocol_filter_exit);
