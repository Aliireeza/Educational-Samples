#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "commonioctlcommands.h"

//This function uses /proc/parallelled to send ioctl command for ones that not
//have any data arguments, AKA control ioctl commands
void send_control_ioctlcmd(unsigned long ioctlcmd){
	int fd;
	//Open a file socket to /proc/parallelled
	fd = open("/proc/parallelled", O_RDONLY);
	if(!fd){
		printf("/proc/parallelled is not available\n");
		exit(-1);
		}
	//Send the ioctl command without any data
	ioctl(fd, ioctlcmd);
	//Close the file socket
	close(fd);	
}

//This function uses /proc/parallelled to send ioctl command and has a 
//buffer argument that could transfer the result by refrerence to the main
void send_read_write_ioctlcmd(unsigned long ioctlcmd, char *buffer){
	int fd;
	//Open a file socket to /proc/parallelled
	fd = open("/proc/parallelled", O_RDONLY);
	if(!fd){
		printf("/proc/parallelled is not available\n");
		exit(-1);
		}
	//Send the ioctl command and send/recieve the data
	ioctl(fd, ioctlcmd, buffer);
	//Close the file socket
	close(fd);
}

//This function reads the /proc/parallelled and print out it's entire content
void read_proc_file(){
	FILE *fp_proc;
	size_t bytes_read;
	char buffer[2048];
	//Open a file descriptor for read
	fp_proc = fopen("/proc/parallelled", "r");
	if(!fp_proc){
		printf("/proc/parallelled is not available\n");
		exit(-1);
		}
	
	//Read the entire file's content
	bytes_read = fread(buffer, 1, sizeof(buffer), fp_proc);
	//Close the file descriptor
	fclose(fp_proc);

	//If any content has read we could print them, unless just print an error
	if(bytes_read == 0 || bytes_read == sizeof(buffer)){
		printf("Some unexpected error in read process\n");
		exit(-1);
		}
	
	printf("Port Status:\n%s\n", buffer);
}


//This application will show a menu to work with Parallel LED Driver
int main(){
	char buffer[30];
	int choice, mode;
	int delay, step;
	
	//Show the main menu
	printf("1:Reset\t\t2:Toggle\t3:Status\t4:Quit\n");
	printf("5:Delay\t\t6:Step\t\t7:Mode\t\t8:Value\n");
	printf("9:Port\t\t10:IRQ\t\t11:Probe\t12:Name\n");
	printf("13:Speed Up\t14:Speed Down\t15:Accelerate\t16:Deccelerate\n");
	printf("17:Single Blink\t18:Whole Blink\t19:Counter\t20:Shuffle\n");
	//Get user's choice
	printf("\nWhat is your choice [1 - 20]: ");
	scanf("%d", &choice);

	//Decide on which choice the user has made
	switch(choice){
		//This secret command will show the credit information as below
		case 0:
			printf("\n\n********  Multi Functional LED Driver on Parallel Port  *******\n");
			printf("Created By: Aliireeza Teymoorian, Spring 2018, Under GPL v3.0\n");
			printf("Contact: teymoorian@gmail.com\thttps://aliireeza.github.io\n");
			printf("This application has compiled by GNU/GCC 7.3 in Ubuntu 18.04\n");
			printf("The related Kernel driver is compatible with Kernel 4.16\n");
			printf("***************************************************************\n");
			break;
			
		//In these two cases we send control commands to reset the port or alter timer status
		case 1:
			send_control_ioctlcmd(IOCTL_PARALLEL_RESET);
			break;
		case 2:
			send_control_ioctlcmd(IOCTL_TIMER_TOGGLE);
			break;
			
		//In this case we just want to read the content of the proc file
		case 3:
			read_proc_file();
			break;
			
		//Quit the program, rememer to unload the module
		case 4:
			exit(0);
			break;
		
		//Here we are going to read driver's settings by ioctl commands with buffer aruments
		case 5:
			send_read_write_ioctlcmd(IOCTL_DELAY_READ, buffer);
			printf("Current delay of parallel port is %d ms\n", atoi(buffer));
			break;
		case 6:
			send_read_write_ioctlcmd(IOCTL_STEP_READ, buffer);
			printf("Delay change of parallel port is %d ms\n", atoi(buffer));
			break;
		case 7:
			send_read_write_ioctlcmd(IOCTL_MODE_READ, buffer);
			mode = atoi(buffer);
			//Decide on the output message depend on mode value
			if(mode == 0)
				printf("Parallel port works in Single Blinking LED mode [%d]\n", mode);
			else if (mode == 1)
				printf("Parallel port works in Whole Blinking LEDs mode [%d]\n", mode);
			else
				printf("Parallel port works in COUNTER mode [%d]\n", mode);
			break;
		case 8:
			send_read_write_ioctlcmd(IOCTL_VALUE_READ, buffer);
			printf("Current value on parallel port is %d\n", atoi(buffer));
			break;
		case 9:
			send_read_write_ioctlcmd(IOCTL_PORT_READ, buffer);
			printf("Parallel port address is %X\n", atoi(buffer));
			break;
		case 10:
			send_read_write_ioctlcmd(IOCTL_IRQ_READ, buffer);
			printf("Registered IRQ number is %d\n", atoi(buffer));
			break;
		case 11:
			send_read_write_ioctlcmd(IOCTL_PROBE_READ, buffer);
			mode = atoi(buffer);
			if(mode == 0)
				printf("Parallel driver uses Kernel Assissted probing\n");
			else
				printf("Parallel driver uses Do-It-Yourself probing\n");
			break;
		case 12:
			//TODO: Using /proc/interrupt to obtain the driver name
			break;
			
		//Here we are using ioctl commands with arguments to change the value of some settings
		case 13:
			send_read_write_ioctlcmd(IOCTL_DELAY_READ, buffer);
			delay = atoi(buffer);
			delay > 100 ? (delay -= 100): (delay = 10000);
			sprintf(buffer, "%d", delay);
			send_read_write_ioctlcmd(IOCTL_DELAY_WRITE, buffer);
			break;
		case 14:
			send_read_write_ioctlcmd(IOCTL_DELAY_READ, buffer);
			delay = atoi(buffer);
			delay < 10000 ?	(delay += 100): (delay = 100);
			sprintf(buffer, "%d", delay);
			send_read_write_ioctlcmd(IOCTL_DELAY_WRITE, buffer);
			break;
			break;
		case 15:
			send_read_write_ioctlcmd(IOCTL_STEP_READ, buffer);
			step = atoi(buffer);
			step > 100 ? (step -= 100): (step = 1000);
			sprintf(buffer, "%d", step);
			send_read_write_ioctlcmd(IOCTL_STEP_WRITE, buffer);
			break;
		case 16:
			send_read_write_ioctlcmd(IOCTL_STEP_READ, buffer);
			step = atoi(buffer);
			step < 1000 ? (step += 100): (step = 100);
			sprintf(buffer, "%d", step);
			send_read_write_ioctlcmd(IOCTL_STEP_WRITE, buffer);
			break;
		case 17:
			mode = 0;
			sprintf(buffer, "%d", mode);
			send_read_write_ioctlcmd(IOCTL_MODE_WRITE, buffer);
			break;
		case 18:
			mode = 1;
			sprintf(buffer, "%d", mode);
			send_read_write_ioctlcmd(IOCTL_MODE_WRITE, buffer);
			break;
		case 19:
			mode = 2;
			sprintf(buffer, "%d", mode);
			send_read_write_ioctlcmd(IOCTL_MODE_WRITE, buffer);
			break;
		case 20:
			mode = rand() % 3;
			sprintf(buffer, "%d", mode);
			send_read_write_ioctlcmd(IOCTL_MODE_WRITE, buffer);
			if(mode == 0)
				printf("Parallel port works in Single Blinking LED mode [%d]\n", mode);
			else if (mode == 1)
				printf("Parallel port works in Whole Blinking LEDs mode [%d]\n", mode);
			else
				printf("Parallel port works in COUNTER mode [%d]\n", mode);
			break;
		//Response to the user's wrong input
		default:
			printf("Wrong Input: You must select a number between 1 to 20\n");
		}
	return 0;
}
