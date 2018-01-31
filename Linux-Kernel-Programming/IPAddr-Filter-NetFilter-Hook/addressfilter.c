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
#define MY_MODULE_NAME "addressfilter"



//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Net Filter Hook module which could filter all packets for a specified IP address");
MODULE_VERSION("1.0.2");




//This is the IP address we want to filter on incoming and outgoing traffic

int hook_address = 0x08080808;
module_param(hook_address, int, 0);
MODULE_PARM_DESC(hook_address, "This is the IP address you want to filter in hexadecimal");
unsigned char ip_string[4], saddr[4], daddr[4];
int i;

//Create a structure to hold an IP header
struct iphdr *ip_header;
//Network namespace pointer
struct net *netns;


//This function will convert IP address to character string
char * change_address_to_string(unsigned int addr){
	for(i=0; i<4; i++)
		ip_string[i] = (addr >> i*8) & 0xFF;
	return ip_string;
}


//This is our hook function
unsigned int hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state){
	ip_header = (struct iphdr *) skb_network_header(skb);
	//This will filter any TCP requsts for the specified port
	if(ip_header){
		//The recieved packet is headed to the specified IP, so we have to track it in our kernel log
		if(hook_address == (unsigned int) ip_header->saddr || hook_address == (unsigned int) ip_header->daddr){
			if(printk_ratelimit()){
				unsigned char *saddr = change_address_to_string((unsigned int) ip_header->saddr);
				unsigned char *daddr = change_address_to_string((unsigned int) ip_header->daddr);
				printk(KERN_INFO "IPADDRESSFILTER: %s packet SRC:(%d.%d.%d.%d) --> DST: (%d.%d.%d.%d)\n", ip_header->protocol == IPPROTO_TCP ? "TCP" : "UDP", saddr[0], saddr[1], saddr[2], saddr[3], daddr[0], daddr[1], daddr[2], daddr[3]);
				}
			return NF_DROP;
			}
		}
	return NF_ACCEPT;
}



//Netfilter hook_operations would be the key to add this functionality to he Kernel
static struct nf_hook_ops in_nfho ={
	.hook = hook_func,
	.hooknum = 1,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FIRST,
};


static struct nf_hook_ops out_nfho ={
	.hook = hook_func,
	.hooknum = 5,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FIRST,
};

//Every Module must have an entry point which is known as init_module or
//some function that defines with module_init macros. init_module is the function
//that would call immidately after insmoding your module to set everything right
//and registering module functionality to the Kernel
static int __init ipaddr_filter_init(void){
	unsigned char *hook_address_string = change_address_to_string(hook_address);
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "IPADDRESSFILTER: Initialization.\n");
	printk(KERN_INFO "IPADDRESSFILTER: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);
	printk(KERN_ALERT "IPADDRESSFILTER: We will filter (%d.%d.%d.%d) IP address:\n",
		hook_address_string[0], hook_address_string[1], hook_address_string[2], hook_address_string[3]);
	
	
	//Initializing network namespace
	netns = get_net(&init_net);
	//Now we have to just add our hook to the netfilter
	nf_register_net_hook(netns, &in_nfho);
	nf_register_net_hook(netns, &out_nfho);
	printk(KERN_INFO "IPADDRESSFILTER: Netfilter-Hooks have been Registered.\n");
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//This is our clean up function
static void __exit ipaddr_filter_exit(void){
	//Remove proc filesystem entry from system
	printk(KERN_INFO "IPADDRESSFILTER: Cleaning Up.\n");
	printk(KERN_INFO "IPADDRESSFILTER: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);
	
	//Unregistering our hook from the system
	nf_unregister_net_hook(netns, &in_nfho);
	nf_unregister_net_hook(netns, &out_nfho);
	printk(KERN_INFO "IPADDRESSFILTER: Netfilter-Hooks have been UN-Registered.\n");
	
	printk(KERN_INFO "IPADDRESSFILTER: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(ipaddr_filter_init);
module_exit(ipaddr_filter_exit);
