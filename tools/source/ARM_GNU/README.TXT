
GNU ARM Toolchain  v20120313
=============
This file describes how to build the GNU ARM toolchain that is shipped with the WICED-SDK.

This Toolchain can be built for both Linux and Windows.

First you will need to download the following source code files : 

binutils-2.23.2.tar.bz2  from http://ftp.gnu.org/gnu/binutils/binutils-2.23.2.tar.bz2
expat-2.1.0.tar.gz       from http://downloads.sourceforge.net/project/expat/expat/2.1.0/expat-2.1.0.tar.gz
gcc-4.8.2.tar.bz2        from ftp://ftp.gnu.org/gnu/gcc/gcc-4.8.2/gcc-4.8.2.tar.bz2
gdb-7.6.1.tar.bz2        from http://ftp.gnu.org/gnu/gdb/gdb-7.6.1.tar.bz2
gmp-5.1.2.tar.bz2        from ftp://ftp.gmplib.org/pub/gmp/gmp-5.1.2.tar.bz2
mpc-1.0.1.tar.gz         from http://www.multiprecision.org/mpc/download/mpc-1.0.1.tar.gz
mpfr-3.1.2.tar.bz2       from http://www.mpfr.org/mpfr-current/mpfr-3.1.2.tar.bz2
newlib-2.0.0.tar.gz      from ftp://sourceware.org/pub/newlib/newlib-2.0.0.tar.gz
newlib-nano-1.0.tar.bz2  from https://launchpadlibrarian.net/143625750/gcc-arm-none-eabi-4_7-2013q2-20130614-src.tar.bz2


Unzip the build scripts file containing this readme file.
Place these files in the downloads directory under the directory containing this readme file.

==============================

Building on MINGW

Obtain MinGW installer from: http://sourceforge.net/projects/mingw/files/Automated%20MinGW%20Installer/mingw-get-inst/
In the install program, select the following items:
*  C++ Compiler
*  MSYS Basic System
*  MinGW Developer Toolkit
*  MSYS System Builder

Open a MinGW shell and go to the directory where you have unzipped the build scripts.

Build the toolchain with:

./xx-build-all.sh

You may get the following error during building of 08-build-gcc.sh
	couldn't commit memory for cygwin heap, Win32 error 0
In this case, go into the gcc-build directory and repeatedly run "make" until it builds successfully. Then run "make install". Then run the 09-build-gdb.sh and 10-strip.sh scripts.
	

The built toolchain will be located in the ./install directory.


==============================

Building on Linux

To ensure the toolchain is widely compatible, it is recommended that the toolchain is build on an old version
of Linux. This prevents the final toolchain being dependent on recent versions of glibc.

Tested on Ubuntu 11.04 (Natty Narwhal) and 11.10 (Oneric Ocelot) and on Redhat Enterprise Linux 5

Ensure the following packages are installed: 
    * libncurses5-dev 
    * texinfo
    * m4
    * zlib1g-dev

Open a bash shell and go to the directory where you have unzipped the build scripts.

Build the toolchain with:

./xx-build-all.sh

The built toolchain will be located in the ./install directory.

There may be issues that need resolving with the built toolchain:

* copy libgmp.so, libgmp.so.10, and libgmp.so.10.0.4 from ./addontools/lib  to ./install/lib

* rename ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1 to ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1_run

* create an executable script with the following content called ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1
	#!/bin/bash
	export LD_LIBRARY_PATH=`$0_dirname $0`/../../../../lib:$LD_LIBRARY_PATH
	${0}_run "$@"

* Copy linux "dirname" executable from /usr/bin/dirname to ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1_dirname

==============================

Building on OS X

Tested on OSX 10.7.4

Open a bash shell and go to the directory where you have unzipped the build scripts.

Build the toolchain with:

./xx-build-all.sh

The built toolchain will be located in the ./install directory.

There may be issues that need resolving with the built toolchain:

* copy libgmp.so, libgmp.so.10, and libgmp.so.10.0.4 from ./addontools/lib  to ./install/lib

* rename ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1 to ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1_run

* create an executable script with the following content called ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1
	#!/bin/bash
	export LD_LIBRARY_PATH=`$0_dirname $0`/../../../../lib:$LD_LIBRARY_PATH
	${0}_run "$@"

* Copy linux "dirname" executable from /usr/bin/dirname to ./install/libexec/gcc/arm-none-eabi/4.6.3/cc1_dirname

==============================

Patch Descriptions :

binutils-2.23.2.patch - Partly from previous Yagarto toolchain 
                      - Also contains patch (with files below) for adding section overflow handling feature to LD
ldgram.c, ldgram.h, ldlex.c - New versions of files created by Bison/Flex for adding section overflow handling - diffs are only large due to different versions of Bison/Flex 
gcc-4.8.2.patch       - From previous Yagarto toolchain
gdb-7.6.1.patch       - fix for bug causing symbol lookup to not be offered - http://sourceware.org/bugzilla/show_bug.cgi?id=12648
newlib-2.0.0.patch    - libgloss does not work with ARM v6m devices (Cortex-M0 etc) - disable building it in this mode.

Patches also create a custom output directory structure, and avoid outputting un-needed files

 
==============================

Toolchain scripts originally from Yagarto ( http://www.yagarto.de )
Modified by Broadcom Inc. for WICED project.

