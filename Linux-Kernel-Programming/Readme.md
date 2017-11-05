# Linux Kernel Educational Samples

A repository for various types of Linux Kernel modules and driver which I use to teach in my classes at Sharif University and other places


### The Goal
Creating a vast variety of Linux Kernel module/driver programming samples, to be used in my classes and also could be used as self documentory sources for studying in this field

### Current Samples
Current samples each packed in a seperated folder with specific Readme and Makefile. they all have been tested on Linux Kernel 4.13.10 and above and I hope that they do not have any particular problems on your system, but if they have and you could not figure the issue, please send me an email (teymoorian [at] gmail [dot] com), but I know for sure that you will figure it out.<br>
All samples tried to be as minimalistic as possible and commented so be self-documented and I know that they might not be very useful when you consider them isolately, but I hope that they provide an up-to-date source-code-based documentation for the aggressively under developing Linux Kernel.

### Essential Needs
For running them you need some essentials of your GNU/Linux distributions
on Ubuntu
```
apt install libncurses5 build-essential linux-headers-`uname -r`
```
on Fedora
```
dnf install kernel-devel
```
Also this remains at the bottom line on Kernel development essential tools, you might find it useful to use some sophisticated code editor or IDEs as well like (link+)

### Resources (Not all of them)
- Linux Device Drivers, 3rd Edition, By Greg Kroah-Hartman, Alessandro Rubini & Jonathan Corbet, O'REILLY
- Linux kernel Development, 3rd Edition, By Robert Love,  Developer’s Library
- Professional Linux Kernel Architecture, By Wolfgang Mauerer, Wiley Publishing
- Understanding the Linux Kernel, 3rd Edition,  By Daniel P. Bovet, O’Reilly
- The Linux Programming Interface,  By Michael Kerrisk, No Strach Press
- Linux Kernel Networking Implementation, By Rami rosen, Apress
- Linux Kernel Module Programming Guide, By Peter Jay Salzman, Michael Burian & Ori Pomerantz
- Advanced Linux Progrmming Guide, By Mark Mitchell, Jeffrey Oldham & Alex Samuel, New Riders Publishing
- Modern Operating Systems, 4th Edition, By Andrew S. Tanenbaum & Herbert Bos,  Person Education Inc.
- Operating System Concepts, 9th Edition, Abraham Silberschatz & Peter Baer Galvin, Wiley Publishing


### Course Syllabus (For Now)
- Part One: An Introduction To Linux Kernel Programming (10 hours)
  - Linux Kernel programming Intro (2 hours)
    - History Behind Linux Kernel
    - Linux Kernel Versions
    - Monolithic Architecture Vs. Micro-Kernel
    - Kernel Modules Vs. User-Space Programs
    - Loadable Kernel Modules Vs. Drivers
    - Kernel Subsystems and Driver Types
    - Setting Up an Environment
    - Compiling a Fresh Kernel
  - Make Your First Loadable Kernel Module (2 hours)
    - Anatomy of a Loadable Kernel Module
    - Writing Your First Kernel Module
    - Using Kernel Logs To Communicate With User
    - Defining Command Line Arguments
    - Adding Pesudo Information
    - Building a Kernel Module with Makefiles
    - Installing, Removing and Reviewing a Module’s Info
    - How It Actually Works
  - Kernel Space - User Space Interfaces (4 hours)
    - Using Proc File System and Seq-Files 
    - Using Device Files, Major and Minor Numbers 
    - Communicating with User-Space Through Charachter Devices
    - Facilitate a Charachter Driver with IOCTL Commands
    - Checking User-Space Capability and Permission
    - Making Device Node Dynamically
    - Using Attributes of Sys File System with Kobjects
    - Putting All Toghether
  - Linux Kernel Programming Concepts (2 hours)
    - Debugging Techniques
    - Atomic Context Vs. Process Context
    - Kernel Data Structures
    - System Call Interface
    - Notifier Chains

- Part Two: Linux Kernel Programming ToolBox (32 hours)
  - Concurrency Handling (4 hours)
    - Semaphores and Mutexes
    - Spin-Locks and Completions
    - Atomic Variables and Lock-Free Algorithms
    - Bit Operations
  - Time, Delay, Deferred Works and Timers (5 hours)
    - Time-stamps and Jiffies
    - Long Delays Vs. Short Delays
    - Busy-Waiting Vs. Sleep Based Delay
    - Work-Queues and Deferred Works
    - Fantastic Kernel Timers
    - Blocking I/O
  - Interrupt Handling (6 hours)
    - Interrupts  Concepts and Vectors
    - Using hard Interrupts
    - Using Softirqs
    - Interrupt Handlers
    -  Slow and Fast Handlers, Top and Bottom Halves
    - Tasklets and Work-Queues
  - Memory Allocation (6 hours)
    - Kmalloc Function and Memory Allocation Concept
    - Linux Memory management Subsystem
    - Memory Pools Data Structure and Functions
    - Lookaside Chaches Data Structure and Functions
    - Kfifo Data Structure and Functions
    - Page Allocations and Obtaining Large Buffers
    - Per-CPU Variables
    - Virtual Addresses and Memory Remap
  - Process Management (4 hours)
    - Process, Threads and Kthreads Concepts
    - Systemd Vs. Initd
    - Process Data Structures
    - Check and Set Process States
    - Send Signals to a Process
    - Moving Around System Process Tree
    - Process Creation and Termination
    - Multi tasking, Parallelism and Processes
    - Process Scheduling Concepts and Linux CFS Scheduler
  - Communicating With Hardwares (4 Hours)
    - I/O Ports and I/O Memories
    - I/O Ports Registration
    - Data Transfer Via an I/O port
    - I/O Memory Registration
    - Data Transfer Via an I/O Memory
    - I/O Ports mapped On I/O Memory
  - Notifier Chains (3 hours)
    - Notifier Chains and Notifier Blocks
    - Keyboard Notifier
    - USB Notifier
    - Network Notifiers
    - Other Driver Notifiers


- Part Three:  Driver Programming Tutorials (32 hours)
  - Parallel Port Drivers (4 hours)
    - Parallel Port Structures and Concepts
    - Preparing a Parallel Port
    - Parallel Port Interrupt Line Probing
    - Parallel Port Interrupt Handler
    - Data Transfer
    - Creating a Parallel Port Driver
    - Parport Interface
    - Creating a Parport Driver
  - PCI Port Drivers (3 hours)
    - PCI Interface Concepts and Structure
    - PCI Adresses and Regions
    - PCI Registration and Probing
    - Accessing PCI Configuration Space
    - Accessing PCI I/O Port and I/O Memory
    - PCI Interrupts
    - Data Transfer
    - Creating a PCI Driver
  - USB Port Drivers (5 hours)
    - USB Ports and Devices Concepts
    - USB Drivers, Core Drivers and Host-Controllers
    - Using USB URBs
    - Using USB Notifiers
    - Probbing, Connect and Disconnect Operations
    - Data Transfer Without URBs
    - USB Data Structures and Data Functions
    - Creating an Adequate USB Driver
  - TTY Drivers (4 hours)
    - TTY Driver Data Structures
    - Buffering and Other Functions
    - Seting and Using Termios
    - Device Access
    - Creating a TTY Driver
    - Creating a Serial Port Driver
  - Block Drivers (2 hours)
    - Block Device Drivers Data Structures
    - Block Requests and Gen-Disks
    - Request Queues
    - DMA Access
    - Creating a Practical Block Driver
  - VFS Interface (4 hours)
    - Common Filesystem Layes
    - Filesystem Abstraction Layer
    - Unix Filesystems
    - VFS Objects and Data Structures
    - Super Block Object and Operations
    - Inode Object and Operations
    - Dentry Object and Operations
    - File Object and Operations
    - Creating a Simple Filesystem
  - Network Drivers (2 hours)
    - Network Addresses, Protocols and Interfaces
    - Network Stack and Its Data Structures
    - Packet Transfer and Acceptance
    - Concurrency, Scatter/Gather I/O, Timeouts and Interrupts
    - Link States, Socket Buffers, Data-Linke Layer
    - Creating a Basic Network Driver
  - Net-Filter Hooks (4 hours)
    - Iptables and Net-Filter
    - Hook Chaines, Priority and Position
    - Protocols, Addresses and Interface Filtering
    - Intercepting a Packet Content
    - Creating a Rule-Based Firewall
  - Miscellaneous Drivers (4 hours)
    - Platform Driver Interface
    - Misc Driver Interface
    - System Call Interface
    - System Call Hooks
    - Power Management Interface
