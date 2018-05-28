#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(){
	FILE *fp_proc, *fp_dev;
	char buffer[2048], output[30];
	size_t bytes_read;
	char *match;
	int i;
	unsigned long ioctlcmd;

	//These are our IOCTL command names
	const char *ioctl_key[12]={"IOCTL_UPTIME", "IOCTL_IRQ_TIME", "IOCTL_JIFFIES", "IOCTL_USER_TIME", "IOCTL_SYSTEM_TIME", "IOCTL_NICE_TIME", "IOCTL_IDLE_TIME", "IOCTL_IOWAIT_TIME", "IOCTL_GUEST_TIME", "IOCTL_GUEST_NICE_TIME", "IOCTL_STEAL_TIME", "IOCTL_SOFTIRQ_TIME"};

	//First, Open the proc file to read the content of it
	fp_proc = fopen("/proc/systemtimes", "r");
	if(!fp_proc){
		printf("/proc/systemtimes is not available\n");
		return 0;
		}

	//Second, By fread, we will obtain the whole contnet which is tuples of IOCTL_SYSNAME//commands name and value
	bytes_read = fread(buffer, 1, sizeof(buffer), fp_proc);
	fclose(fp_proc);

	if(bytes_read == 0 || bytes_read == sizeof(buffer)){
		printf("Some unexpected error in read process\n");
		return 0;
		}

	//Third, we have to show the user menu
	printf("--------   Procfs-IOCTL System Times   --------\n");
	printf("Enter a Number Between 1 to 12:\n");
	printf("1:Uptime\t2:IRQ Time\n");
	printf("3:Jiffies\t4:User Time\n");
	printf("5:System Time\t6:Nice Time\n");
	printf("7:Idle Time\t8:IOWait Time\n");
	printf("9:Guest Time\t10:Guest Nice Time\n");
	printf("11:Steal Time\t12:Softirq Time\n");
	printf("Your Choice: ");
	scanf("%d", &i);
	printf("-----------------------------------------------\n");


	//Fourth, we have to read the correct line of the content
	//using sscanf to obtain the value of user selected IOCTL command
	if(i>=1 && i<=12){
		match = strstr(buffer, ioctl_key[i-1]);
		if(!match){
			printf("%s has not found\n", ioctl_key[i-1]);
			return -1;
			}
		switch(i){
			case 1:
				sscanf(match, "IOCTL_UPTIME, %lu", &ioctlcmd);
				break;
			case 2:
				sscanf(match, "IOCTL_IRQ_TIME, %lu", &ioctlcmd);
				break;
			case 3:
				sscanf(match, "IOCTL_JIFFIES, %lu", &ioctlcmd);
				break;
			case 4:
				sscanf(match, "IOCTL_USER_TIME, %lu", &ioctlcmd);
				break;
			case 5:
				sscanf(match, "IOCTL_SYSTEM_TIME, %lu", &ioctlcmd);
				break;
			case 6:
				sscanf(match, "IOCTL_NICE_TIME, %lu", &ioctlcmd);
				break;
			case 7:
				sscanf(match, "IOCTL_IDLE_TIME, %lu", &ioctlcmd);
				break;
			case 8:
				sscanf(match, "IOCTL_IOWAIT_TIME, %lu", &ioctlcmd);
				break;
			case 9:
				sscanf(match, "IOCTL_GUEST_TIME, %lu", &ioctlcmd);
				break;
			case 10:
				sscanf(match, "IOCTL_GUEST_NICE_TIME, %lu", &ioctlcmd);
				break;
			case 11:
				sscanf(match, "IOCTL_STEAL_TIME, %lu", &ioctlcmd);
				break;
			default:
				sscanf(match, "IOCTL_SOFTIRQ_TIME, %lu", &ioctlcmd);
		}
	}
	else{
		printf("Invalid Command\n");
		return -1;
	}

	//Fifth, we have to open a file (local) socket to the proc file again
	int fd = open("/proc/systemtimes", O_RDONLY);
	//Sixth, we send the obtained IOCTL command value to the driver through our socket
	ioctl(fd, ioctlcmd, output);
	close(fd);

	//Finally, we can print out the result of IOCTL command execution
	printf("Output: %s\n", output);
	return 0;
}
