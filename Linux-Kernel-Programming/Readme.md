# Linux Kernel Educational Samples

A repository for various types of Linux Kernel modules and driver which I use to teach in my classes at Sharif University and other places


### The Goal
This module will filter all incoming and outgoing network traffic of your system on a single port, then if it received a specific ICMP packet it will open it for the sender IP address and by receiving the second magic packet it closes it again. This will be perform by registering two netfilter hooks.

### Current Samples


### Essential Needs


### Resources (Not all of them)
1. Linux Device Drivers, 3rd Edition, By Greg Kroah-Hartman, Alessandro Rubini & Jonathan Corbet, O'REILLY
2. Linux kernel Development, 3rd Edition, By Robert Love,  Developer’s Library
3. Professional Linux Kernel Architecture, By Wolfgang Mauerer, Wiley Publishing
4. Understanding the Linux Kernel, 3rd Edition,  By Daniel P. Bovet, O’Reilly
5. The Linux Programming Interface,  By Michael Kerrisk, No Strach Press
6. Linux Kernel Networking Implementation, By Rami rosen, Apress
7. Linux Kernel Module Programming Guide, By Peter Jay Salzman, Michael Burian & Ori Pomerantz
8. Advanced Linux Progrmming Guide, By Mark Mitchell, Jeffrey Oldham & Alex Samuel, New Riders Publishing
9. Modern Operating Systems, 4th Edition, By Andrew S. Tanenbaum & Herbert Bos,  Person Education Inc.
10. Operating System Concepts, 9th Edition, Abraham Silberschatz & Peter Baer Galvin, Wiley Publishing


### Course Syllabus (For Now)
1000. Part One: An Introduction To Linux Kernel Programming (10 hours)
      i. Linux Kernel programming Intro (2 hours)
           ii. History Behind Linux Kernel
           ii. Linux Kernel Versions
           ii. Monolithic Architecture Vs. Micro-Kernel
           ii. Kernel Modules Vs. User-Space Programs
           ii. Loadable Kernel Modules Vs. Drivers
           ii. Kernel Subsystems and Driver Types
           ii. Setting Up an Environment
           ii. Compiling a Fresh Kernel
	i. Make Your First Loadable Kernel Module (2 hours)
           1.2.1. Anatomy of a Loadable Kernel Module
           1.2.2. Writing Your First Kernel Module
           1.2.3. Using Kernel Logs To Communicate With User
           1.2.4. Defining Command Line Arguments
           1.2.5. Adding Pesudo Information
           1.2.6. Building a Kernel Module with Makefiles
           1.2.7. Installing, Removing and Reviewing a Module’s Info
           1.2.8. How It Actually Works
      i. Kernel Space - User Space Interfaces (4 hours)
           1.3.1. Using Proc File System and Seq-Files 
           1.3.2. Using Device Files, Major and Minor Numbers 
           1.3.3. Communicating with User-Space Through Charachter Devices
           1.3.4. Facilitate a Charachter Driver with IOCTL Commands
           1.3.5. Checking User-Space Capability and Permission
           1.3.6. Making Device Node Dynamically
           1.3.7. Using Attributes of Sys File System with Kobjects
           1.3.8. Putting All Toghether
      1.4. Linux Kernel Programming Concepts (2 hours)
           1.4.1. Debugging Techniques
           1.4.2. Atomic Context Vs. Process Context
           1.4.3. Kernel Data Structures
           1.4.4. System Call Interface
           1.4.5. Notifier Chains

2. Part Two: Linux Kernel Programming ToolBox (32 hours)
      2.1. Concurrency Handling (4 hours)
           2.1.1. Semaphores and Mutexes
           2.1.2. Spin-Locks and Completions
           2.1.3. Atomic Variables and Lock-Free Algorithms
           2.1.4. Bit Operations
      2.2. Time, Delay, Deferred Works and Timers (5 hours)
           2.2.1. Time-stamps and Jiffies
           2.2.2. Long Delays Vs. Short Delays
           2.2.3. Busy-Waiting Vs. Sleep Based Delay
           2.2.4. Work-Queues and Deferred Works
           2.2.5. Fantastic Kernel Timers
           2.2.6. Blocking I/O
      2.3.  Interrupt Handling (6 hours)
           2.3.1. Interrupts  Concepts and Vectors
           2.3.2. Using hard Interrupts
           2.3.3. Using Softirqs
           2.3.4. Interrupt Handlers
           2.3.5.  Slow and Fast Handlers, Top and Bottom Halves
           2.3.6. Tasklets and Work-Queues
      2.4. Memory Allocation (6 hours)
           2.4.1. Kmalloc Function and Memory Allocation Concept
           2.4.2. Linux Memory management Subsystem
           2.4.3. Memory Pools Data Structure and Functions
           2.4.4. Lookaside Chaches Data Structure and Functions
           2.4.5 Kfifo Data Structure and Functions
           2.4.6. Page Allocations and Obtaining Large Buffers
           2.4.7. Per-CPU Variables
           2.4.8. Virtual Addresses and Memory Remap
      2.5. Process Management (4 hours)
           2.5.1. Process, Threads and Kthreads Concepts
           2.5.2. Systemd Vs. Initd
           2.5.3. Process Data Structures
           2.5.6. Check and Set Process States
           2.5.7. Send Signals to a Process
           2.5.8. Moving Around System Process Tree
           2.5.8. Process Creation and Termination
           2.5.9. Multi tasking, Parallelism and Processes
           2.5.10. Process Scheduling Concepts and Linux CFS Scheduler
      2.6. Communicating With Hardwares (4 Hours)
           2.6.1. I/O Ports and I/O Memories
           2.6.2. I/O Ports Registration
           2.6.3. Data Transfer Via an I/O port
           2.6.4. I/O Memory Registration
           2.6.5. Data Transfer Via an I/O Memory
           2.6.6. I/O Ports mapped On I/O Memory
      2.7. Notifier Chains (3 hours)
           2.7.1. Notifier Chains and Notifier Blocks
           2.7.2. Keyboard Notifier
           2.7.3. USB Notifier
           2.7.4. Network Notifiers
           2.7.5. Other Driver Notifiers


3. Part Three:  Driver Programming Tutorials (32 hours)
      3.1. Parallel Port Drivers (4 hours)
           3.1.1. Parallel Port Structures and Concepts
           3.1.2. Preparing a Parallel Port
           3.1.3. Parallel Port Interrupt Line Probing
           3.1.4. Parallel Port Interrupt Handler
           3.1.5. Data Transfer
           3.1.6. Creating a Parallel Port Driver
           3.1.7. Parport Interface
           3.1.8. Creating a Parport Driver
      3.2. PCI Port Drivers (3 hours)
           3.2.1. PCI Interface Concepts and Structure
           3.2.2. PCI Adresses and Regions
           3.2.3. PCI Registration and Probing
           3.2.4. Accessing PCI Configuration Space
           3.2.5. Accessing PCI I/O Port and I/O Memory
           3.2.6. PCI Interrupts
           3.2.7. Data Transfer
           3.2.8. Creating a PCI Driver
      3.3. USB Port Drivers (5 hours)
           3.3.1. USB Ports and Devices Concepts
           3.3.2. USB Drivers, Core Drivers and Host-Controllers
           3.3.3. Using USB URBs
           3.3.4. Using USB Notifiers
           3.3.5. Probbing, Connect and Disconnect Operations
           3.3.6. Data Transfer Without URBs
           3.3.7. USB Data Structures and Data Functions
           3.3.8. Creating an Adequate USB Driver
      3.4. TTY Drivers (4 hours)
           3.4.1. TTY Driver Data Structures
           3.4.2. Buffering and Other Functions
           3.4.3. Seting and Using Termios
           3.4.4. Device Access
           3.4.5. Creating a TTY Driver
           3.4.6. Creating a Serial Port Driver
      3.5. Block Drivers (2 hours)
           3.5.1. Block Device Drivers Data Structures
           3.5.2. Block Requests and Gen-Disks
           3.5.3. Request Queues
           3.5.4. DMA Access
           3.5.5. Creating a Practical Block Driver
      3.6. VFS Interface (4 hours)
           3.6.2. Common Filesystem Layes
           3.6.2. Filesystem Abstraction Layer
           3.6.3. Unix Filesystems
           3.6.4. VFS Objects and Data Structures
           3.6.5. Super Block Object and Operations
           3.6.6. Inode Object and Operations
           3.6.7. Dentry Object and Operations
           3.6.8. File Object and Operations
           3.6.9. Creating a Simple Filesystem
      3.7. Network Drivers (2 hours)
           3.7.1. Network Addresses, Protocols and Interfaces
           3.7.2. Network Stack and Its Data Structures
           3.7.3. Packet Transfer and Acceptance
           3.7.4. Concurrency, Scatter/Gather I/O, Timeouts and Interrupts
           3.7.5. Link States, Socket Buffers, Data-Linke Layer
           3.7.6. Creating a Basic Network Driver
      3.8. Net-Filter Hooks (4 hours)
           3.8.1. Iptables and Net-Filter
           3.8.2. Hook Chaines, Priority and Position
           3.8.3. Protocols, Addresses and Interface Filtering
           3.8.4. Intercepting a Packet Content
           3.8.5. Creating a Rule-Based Firewall
      3.9. Miscellaneous Drivers (4 hours)
           3.9.1 Platform Driver Interface
           3.9.2. Misc Driver Interface
           3.9.3. System Call Interface
           3.9.4. System Call Hooks
           3.9.5. Power Management Interface
