#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include "commonioctlcommands.h"

void parallel_mode_detector(int mode){
	switch(mode){
		case 0:
			printf("Parallel LED driver works in Single LED Blinking mode.\n");
			break;
		case 1:
			printf("Parallel LED driver works in Whole LED Blinking mode.\n");
			break;
		default:
			printf("Parallel LED driver works in COUNTER LED mode.\n");	
	}
}

//This is a simple program to test the IOCTL function of Parallel LED Kernel Module
int main(){
	char output[30];
	int i = 2;
	printf("---------   Procfs-IOCTL Parallel Port LED   ---------\n");
	
	//First, we have to open a file (local) socket to the proc file
	//associated to uour driver
	int fd = open("/proc/parallelled", O_RDONLY);
	if(fd > 0){
		//Second, we have to send the correct ioctl commands
		
		ioctl(fd, IOCTL_MODE_READ, output);
		parallel_mode_detector(atoi(output));
		
		//Set the blinking mode to counter
		sprintf(output, "%d\n", i);
		ioctl(fd, IOCTL_MODE_WRITE, output);
		printf("CHANGE/SET MODE: %d\n", atoi(output));

		//Read the value associated with blinking mode command
		ioctl(fd, IOCTL_MODE_READ, output);
		parallel_mode_detector(atoi(output));
	}
	//In case that the proc file not exists or not accessible
	else
		printf("ERROR: Could not access to the /proc/parallelled\n");
	printf("------------------------------------------------------\n");
	//Third, we sent the obtained IOCTL command value to the driver
	// through our socket so we can terminate the socket
	close(fd);
	
	return 0;
}
