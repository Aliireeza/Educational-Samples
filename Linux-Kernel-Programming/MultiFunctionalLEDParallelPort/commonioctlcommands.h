#define		MAGIC		'T'
#define		IOC_MAXNR	12

#define		IOCTL_PARALLEL_RESET		_IO(MAGIC, 0)
#define		IOCTL_TIMER_TOGGLE		_IO(MAGIC, 1)

#define		IOCTL_PROBE_READ		_IOR(MAGIC, 2, char)
#define		IOCTL_DELAY_READ		_IOR(MAGIC, 3, char)
#define		IOCTL_STEP_READ			_IOR(MAGIC, 4, char)
#define		IOCTL_MODE_READ			_IOR(MAGIC, 5, char)
#define		IOCTL_VALUE_READ		_IOR(MAGIC, 6, char)
#define		IOCTL_IRQ_READ			_IOR(MAGIC, 7, char)
#define		IOCTL_PORT_READ			_IOR(MAGIC, 8, char)

#define		IOCTL_DELAY_WRITE		_IOW(MAGIC, 9, char)
#define		IOCTL_STEP_WRITE		_IOW(MAGIC, 10, char)
#define		IOCTL_MODE_WRITE		_IOW(MAGIC, 11, char)
