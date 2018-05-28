//These are our ioctl definition
#define MAGIC 'T'
#define IOC_MAXNR 18
#define IOCTL_PROCESS_NAME _IOR(MAGIC, 0, char)
#define IOCTL_PROCESS_PID _IOR(MAGIC, 1, char)

#define IOCTL_PROCESS_UID _IOR(MAGIC, 2, char)
#define IOCTL_PROCESS_EUID _IOR(MAGIC, 3, char)
#define IOCTL_PROCESS_SUID _IOR(MAGIC, 4, char)
#define IOCTL_PROCESS_FSUID _IOR(MAGIC, 5, char)

#define IOCTL_PROCESS_GID _IOR(MAGIC, 6, char)
#define IOCTL_PROCESS_EGID _IOR(MAGIC, 7, char)
#define IOCTL_PROCESS_SGID _IOR(MAGIC, 8, char)
#define IOCTL_PROCESS_FSGID _IOR(MAGIC, 9, char)

#define IOCTL_PROCESS_PGRP _IOR(MAGIC, 10, char)
#define IOCTL_PROCESS_PRIORITY _IOR(MAGIC, 11, char)
#define IOCTL_PROCESS_REAL_PRIORITY _IOR(MAGIC, 12, char)
#define IOCTL_PROCESS_PROCESSOR _IOR(MAGIC, 13, char)

#define IOCTL_PROCESS_UTIME _IOR(MAGIC, 14, char)
#define IOCTL_PROCESS_STIME _IOR(MAGIC, 15, char)
#define IOCTL_PROCESS_GTIME _IOR(MAGIC, 16, char)
#define IOCTL_PROCESS_STARTTIME _IOR(MAGIC, 17, char)
