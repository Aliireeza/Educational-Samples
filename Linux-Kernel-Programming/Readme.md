# Linux Kernel Educational Samples

A repository for various types of Linux Kernel modules and driver which I use to teach in my classes at Sharif University and other places


### The Goal
This module will filter all incoming and outgoing network traffic of your system on a single port, then if it received a specific ICMP packet it will open it for the sender IP address and by receiving the second magic packet it closes it again. This will be perform by registering two netfilter hooks.

### Current Samples


### Essential Needs


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
