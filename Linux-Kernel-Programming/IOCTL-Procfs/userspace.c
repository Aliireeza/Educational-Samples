#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(){
	FILE *fp_proc, *fp_dev;
	char buffer[1024], output[30];
	size_t bytes_read;
	char *match;
	int i;
	unsigned long ioctlcmd;
	
	fp_proc = fopen("/proc/information", "r");
	if(!fp_proc){
		printf("/proc/information is not available\n");
		return 0;
		}
	bytes_read = fread(buffer, 1, sizeof(buffer), fp_proc);
	fclose(fp_proc);

	if(bytes_read == 0 || bytes_read == sizeof(buffer)){
		printf("Some unexpected error in read process\n");
		return 0;
		}

	printf("Enter a Number Between 1 to 6:\n");
	printf("1:SysName\t2:NodeName\n3:Release\t4:Version\n5:Machine\t6:DomainName\n");
	printf("Your Choice: ");
	scanf("%d", &i);

	switch(i){
		case 1:
			match = strstr(buffer, "IOCTL_SYSNAME");
			if(!match){
				printf("IOCTL_SYSNAME has not found\n");
				return 0;
				}

			sscanf(match, "IOCTL_SYSNAME, %lu", &ioctlcmd);
			break;
		case 2:
			match = strstr(buffer, "IOCTL_NODENAME");
			if(!match){
				printf("IOCTL_NODENAME has not found\n");
				return 0;
				}

			sscanf(match, "IOCTL_NODENAME, %lu", &ioctlcmd);
			break;
		case 3:
			match = strstr(buffer, "IOCTL_RELEASE");
			if(!match){
				printf("IOCTL_RELEASE has not found\n");
				return 0;
				}

			sscanf(match, "IOCTL_RELEASE, %lu", &ioctlcmd);
			break;
		case 4:
			match = strstr(buffer, "IOCTL_VERSION");
			if(!match){
				printf("IOCTL_VERSION has not found\n");
				return 0;
				}

			sscanf(match, "IOCTL_VERSION, %lu", &ioctlcmd);
			break;
		case 5:
			match = strstr(buffer, "IOCTL_MACHINE");
			if(!match){
				printf("IOCTL_MACHINE has not found\n");
				return 0;
				}

			sscanf(match, "IOCTL_MACHINE, %lu", &ioctlcmd);
			break;
		case 6:
			match = strstr(buffer, "IOCTL_DOMAINNAME");
			if(!match){
				printf("IOCTL_DOMAINNAME has not found\n");
				return 0;
				}

			sscanf(match, "IOCTL_DOMAINNAME, %lu", &ioctlcmd);
			break;
		default:
			printf("Invalid Command\n");
			return -1;
	}

	int fd = open("/proc/information", O_RDONLY);
	ioctl(fd, ioctlcmd, output);
	close(fd);

	printf("Output: %s\n", output);
	return 0;
}
