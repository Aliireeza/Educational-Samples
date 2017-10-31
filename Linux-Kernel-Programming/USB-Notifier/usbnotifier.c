//We are going to do some kernel programming
#include <linux/kernel.h>
//Obviously we are creating a module
#include <linux/module.h>
//Just for using macros here
#include <linux/init.h>
//We want to play with USB devices
#include <linux/usb.h>
//For obtaining PID and process name
#include <linux/sched.h>


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple module based on Linux Kernel notifier chains to monitor usb device/bus activities");
MODULE_VERSION("1.0.2");


//This function will distinguish between various device classes
static void identify_device_class_type(__u8 device_class){
	switch(device_class){
		case USB_CLASS_AUDIO:
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: Audio Device Class\n");
			break;
		case USB_CLASS_COMM:
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: COMM Device Class\n");
			break;
		case USB_CLASS_HID:
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: HID Device Class\n");
			break;
		case USB_CLASS_PRINTER:
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: Printer Device Class\n");
			break;
		case USB_CLASS_HUB:
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: HUB Device Class\n");
			break;
		case USB_CLASS_VIDEO:
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: Video Device Class\n");
			break;
		case USB_CLASS_MASS_STORAGE:
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: Mass-Storage Device Class\n");
			break;
		case USB_CLASS_WIRELESS_CONTROLLER:
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: Wireless Device Class\n");
			break;
		default:
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: No Specified Device Class\n");
			break;
		}
}



static int usb_notify(struct notifier_block *self, unsigned long action, void *dev) {
	//Who called the notifier_block?
 	printk(KERN_INFO "SIMPLEUSBNOFIFIER: Notifier function, Process(%s:%i)\n", current->comm, current->pid);

	//Get usb_device struct from the passing data
	struct usb_device *usbdev = (struct usb_device *) dev;
	
	//Decide on different action
	switch(action){ 
		case USB_DEVICE_ADD: 
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: USB device (%04X:%04X) added.\n",  usbdev->descriptor.idVendor, usbdev->descriptor.idProduct);
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: Product(%s), Manufacturer(%s), Serial(%s)\n", usbdev->product, usbdev->manufacturer, usbdev->serial);
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: Class(%d), SubClass(%d), Protocol(%d)\n",  usbdev->descriptor.bDeviceClass,  usbdev->descriptor.bDeviceSubClass,  usbdev->descriptor.bDeviceProtocol);
			identify_device_class_type(usbdev->descriptor.bDeviceClass);
			break;
		case USB_DEVICE_REMOVE: 
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: USB device (%04X:%04X) removed.\n", usbdev->descriptor.idVendor, usbdev->descriptor.idProduct); 
			break; 
		case USB_BUS_ADD: 
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: USB Bus \"%s:%s:%s\" added with (%04X:%04X).\n", usbdev->product, usbdev->manufacturer, usbdev->serial, usbdev->descriptor.idVendor, usbdev->descriptor.idProduct); 
			break; 
		case USB_BUS_REMOVE: 
			printk(KERN_INFO "SIMPLEUSBNOFIFIER: USB Bus (%04X:%04X) removed.\n", usbdev->product, usbdev->manufacturer, usbdev->serial, usbdev->descriptor.idVendor, usbdev->descriptor.idProduct);
			break;
		default:
		        printk(KERN_INFO "SIMPLEUSBNOFIFIER: Something rather than USB/BUS ADD/REMOVE has occured to \"%s:%s:%s\" device with (%04X:%04X).\n", usbdev->product, usbdev->manufacturer, usbdev->serial, usbdev->descriptor.idVendor, usbdev->descriptor.idProduct);
		        return NOTIFY_DONE;
		} 
		
	return NOTIFY_OK; 
} 



//Using notifiers for identifying actions on usb
static struct notifier_block usb_nb = { 
	.notifier_call = usb_notify, //This fuction will call whenever the notifier triggers
};


//This would be our module start up point
static int __init usb_notifier_init(void){
	printk(KERN_INFO "SIMPLEUSBNOFIFIER: Init module, Process(%s:%i)\n", current->comm, current->pid);
	//Registering a Notifier
	usb_register_notify(&usb_nb);
	printk(KERN_INFO "SIMPLEUSBNOFIFIER: USB Notifier registered successfully.\n");

	return 0;
}


//Now, due to end this module, its time to clean up the mess
static void __exit usb_notifier_exit(void){
	printk(KERN_INFO "SIMPLEUSBNOFIFIER: Cleanup module, Process(%s:%i)\n", current->comm, current->pid);
	//Unregistering driver and notifier
	usb_unregister_notify(&usb_nb);

        printk(KERN_INFO "SIMPLEUSBNOFIFIER: USB Notifier module deregistered.\n");
}


//Definition of init_module and cleanup_modules with user-defined functions
module_init(usb_notifier_init);
module_exit(usb_notifier_exit);
