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
1. Part One: An Introduction To Linux Kernel Programming (10 hours)
   1.1. Linux Kernel programming Intro (2 hours)
        1.1.1. History Behind Linux Kernel
        1.1.2. Linux Kernel Versions
        1.1.3. Monolithic Architecture Vs. Micro-Kernel
        1.1.4. Kernel Modules Vs. User-Space Programs
        1.1.5. Loadable Kernel Modules Vs. Drivers
        1.1.6. Kernel Subsystems and Driver Types
        1.1.7. Setting Up an Environment
        1.1.8. Compiling a Fresh Kernel
