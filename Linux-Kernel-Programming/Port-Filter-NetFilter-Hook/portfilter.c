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
//For tcphdr, TCP header structure
#include <linux/tcp.h>
//For udphdr, UDP header structure
#include <linux/udp.h>

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
MODULE_DESCRIPTION("This is just a simple Net Filter Hook module which could filter all packets for a specified port");
MODULE_VERSION("1.0.2");



int hook_protocol = 4;
module_param(hook_protocol, int, 0);
MODULE_PARM_DESC(hook_protocol, "FTP:20 [0], SSH:22 [1], Telnet:23 [2], smtp:25 [3] , *SSL:443 [4], DNS:53 [5], HTTP:80 [6], Any:0-65535 [7]");

int hook_port = 443;
module_param(hook_port, int, 0);
MODULE_PARM_DESC(hook_port, "This is the port yu want to filter ranged from 0 to 65535 depend on what protocol you want to filter [default: 443]");


//Create a structure to hold an IP header
struct iphdr *ip_header;
//Create a structure to hold a UDP header
struct udphdr *udp_header;
//Create a structure to hold a TCP header
struct tcphdr *tcp_header;
//Network namespace pointer
struct net *netns;



//This is our hook function
unsigned int hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state){
	
	ip_header = (struct iphdr *) skb_network_header(skb);
	
	//This will filter any TCP requsts for the specified port
	if(ip_header && ip_header->protocol == IPPROTO_TCP){
		tcp_header = (struct tcphdr *) skb_transport_header(skb);
		if(tcp_header && (ntohs(tcp_header->dest) == (unsigned short) hook_port || ntohs(tcp_header->source) == (unsigned short) hook_port)){
			//The recieved packet is headed to the specified port, so we have to track it in our kernel log
			if(printk_ratelimit())
				printk(KERN_ALERT "PORTFILTER: TCP packet SRC:%d --> DST:%d\n", ntohs(tcp_header->source), ntohs(tcp_header->dest));
			return NF_DROP;
		}
	}
	
	//This will filter any TCP requsts for the specified port
	if(ip_header && ip_header->protocol == IPPROTO_UDP){
		udp_header = (struct udphdr *) skb_transport_header(skb);
		if(udp_header && (ntohs(udp_header->dest) == (unsigned short) hook_port || ntohs(udp_header->source) == (unsigned short) hook_port)){
			//The recieved packet is headed to the specified port, so we have to track it in our kernel log
			if(printk_ratelimit())
				printk(KERN_INFO "PORTFILTER: UDP packet SRC:%d --> DST:%d\n", ntohs(udp_header->source), ntohs(udp_header->dest));
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
static int __init port_filter_init(void){
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "PORTFILTER: Initialization.\n");
	printk(KERN_INFO "PORTFILTER: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Decide on module parameters
	switch(hook_protocol){
		case 0:
			hook_port = 20;
			printk(KERN_INFO "PORTFILTER: The selected FTP protocol port (%d) to block\n", hook_port);
			break;
		case 1:
			hook_port = 22;
			printk(KERN_INFO "PORTFILTER: The selected SSH protocol port (%d) to block\n", hook_port);
			break;
		case 2:
			hook_port = 23;
			printk(KERN_INFO "PORTFILTER: The selected Telnet protocol port (%d) to block\n", hook_port);
			break;
		case 3:
			hook_port = 25;
			printk(KERN_INFO "PORTFILTER: The selected SMTP protocol port (%d) to block\n", hook_port);
			break;
		case 4:
			hook_port = 443;
			printk(KERN_INFO "PORTFILTER: The selected SSL protocol port (%d) to block\n", hook_port);
			break;
		case 5:
			hook_port = 53;
			printk(KERN_INFO "PORTFILTER: The selected DNS protocol port (%d) to block\n", hook_port);
			break;
		case 6:
			hook_port = 80;
			printk(KERN_INFO "PORTFILTER: The selected Web protocol port (%d) to block\n", hook_port);
			break;
		default:
			printk(KERN_INFO "PORTFILTER: The selected port to block is %d\n", hook_port);
	}
	
	
	
	//Initializing network namespace
	netns = get_net(&init_net);
	//Now we have to just add our hook to the netfilter
	nf_register_net_hook(netns, &in_nfho);
	nf_register_net_hook(netns, &out_nfho);
	
	printk(KERN_INFO "PORTFILTER: Netfilter-Hooks have been Registered.\n");
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//This is our clean up function
static void __exit port_filter_exit(void){
	//Remove proc filesystem entry from system
	printk(KERN_INFO "PORTFILTER: Cleaning Up.\n");
	printk(KERN_INFO "PORTFILTER: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);
	
	//Unregistering our hook from the system
	nf_unregister_net_hook(netns, &in_nfho);
	nf_unregister_net_hook(netns, &out_nfho);
	printk(KERN_INFO "PORTFILTER: Netfilter-Hooks have been UN-Registered.\n");
	
	printk(KERN_INFO "PORTFILTER: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(port_filter_init);
module_exit(port_filter_exit);
