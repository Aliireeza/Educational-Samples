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


//This is our driver name
#define DEVICE_NAME "simpleusbdriver"


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple module that register a usb_driver inorder to probe incomping and out going devices, without offering any functionality to them");
MODULE_VERSION("1.0.2");


//We need a struct to hold and manipulate USB device
static struct usb_device *our_device;
static struct usb_class_driver class;


//We need to stopped usb-storage, in order to using our own probe function when we connect a USB storage device
static int testusb_probe(struct usb_interface *interface, const struct usb_device_id *id){
	//Who called the probe?
 	printk(KERN_INFO "SIMPLEUSBDRIVER: PROBE function, Process(%s:%i)\n", current->comm, current->pid);
	//Catch the deive VendorID:ProductID
	printk(KERN_INFO "SIMPLEUSBDRIVER: My usb drive (%04X:%04X) plugged in.\n", id->idVendor, id->idProduct);
	//Catch the device type, mouse, keyboard, storage, ...
    	printk(KERN_INFO "SIMPLEUSBDRIVER: My probe called for %s device.\n",(char *)id->driver_info);
    	return 0;
}


//When a device disconnect form a computer, this function calls
static void testusb_disconnect(struct usb_interface *interface){
	printk(KERN_INFO "SIMPLEUSBDRIVER: usb interface %d now disconnected\n", interface->cur_altsetting->desc.bInterfaceNumber);
}


//List the USB devices which our driver will work with them described in two ways
static struct usb_device_id usb_table[] = {
	//First, Description of USB devices with their interrface information
	//The three numbers are class, subclass, protocol.
	{ USB_INTERFACE_INFO(3, 1, 1), driver_info: (unsigned long)"keyboard" },
	{ USB_INTERFACE_INFO(3, 1, 2), driver_info: (unsigned long)"mouse" },

	//Second, Description of USB devices with VendorID:ProductID
	//Obtain it by lsusb command from bash
	{ USB_DEVICE(0x8564, 0x1000), driver_info: (unsigned long)"storage" }, //My Trancend 8GB JetFlash
	{ USB_DEVICE(0x13fe, 0x13fe), driver_info: (unsigned long)"storage" }, //My Kingston 32GB Cooldisk
	{ USB_DEVICE(0x03f0, 0x5a07), driver_info: (unsigned long)"storage" }, //My Hewlet-Packard 8GB Cooldisk
	{ 0 },
	};


//Registering USB device table to the driver (Type like usb or pci, TableName)
MODULE_DEVICE_TABLE(usb, usb_table);


//We need a struct to manipulate USB devices
static struct usb_driver testusb_driver = {
	.name = DEVICE_NAME, //Our USB driver name
	.id_table = usb_table, //Register USB table of the module to the driver
	.probe = testusb_probe, //Our probe function which will work on device when it connects to the computer
	.disconnect = testusb_disconnect, //On disconnecting a USB device from Computer this fuction will call
};


//This would be our module start up point
static int __init testusb_init(void){
	int result;

	//Registering USB deriver to the structure
	result = usb_register(&testusb_driver);
	if(result){
		printk(KERN_INFO "SIMPLEUSBDRIVER: Registering driver failed.\n");
		return result;
	}
	printk(KERN_INFO "SIMPLEUSBDRIVER: Driver registered successfully.\n");

	return result;
}


//Now, due to end this module, its time to clean up the mess
static void __exit testusb_exit(void){
	//Unregistering driver
	usb_deregister(&testusb_driver);

        printk(KERN_INFO "SIMPLEUSBDRIVER: Module deregistered.\n");
}


//Definition of init_module and cleanup_modules with user-defined functions
module_init(testusb_init);
module_exit(testusb_exit);
