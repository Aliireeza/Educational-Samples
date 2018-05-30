//We need this header in all kernel modules
#include <linux/module.h>
//Absolutely because we are doing kernel job
#include <linux/kernel.h>
//And this is needed for the macros
#include <linux/init.h>
//For using interrupt functions and Tasklets
#include <linux/interrupt.h>
//For using I/O ports
#include <linux/ioport.h>
//Most of delay functions lie here
#include <linux/delay.h>
//For I/O read and write functions such as inb and outb_p
#include <asm/io.h>
//For using Kernel timers
#include <linux/timer.h>


//It is always good to have a meaningful constant as a return code
#define SUCCESS 0
//This will be our module name
#define DEVICE_NAME "parallelport"


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple module which using parallel port for educational purpose and creating a blinking LED");
MODULE_VERSION("1.0.3");


//On Intel based architecture
static int parallel_base = 0x378;
//Some global variables
static int parallel_irq = -1;
static int timer_expire_delay = 1000;
static unsigned int desire_output = 0;
//We have to use struct timer_list to creat our own timer
static struct timer_list our_timer;


//This function will call at each timer's timeout
void our_timer_function(struct timer_list *timer){
	//In Timer function we want to create a simple single byte output
	//which could light a LED in a row of 8 LEDs, each time it will do it for next one
	//at the end of the row (pin 9) it will generate an interrupt which make everything slower
	//after ten times, everyting reset to the initial speed
	if(desire_output >= 255 || desire_output == 0)
		desire_output = 1;
	else
		desire_output *= 2;
	outb(desire_output, parallel_base);
	wmb();
	printk(KERN_INFO "PARALLELLED: Port Output = %u\n", desire_output);
	//Now we just reschedule timer and get out
	mod_timer(&our_timer, jiffies + timer_expire_delay);
}


irqreturn_t blink_interrupt_handler(int irq, void *dev_id){
	//Here if we are producing a beautiful blinking LEDs on the output and then
	//an interrupt happens, we will just re-schedule our output timer to make it more slow
	if(timer_expire_delay <= 10000)
		timer_expire_delay += 1000;
	else
		timer_expire_delay = 1000;
	//Acknowledge the Kernel that everything is ok
	return IRQ_HANDLED;
}


void parallel_kernel_probing(void){
	int count = 0;
	do{
		unsigned long mask;

		mask = probe_irq_on();//Save the current interrupt state of the system
		outb_p(0x10,parallel_base + 2); //Enable interrupts on parrallel port
		outb_p(0x00,parallel_base); //Erase data bits
		udelay(5);
		outb_p(0xFF,parallel_base); //Toggle them, so we can capture an interrupt
		outb_p(0x00,parallel_base + 2); //Disable interrupts on parrallel port
		udelay(5);
		parallel_irq = probe_irq_off(mask);//Compare the ne state with the saved one, and show the result

		if(parallel_irq == 0){
			printk(KERN_INFO "PARALLELLED: No irq reported by probe, in %d try.\n", count);
			parallel_irq = -1;
			}
		//If more than one line has been activated, the result is
		//negative. We should service the interrupt (but the lpt port
		//doesn't need it) and loop over again. Do it at most 5 times
	}while(parallel_irq < 0 && count++ < 5);
	//If not find
	if(parallel_irq < 0)
		printk("PARALLELLED: Probe has failed %i times, giving up!\n", count);
}


//Your module's entry point
static int __init parallel_led_init(void){
	//These mesages will not show to the user but instead they will apear in kernel log
	//You can observe the kernel log in /dev/kmsg or with using dmsg command
	printk(KERN_INFO "PARALLELLED: Initialization ...\n");

	//First, we have to access I/O port of the parallel interface
	if(request_region(parallel_base, 4, DEVICE_NAME))
		printk(KERN_ALERT "PARALLELLED: Parallel ports has been registered to %s\n", DEVICE_NAME);
	else{
		printk(KERN_ALERT "PARALLELLED: Parallel Ports Registration Failure.\n");
		return -EFAULT;
	}

	//Now probe for the irq number by the Kernel help
	parallel_kernel_probing();

	//Then, if we have found the compatible irq line, then we will register it to the proper handler
	if(parallel_irq >= 0)
		if(request_irq(parallel_irq, blink_interrupt_handler, IRQF_NO_SUSPEND, DEVICE_NAME, NULL) == 0)
			printk(KERN_INFO "PARALLELLED: %s has registered %d interrupt line successfully.\n", DEVICE_NAME, parallel_irq);
		else{
			printk(KERN_INFO "PARALLELLED: %s could NOT registered %d interrupt line.\n", DEVICE_NAME, parallel_irq);
			release_region(parallel_base, 4);
			return -EFAULT;
			}
	else{
		printk(KERN_INFO "PARALLELLED: %s could NOT ATTEMPT to registered %d interrupt line.\n", DEVICE_NAME, parallel_irq);
		release_region(parallel_base, 4);
		return -EFAULT;
		}


	//It is now time to initiate our timer, but it will not start yet	
	our_timer.function = our_timer_function;
	our_timer.expires = jiffies + msecs_to_jiffies(timer_expire_delay);

	//now start the timer
	add_timer(&our_timer);
	printk(KERN_INFO "PARALLELLED: %s has initiated Timer successfully.\n", DEVICE_NAME);
		

	//The init_module should return a value to the rest of kernel that asure
	//them to its successfully registration of its functionality
	return SUCCESS;
}


//You sould clean up the mess before exiting the module
static void __exit parallel_led_exit(void){

	//Release the irq line, if it has been obtained previously
	if(parallel_irq >= 0)
		free_irq(parallel_irq, NULL);
	printk(KERN_INFO "PARALLELLED: %s has released IRQ %d interrupt.\n", DEVICE_NAME, parallel_irq);
	
	//Release the I/O ports region
	release_region(parallel_base, 4);
	printk(KERN_INFO "PARALLELLED: %s has released I/O ports region.\n", DEVICE_NAME);

	//Now we have to release the timer resources
	del_timer(&our_timer);
	printk(KERN_INFO "PARALLELLED: %s has finished and released Timer.\n", DEVICE_NAME);

	printk(KERN_INFO "PARALLELLED: GoodBye.\n");
	//The cleanup_module function doesn't need to return any value to the rest of the kernel
}


//Now we need to define init-module and cleanup_module aliases
module_init(parallel_led_init);
module_exit(parallel_led_exit);
