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
//For finding the parent process ID of the module
#include <asm/current.h>
//For using task_struct
#include <linux/sched.h>
//For using diffrent time related functions
#include <linux/time.h>
//For obtaining CPU status
#include <linux/kernel_stat.h>
//For counting jiffies and HZ
#include <linux/jiffies.h>
//For using cputime_to_timespec function
#include <asm/cputime.h>
//For ktime_get_ts function to obtain uptime
#include <linux/timekeeping.h>
//For ioctl commands and macros
#include <linux/ioctl.h>
#include <asm/ioctl.h>
//For copy_to_user, copy_from_user, put_user
#include <asm/uaccess.h>

//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "systemtimes"


//These are our ioctl definition
#define MAGIC 'T'
#define IOC_MAXNR 12
#define IOCTL_UPTIME _IOR(MAGIC, 0, unsigned long)
#define IOCTL_IRQ_TIME _IOR(MAGIC, 1, unsigned long)
#define IOCTL_JIFFIES _IOR(MAGIC, 2, unsigned long)
#define IOCTL_USER_TIME _IOR(MAGIC, 3, unsigned long)
#define IOCTL_SYSTEM_TIME _IOR(MAGIC, 4, unsigned long)
#define IOCTL_NICE_TIME _IOR(MAGIC, 5, unsigned long)
#define IOCTL_IDLE_TIME _IOR(MAGIC, 6, unsigned long)
#define IOCTL_IOWAIT_TIME _IOR(MAGIC, 7, unsigned long)
#define IOCTL_GUEST_TIME _IOR(MAGIC, 8, unsigned long)
#define IOCTL_GUEST_NICE_TIME _IOR(MAGIC, 9, unsigned long)
#define IOCTL_STEAL_TIME _IOR(MAGIC, 10, unsigned long)
#define IOCTL_SOFTIRQ_TIME _IOR(MAGIC, 11, unsigned long)



//These are some useful information that could reveald with modinfo command
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple Kernel module using procfs, which could be used as ioctl handler in user space to obtain some system times");
MODULE_VERSION("1.0.2");

//Creating a proc directory entry structure
static struct proc_dir_entry* our_proc_file;

//This is a simple function which turn unsigned long to string
void ltoa(unsigned long value, char *ptr, int base){
	unsigned long t = 0, res = 0;
    unsigned long tmp = value;
    int count = 0;

    if (NULL == ptr)
      return NULL;

    if (tmp == 0)
      count++;

    while(tmp > 0){
      tmp = tmp/base;
      count++;
    }

    ptr += count;
    *ptr = '\0';

    do{
      res = value - base * (t = value / base);
      if (res < 10)
        * -- ptr = '0' + res;
      else if ((res >= 10) && (res < 16))
        * --ptr = 'A' - 10 + res;
    }while ((value = t) != 0);

    return(ptr);
}


//When device recive ioctl commands this function will perform the job depending on what kind of command it recieved
long proc_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	static int err = 0, retval = 0;

	//These are our primary variables to obtain CPU times in jiffies
	u64 user = 0, nice = 0, system = 0, idle = 0, iowait =0;
	u64 irq = 0 , softirq = 0, steal = 0, guest = 0, guest_nice = 0;
	//It is just a counter
	int i;
	//These are our output variables to convert times from primary variables to human readable seconds
	struct timespec calc_user, calc_nice, calc_system, calc_idle, calc_iowait;
	struct timespec calc_irq, calc_softirq, calc_steal, calc_guest, calc_guest_nice;

	char buff_uptime[30], buff_jiffies[30], buff_user[30], buff_nice[30], buff_system[30], buff_irq[30], buff_softirq[30], buff_steal[30], buff_guest[30], buff_guest_nice[30], buff_iowait[30], buff_idle[30];

	struct timespec calc_uptime;


	printk(KERN_INFO "IOCTLPROCFS: IOCTL Function, Process \"%s:%i\"\n", current->comm, current->pid);
	printk(KERN_INFO "IOCTLPROCFS: IOCTL Command, %d\n", cmd);

	if(_IOC_TYPE(cmd) != MAGIC || _IOC_NR(cmd) > IOC_MAXNR)
		return -ENOTTY;

	if(_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_READ, (void __user *) arg, _IOC_SIZE(cmd));
	if(_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_WRITE, (void __user *) arg, _IOC_SIZE(cmd));
	if(err)
		return -EFAULT;

	//Due to the fact that our signal is correct, now it is time to calculate time values, begin with uptime
	ktime_get_ts(&calc_uptime);

	//Now we are going to calculate other times for each CPUs
	for_each_possible_cpu(i){
		user += kcpustat_cpu(i).cpustat[CPUTIME_USER];
	 	nice += kcpustat_cpu(i).cpustat[CPUTIME_NICE];
		system += kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM];
	 	idle += kcpustat_cpu(i).cpustat[CPUTIME_IDLE];
	 	iowait += kcpustat_cpu(i).cpustat[CPUTIME_IOWAIT];
		irq += kcpustat_cpu(i).cpustat[CPUTIME_IRQ];
		softirq += kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ];
		steal += kcpustat_cpu(i).cpustat[CPUTIME_STEAL];
		guest += kcpustat_cpu(i).cpustat[CPUTIME_GUEST];
		guest_nice += kcpustat_cpu(i).cpustat[CPUTIME_GUEST_NICE];
	}

	//Then converting the results to timespec structs
	cputime_to_timespec(user, &calc_user);
	cputime_to_timespec(nice, &calc_nice);
	cputime_to_timespec(system, &calc_system);
	cputime_to_timespec(idle, &calc_idle);
	cputime_to_timespec(iowait, &calc_iowait);
	cputime_to_timespec(irq, &calc_irq);
	cputime_to_timespec(softirq, &calc_softirq);
	cputime_to_timespec(steal, &calc_steal);
	cputime_to_timespec(guest, &calc_guest);
	cputime_to_timespec(guest_nice, &calc_guest_nice);


	//for using them as string buffer, we have to convert them into string
	ltoa(calc_uptime.tv_sec, buff_uptime, 10);
	ltoa(jiffies, buff_jiffies, 10);
	ltoa(calc_user.tv_sec, buff_user, 10);
	ltoa(calc_nice.tv_sec, buff_nice, 10);

	ltoa(calc_system.tv_sec, buff_system, 10);
	ltoa(calc_idle.tv_sec, buff_idle, 10);
	ltoa(calc_iowait.tv_sec, buff_iowait, 10);
	ltoa(calc_irq.tv_sec, buff_softirq, 10);

	ltoa(calc_softirq.tv_sec, buff_softirq, 10);
	ltoa(calc_steal.tv_sec, buff_steal, 10);
	ltoa(calc_guest.tv_sec, buff_guest, 10);
	ltoa(calc_guest_nice.tv_sec, buff_guest_nice, 10);


	//Now we have all our times, so we have to rcognize the signal
	switch(cmd){
		case IOCTL_UPTIME:
			copy_to_user((int __user *) arg,  buff_uptime, 30);
			break;
		case IOCTL_IRQ_TIME:
			copy_to_user((int __user *) arg,  buff_irq, 30);
			break;
		case IOCTL_JIFFIES:
			copy_to_user((int __user *) arg,  buff_jiffies, 30);
			break;
		case IOCTL_USER_TIME:
			copy_to_user((int __user *) arg,  buff_user, 30);
			break;
		case IOCTL_SYSTEM_TIME:
			copy_to_user((int __user *) arg,  buff_system, 30);
			break;
		case IOCTL_NICE_TIME:
			copy_to_user((int __user *) arg,  buff_nice, 30);
			break;
		case IOCTL_IDLE_TIME:
			copy_to_user((int __user *) arg,  buff_idle, 30);
			break;
		case IOCTL_IOWAIT_TIME:
			copy_to_user((int __user *) arg,  buff_iowait, 30);
			break;
		case IOCTL_GUEST_TIME:
			copy_to_user((int __user *) arg,  buff_guest, 30);
			break;
		case IOCTL_GUEST_NICE_TIME:
			copy_to_user((int __user *) arg,  buff_nice, 30);
			break;
		case IOCTL_STEAL_TIME:
			copy_to_user((int __user *) arg,  buff_steal, 30);
			break;
		case IOCTL_SOFTIRQ_TIME:
			copy_to_user((int __user *) arg,  buff_uptime, 30);
			break;
		default:
			printk(KERN_ALERT "IOCTLPROCFS: Invalid IOCTL Command!\n");
			return -ENOTTY;
	}
	return retval;
}


//This function calls on demand of read request from seq_files
static int proc_show(struct seq_file *m, void *v){
	seq_printf(m, "IOCTL_UPTIME, %lu\n", IOCTL_UPTIME);
	seq_printf(m, "IOCTL_IRQ_TIME, %lu\n", IOCTL_IRQ_TIME);
	seq_printf(m, "IOCTL_JIFFIES, %lu\n", IOCTL_JIFFIES);
	seq_printf(m, "IOCTL_USER_TIME, %lu\n", IOCTL_USER_TIME);
	seq_printf(m, "IOCTL_SYSTEM_TIME, %lu\n", IOCTL_SYSTEM_TIME);
	seq_printf(m, "IOCTL_NICE_TIME, %lu\n", IOCTL_NICE_TIME);

	seq_printf(m, "IOCTL_IDLE_TIME, %lu\n", IOCTL_IDLE_TIME);
	seq_printf(m, "IOCTL_IOWAIT_TIME, %lu\n", IOCTL_IOWAIT_TIME);
	seq_printf(m, "IOCTL_GUEST_TIME, %lu\n", IOCTL_GUEST_TIME);
	seq_printf(m, "IOCTL_GUEST_NICE_TIME, %lu\n", IOCTL_GUEST_NICE_TIME);
	seq_printf(m, "IOCTL_STEAL_TIME, %lu\n", IOCTL_STEAL_TIME);
	seq_printf(m, "IOCTL_SOFTIRQ_TIME, %lu\n", IOCTL_SOFTIRQ_TIME);
	return SUCCESS;
}

//This is where system functionallity triggers every time some process try to read from our proc entry
static int proc_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "IOCTLPROCFS: Open Function, Process \"%s:%i\"\n", current->comm, current->pid);
	return single_open(file, proc_show, NULL);
}


//Struct file_operations is the key to the functionality of the module
//functions that defined here are going to add to the kernel functionallity
//in order to respond to userspace access demand to the corresponding /proc entry
static const struct file_operations fops = {
	.open = proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.unlocked_ioctl = proc_ioctl,
};


//Your module's entry point
static int __init capability_procfs_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "IOCTLPROCFS: Initialization.\n");
	printk(KERN_INFO "IOCTLPROCFS: Init Module, Process \"%s:%i\"\n", current->comm, current->pid);

	our_proc_file = proc_create(DEVICE_NAME, 0644 , NULL, &fops);
	//Put an error message in kernel log if cannot create proc entry
	if(!our_proc_file){
		printk(KERN_ALERT "IOCTLPROCFS: Registration Failure.\n");
		//Because of this fact that procfs is a ram filesystem, this error means the lack of enough memory
		return -ENOMEM;
	}

	printk(KERN_INFO "IOCTLPROCFS: /proc/%s has been created.\n", DEVICE_NAME);
	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit capability_procfs_exit(void){
	//Initiating module unloading procedure
	printk(KERN_INFO "IOCTLPROCFS: Cleanup Module, Process \"%s:%i\"\n", current->comm, current->pid);

	//Remove proc filesystem entry from system
	remove_proc_entry(DEVICE_NAME, NULL);
	printk(KERN_INFO "IOCTLPROCFS: /proc/%s has been removed.\n", DEVICE_NAME);

	printk(KERN_INFO "IOCTLPROCFS: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}

//Now we need to define init-module and cleanup_module aliases
module_init(capability_procfs_init);
module_exit(capability_procfs_exit);
