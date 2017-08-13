#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(){
  FILE *fp_proc, *fp_dev;
  char buffer[1024];
  size_t bytes_read;
  char *match;
  unsigned long ioctlcmd;
  fp_proc = fopen("/proc/advancedchardev", "r");
  if(!fp_proc){
    printf("/proc/advancedchardev is not available\n");
    return 0;
  }
  bytes_read = fread(buffer, 1, sizeof(buffer), fp_proc);
  fclose(fp_proc);

  if(bytes_read == 0 || bytes_read == sizeof(buffer)){
    printf("Some unexpected error in read process\n");
    return 0;
  }

  match = strstr(buffer, "ADVANCED_CHARDEV_IOCTL_ENCRYPT");
  if(!match){
    printf("ADVANCED_CHARDEV_IOCTL_ENCRYPT has not found\n");
    return 0;
  }

  sscanf(match, "ADVANCED_CHARDEV_IOCTL_ENCRYPT, %lu", &ioctlcmd);
  printf("ioctl command: %lu\n", ioctlcmd);

  int fd = open("/dev/advancedchardev", O_RDONLY);
  ioctl(fd, ioctlcmd);
  close(fd);

  for(int i=0; i<sizeof(buffer); buffer[i++]=NULL);

  fp_dev = fopen("/dev/advancedchardev", "r");
  if(!fp_dev){
    printf("/dev/advancedchardev is not available\n");
    return 0;
  }

  bytes_read = fread(buffer, 1, sizeof(buffer), fp_dev);
  fclose(fp_dev);

  printf("Encrypted Text: %s", buffer);
  return 0;
}

