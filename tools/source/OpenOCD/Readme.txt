Building OpenOCD
================

This file describes how to build the OpenOCD binary that is shipped with the WICED-SDK  
It can be compiled using MinGW, Linux or Mac OS X.

Prerequisites:
	* Linux    :  Install packages "libusb-dev", "git" "autotools"
	* Mac OS X : Install MacPorts from macports.org then install "libusb-compat" with "sudo port install libusb-compat"
	* Windows  : install MinGW from mingw.org. execute build script from MinGW bash shell
	
	
Building:
	* Open bash and go to Tools/source/OpenOCD/
    * ./build_all.sh


Your OpenOCD binaries should be in Tools/source/OpenOCD/install/
        