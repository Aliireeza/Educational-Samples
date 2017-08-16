#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "commonioctlcommands.h"

int main(){
	char output[30];

	printf("--------   Procfs-IOCTL Process Information   --------\n");
	//First, we have to open a file (local) socket to the proc file again
	int fd = open("/proc/selfprocess", O_RDONLY);
	if(fd > 0 ){
		//Second, we have to send the correct ioctl commands
		//Process information
		ioctl(fd, IOCTL_PROCESS_NAME, output);
		printf("Name: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_PID, output);
		printf("PID: %s\n", output);

		//User information
		ioctl(fd, IOCTL_PROCESS_UID, output);
		printf("UID: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_EUID, output);
		printf("EUID: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_SUID, output);
		printf("SUID: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_FSUID, output);
		printf("FSUID: %s\n", output);


		//Group information
		ioctl(fd, IOCTL_PROCESS_GID, output);
		printf("GID: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_EGID, output);
		printf("EGID: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_SGID, output);
		printf("SGID: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_FSGID, output);
		printf("FSGID: %s\n", output);


		//Pririty information
		ioctl(fd, IOCTL_PROCESS_PGRP, output);
		printf("PGRP: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_PRIORITY, output);
		printf("Priority [Nice #]: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_REAL_PRIORITY, output);
		printf("Real Priority: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_PROCESSOR, output);
		printf("Processor: %s\n", output);


		//Time Information
		ioctl(fd, IOCTL_PROCESS_UTIME, output);
		printf("UTime: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_STIME, output);
		printf("STime: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_GTIME, output);
		printf("GTime: %s\n", output);

		ioctl(fd, IOCTL_PROCESS_STARTTIME, output);
		printf("Start Time: %s\n", output);
	}
	//Third, we sent the obtained IOCTL command value to the driver through our socket so we can terminate the socket
	close(fd);
	printf("------------------------------------------------------\n");
	return 0;
}
