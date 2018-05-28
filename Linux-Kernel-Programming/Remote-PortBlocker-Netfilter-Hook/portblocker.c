//We are going to do some kernel programming
#include <linux/kernel.h>
//Obviously we are creating a module
#include <linux/module.h>
//Just for using macros here
#include <linux/init.h>
//For Some String Functions
#include <linux/string.h>

//For using basic filesystem
#include <linux/fs.h>
//For using proc filesystem
#include <linux/proc_fs.h>
//We want to copy IPs and PORTs from user-space to kernel-space
#include <asm/uaccess.h>

//What do you expect really, we want to manipulate network packets
#include <linux/netfilter.h>
//And we are going to hook some default behaviors to IPv4 packets
#include <linux/netfilter_ipv4.h>
//For net_device, network device structue
#include <linux/netdevice.h>
//For sk_buff, socket buffer structure
#include <linux/skbuff.h>

//For iphdr, IP header structure
#include <linux/ip.h>
//For ip_hdrlen function
#include <net/ip.h>
//For tcphdr, TCP header structure
#include <linux/tcp.h>
//For udphdr, UDP header structure
#include <linux/udp.h>
//For icmphdr, ICMP header structure
#include <linux/icmp.h>
//For network namespaces
#include <net/net_namespace.h>


//For accessing MAC and IP addresses
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

//We need to define Magic Code
#define magic_code 0x5B


//This module name pre-processor const
#define my_module_name "remoteportblocker"
//Maximum size of the buffer which use to read and write in procfs
#define PROCFS_MAX_SIZE 2048

//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("Simple Port Blocker, will block a single port to any kind of traffics, unless it recieve an ICMP Magic packet, then the port will open to the sender till the next Magic packet arrives");
MODULE_VERSION("1.0.2");


//Create a structure to hold a Socket buffer
struct sk_buff *sock_buff;
//Create a structure to hold a UDP header
struct udphdr *udp_header;
//Create a structure to hold a TCP header
struct tcphdr *tcp_header;
//Create a structure to hold an IP header
struct iphdr *ip_header;
//Create a structure to hold an ICMP header
struct icmphdr *icmp_header;

struct net *netns;


//Creating a proc directory entry structure
static struct proc_dir_entry* simpleportblocker;

//We need some variable to hold our favorite IP and PORT
static unsigned char ip_address[] = "\xef\x00\x00\x01";                    
unsigned char port[] = "\x00\x00";

//This is our flag, 0 means we didn't recieve magic packet, and 1 means we recieved one
static int magic_flag = 0;

//This command-line argument variable wil define the PORTBLOCKER default behavior
//behavior global variable can get two values, 0:Block, 1:Pass
int behavior = 1;
module_param(behavior, int, 0);
MODULE_PARM_DESC(behavior, "Set default behavior of the module, 0:Block All Ports, [1:Allow All Ports]");


//SIMPLE PORT BLOCKER USER MANUAL ;)
//You can use "cat /proc/simpleportblocker" to see which port is about to block
//Also you can "echo PORT# > /proc/simpleportblocker" to set a port to block traffic on it
//If module recieve a Magic ICMP packet, then the sender can access to the port
//If module recieve the second Magic ICMP packet from the same sender, then the port will close
//There is not possible for two sender to access a port on the same time
//Access to a port will report to the Magic ICMP packet sender with a Reply ICMP packet



//We will read and write to procfs entry, so we need to define a buffer
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

//If user try to cat our module procfile, then this function would call
static ssize_t procfs_read(struct file *filp, char *buffer,size_t length,loff_t * offset){
	
	static int finished = 0;
	//We return 0 to indicate end of file, that we have
	//no more information. Otherwise, processes will
	//continue to read from us in an endless loop. 
	if (finished) {
		printk(KERN_INFO "PORTBLOCKER: Port (from /proc/%s) showed to user\n", my_module_name);
		finished = 0;
		return 0;
		}
	
	finished = 1;
		
	//We use put_to_user to copy the string from the kernel's
	//memory segment to the memory segment of the process
	//that called us. get_from_user, BTW, is
	//used for the reverse. 
	if (copy_to_user(buffer, procfs_buffer, procfs_buffer_size) ) {
		return -EFAULT;
		}

	//Return the number of bytes "read"
	return procfs_buffer_size;	 
}


//If user try to echo a rule to our module procfile, then this function would call
static ssize_t procfs_write(struct file *file, const char *buffer, size_t length, loff_t * off){

	//Get buffer size
	procfs_buffer_size = length;
	if (procfs_buffer_size > PROCFS_MAX_SIZE ) {
		procfs_buffer_size = PROCFS_MAX_SIZE;
		}
	
	//Write data to the buffer
	if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size) ) {
		return -EFAULT;
		}
	
	//Write the buffer data to our port variable
	if(!strcpy(port, procfs_buffer)){
		printk(KERN_INFO "PORTBLOCKER: String (%s) is not a valid port (from /proc/%s) entry\n", procfs_buffer, my_module_name);
		return -EFAULT;
	}
	printk(KERN_INFO "PORTBLOCKER: Port (%s) just entered (from /proc/%s) entry\n", procfs_buffer, my_module_name);

	//Return the number of bytes "write"
	return procfs_buffer_size;
}


//The file is opened - we don't really care about that, but it does mean we need to increment the module's reference count. 
int procfs_open(struct inode *inode, struct file *file){
	try_module_get(THIS_MODULE);
	return 0;
}

//The file is closed - again, interesting only because of the reference count.
int procfs_close(struct inode *inode, struct file *file){
	module_put(THIS_MODULE);
	return 0;
}



//Really, This is our Port Blocker :)
//This is hook number 2, that means it will run after any number 1 hooks, so we could capture our Magic ICMP packet
//in the following hook and then block all ports including ICMP port in this hook, and the module will work properly
unsigned int block_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state){
	
	sock_buff = skb;
	//Accept any UNCLEAN packets
	if(!sock_buff)
		return NF_ACCEPT;
		
	//Accept any NON-IP packets
	ip_header = (struct iphdr *) skb_network_header(sock_buff);
	if(!ip_header)
		return NF_ACCEPT;
		
	//This will filter any TCP requsts for the specified port
	if(ip_header->protocol == IPPROTO_TCP){
		tcp_header = (struct tcphdr *) (skb_transport_header(sock_buff) + ip_hdrlen(sock_buff));
		if(tcp_header && tcp_header->dest == *(unsigned short *) port){
			//The recieved packet is headed to the specified port, so we have to track it in our kernel log
			unsigned char* Saddr = (unsigned char*) ip_header->saddr;
			unsigned char* Daddr = (unsigned char*) ip_header->daddr;
			printk(KERN_INFO "PORTBLOCKER: TCP packet SRC:(%d.%d.%d.%d):%d --> DST: (%d.%d.%d.%d):%d\n", 
				Saddr[0], Saddr[1], Saddr[2], Saddr[3], ntohs(tcp_header->source), 
				Daddr[0], Daddr[1], Daddr[2], Daddr[3], ntohs(tcp_header->dest));
			//if we recived the Magic ICMP packet before and the current packet to the specified port is come from that user accept that
			if(magic_flag == 1 && ip_header->saddr == *(unsigned int*) ip_address)
				return NF_ACCEPT;
			//Otherwise, if we recieve a packet for specified port, we will drop it, because the port is close to anyone except the sender of the Magic ICMP packet
			else
				return NF_DROP;
		}
	}

	//This will filter any UDP requsts for the specified port	
	if(ip_header->protocol = IPPROTO_UDP){
		udp_header = (struct udphdr *) (skb_transport_header(sock_buff) + ip_hdrlen(sock_buff));
		if(udp_header && udp_header->dest == *(unsigned short *) port){
			//The recieved packet is headed to the specified port, so we have to track it in our kernel log
			unsigned char* Saddr = (unsigned char*) ip_header->saddr;
			unsigned char* Daddr = (unsigned char*) ip_header->daddr;
			printk(KERN_INFO "PORTBLOCKER: UDP packet SRC:(%d.%d.%d.%d):%d --> DST: (%d.%d.%d.%d):%d\n", 
				Saddr[0], Saddr[1], Saddr[2], Saddr[3], ntohs(tcp_header->source), 
				Daddr[0], Daddr[1], Daddr[2], Daddr[3], ntohs(tcp_header->dest));
			//if we recived the Magic ICMP packet before and the current packet to the specified port is come from that user accept that
			if(magic_flag == 1 && ip_header->saddr == *(unsigned int*) ip_address)
				return NF_ACCEPT;
			//Otherwise, if we recieve a packet for specified port, we will drop it, because the port is close to anyone except the sender of the Magic ICMP packet
			else
				return NF_DROP;
		}
	}

	//Otherwise, the recieved packet headed to another port, we can decide over it by our command line argument
	return behavior == 1 ? NF_ACCEPT : NF_DROP;
}



//Really, This is our Magic ICMP packet response function
unsigned int magic_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state){
	
	sock_buff = skb;
	//Accept any UNCLEAN packets
	if(!sock_buff)
		return NF_ACCEPT;
		
	//Accept any NON-IP packets
	ip_header = (struct iphdr *) skb_network_header(sock_buff);
	if(!ip_header)
		return NF_ACCEPT;

	//We are watching for any ICMP traffic
	if(ip_header->protocol == IPPROTO_ICMP){
		icmp_header = (struct icmphdr *)(skb_transport_header(sock_buff) + ip_hdrlen(sock_buff));
		//We have to check this ICMP packet is the Echo Magic packet or waht?
		if(icmp_header && icmp_header->code == magic_code && icmp_header->type == ICMP_ECHO){
			//The recieved packet the Magic ICMP packet, so we have to track it in our kernel log
			unsigned char* Saddr = (unsigned char*) ip_header->saddr;
			unsigned char* Daddr = (unsigned char*) ip_header->daddr;		
			printk(KERN_INFO "PORTBLOCKER: Magic ICMP packet SRC: (%d.%d.%d.%d) --> DST: (%d.%d.%d.%d), type: %d - ICMP code: %d\n",
					Saddr[0], Saddr[1], Saddr[2], Saddr[3], Daddr[0], Daddr[1], Daddr[2], Daddr[3], icmp_header->type, icmp_header->code);
					
			//We have to check now, that this is the first time e recieved a magic packet or not
			if(magic_flag == 0){
				//Set the flag to open the port
				magic_flag = 1;
				//Capture source IP address of the Magic ICMP packet sender
				strcpy(ip_address, ip_header->saddr);
			}
			else{
				//Reset the flag to close the port
				magic_flag = 0;
			}	
		}
	}
	//This is the first hook, we could accept all teraffic for all ports, so the next level hook which means in this case
	//the above hook, will filter all packet due to our command line rule and the status of recieved Magic ICMP packet
	return NF_ACCEPT;
}



//Setting file_operations instant functions
static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = procfs_read,
	.write = procfs_write,
	.open = procfs_open,
	.release = procfs_close,
};


//There are 5 types of available hooks for IPv4
//NF_IP_PRE_ROUTING   1		After sanity checks, before routing decisions.
//NF_IP_LOCAL_IN      2		After routing decisions if packet is for this host.
//NF_IP_FORWARD       3		If the packet is destined for another interface.
//NF_IP_LOCAL_OUT     4		For packets coming from local processes on their way out.
//NF_IP_POST_ROUTING  5		Just before outbound packets "hit the wire".


//Also Netfilter return codes could be categorized in 5
//NF_DROP        Discard the packet.
//NF_ACCEPT      Keep the packet.
//NF_STOLEN      Forget about the packet.
//NF_QUEUE       Queue packet for userspace.
//NF_REPEAT      Call this hook function again

//In order to set Protocol Family, we could use PF_INET for IPv4 and PF_INET6 for IPv6


//Netfilter hook_operation struct settings, for for blocking a port
static struct nf_hook_ops block_nfho = {
	.hook = block_hook_func,
	.hooknum = 2,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FIRST,
};

//Netfilter hook_operation struct settings, for recieving Magic ICMP packet
static struct nf_hook_ops magic_nfho = {
	.hook = magic_hook_func,
	.hooknum = 2,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FIRST,
};


//This would be our module start up point
static int __init port_blocker_init(void){

	printk(KERN_INFO "PORTBLOCKER: Initializing ...\n");
	strcpy(procfs_buffer, port);
	//Create /proc entry for this module
	simpleportblocker = proc_create(my_module_name, 0 , NULL, &fops);

	//Throw an error message in kernel log if cannot create proc entry
	if(!simpleportblocker){
		//Because of the fact that procfs is mounted on system main memory,
		//If we would not able to create an entry, this means we are runout of memory, so ...
		printk(KERN_ALERT "PORTBLOCKER: Error, not enough memory, so cannot create entry under /proc/%s.\n", my_module_name);
		return -ENOMEM;
	}

	//Otherwise, we could create a /proc entry
	printk(KERN_INFO "PORTBLOCKER: Module entry created under /proc/%s.\n", my_module_name);
	
	netns = get_net(&init_net);
	//Now we have to add ours hook to netfilter
	nf_register_net_hook(netns, &block_nfho);
	nf_register_net_hook(netns, &magic_nfho);
	printk(KERN_INFO "PORTBLOCKER: Module hooks registered to the Netfilter.\n");


	//Print our user manual in kernel logs, boring really ;)
	printk(KERN_INFO "PORTBLOCKER: You can use \"cat /proc/simpleportblocker\" to see which port is about to block.\n");
	printk(KERN_INFO "PORTBLOCKER: Also you can \"echo PORT# > /proc/simpleportblocker\" to set a port to block traffic on it.\n");
	printk(KERN_INFO "PORTBLOCKER: If module recieve a Magic ICMP packet, then the sender can access to the port.\n");
	printk(KERN_INFO "PORTBLOCKER: If module recieve the second Magic ICMP packet from the same sender, then the port will close.\n");
	printk(KERN_INFO "PORTBLOCKER: There is not possible for two sender to access a port on the same time.\n");
	printk(KERN_INFO "PORTBLOCKER: Access to a port will report to the Magic ICMP packet sender with a Reply ICMP packet.\n");
	return 0;
}


//Now, due to end this module, its time to clean up the mess
static void __exit port_blocker_exit(void){

	printk(KERN_INFO "PORTBLOCKER: Finalizing ...\n");

	//Remove /proc entry from system
	remove_proc_entry(my_module_name, NULL);
	printk(KERN_INFO "PORTBLOCKER: Module entry removed from /proc/%s.\n", my_module_name);

	//Unregistering our hooks to netfilter
	nf_unregister_net_hook(netns, &block_nfho);
	nf_unregister_net_hook(netns, &magic_nfho);
	printk(KERN_INFO "PORTBLOCKER: Module hooks unregistered.\n");

	printk(KERN_INFO "PORTBLOCKER: Module ended here.\n");
}


//Definition of init_module and cleanup_modules with user-defined functions
//Now we need to define init-module and cleanup_module aliases
module_init(port_blocker_init);
module_exit(port_blocker_exit);
