

This readme will guide you to build your arm toolchain on MinGW from the very first step.


How to setup the MinGW/MSYS environment ?
------------------------------------------

For a proper MinGW/MSYS environment, you need the following tools:


MinGW-5.1.3 
(http://prdownloads.sf.net/mingw/MinGW-5.1.3.exe?download)


msys-1.0.11-2004.04.30-1 
(http://prdownloads.sf.net/mingw/MSYS-1.0.11-2004.04.30-1.exe?download)


msysDTK-1.0.1 
(http://prdownloads.sf.net/mingw/msysDTK-1.0.1.exe?download)


bison-2.1 
(http://prdownloads.sourceforge.net/gnuwin32/bison-2.1.exe?download)


flex-2.5.4a-1 
(http://prdownloads.sourceforge.net/gnuwin32/flex-2.5.4a-1.exe?download)


texinfo-4.8 
(http://prdownloads.sourceforge.net/gnuwin32/texinfo-4.8.exe?download)

================================================================================

You can download all these files in one directory. After you have download
the files, start MinGW-5.1.3.exe. The program will guide you through
the installtion process.

Choose the mirror you want, and select "Download as needed and install".
At the next page select "Current" and "Minimal" on the following page for
the components to install.

Use the default "c:\MinGW" for the destination folder. The install program
will dowload some MinGW tools during the installation process.

Now you can install MSYS, therefore start msys-1.0.11-2004.04.30-1.exe.
Install MSYS to "c:\msys\1.0", answer the post-install questions with y, 
when asked give the path as c:/MinGW

Now you can install msysDTK-1.0.1.exe to "c:\msys\1.0".

At the next steps you must install, bison, flex and texinfo.
For the destination folder use "c:\MinGW".

Congratulations, you have installed your MinGW/MSYS environment,
and can start it with the desktop icon "MSYS".

Inside the MSYS shell, type:

gcc --version

As an result, the version info of gcc will be displayed. 
The version should be 3.4.5. To check the texinfo version, type:

makeinfo --version

The result should be 4.8.

=================================================================================

Yes I know, it is a chicken-egg problem, how could you read this README,
because I have not told you how to expand. 

Copy the build-scripts-YYYYMMDD.tar.bz2 into your home folder of MSYS.
(c:\msys\1.0\home\<your-name>)

The file can now be extracted with:

tar -xjvf build-scripts-YYYYMMDD.tar.bz2

Now you will find the chicken, sorry this README.TXT and all the other scripts
you need to build the arm toolchain. An directory "download" is created too. This
is an empty directory, where you must download the arm-toolchain source.

=================================================================================

How to build an arm-toolchain on MinGW ?
-----------------------------------------

For the arm-toolchain you need the following sources:

expat-2.0.1
(http://sourceforge.net/project/showfiles.php?group_id=10127&package_id=10780&release_id=513851)


gmp-4.3.2
(ftp://ftp.gnu.org/gnu/gmp/gmp-4.3.2.tar.bz2)


mpfr-2.4.2
(http://www.mpfr.org/mpfr-2.4.2/mpfr-2.4.2.tar.bz2)


binutils 2.21
(ftp://sources.redhat.com/pub/binutils/releases/binutils-2.21.tar.bz2)


gcc-core 4.6.0
(ftp://sources.redhat.com/pub/gcc/releases/gcc-4.6.0/gcc-core-4.6.0.tar.bz2)


gcc-g++ 4.6.0
(ftp://sources.redhat.com/pub/gcc/releases/gcc-4.5.2/gcc-g++-4.6.0.tar.bz2)


newlib 1.19.0
(ftp://sources.redhat.com/pub/newlib/newlib-1.19.0.tar.gz)


gdb 7.2
(ftp://sources.redhat.com/pub/gdb/releases/gdb-7.2.tar.bz2)

mpc 0.8.1
(http://www.multiprecision.org/mpc/download/mpc-0.8.1.tar.gz)


You MUST download all these files into the download direcroty which was created
in the step before (you have expand the build-scripts-YYYYMMDD.tar.bz2, and 
created the directory)


Open a MSYS shell, and change into the directory where you have extract the
build-scripts-YYYYMMDD.tar.bz2. Here you will find some build scripts. 

Normally The scripts will be started with "./<script-name.sh>"



ATTENTION ATTENTION ATTENTION ATTENTION
=======================================

The first script, "00-set-env.sh" _MUST_ be startet with ". 00-set-env.sh"
(dot space 00-set-env.sh)

=======================================


The script will expand the source and patched if needed. After this you can
start script 01 to 09 (in this order) with "./<script-name.sh>".

At the end, you will find the arm-toolchain inside the <prefix> directory.

You can copy this directory anywhere on your windows pc for example inside
"armdev", but you must expand the PATH by <armdev>\bin.

The toolchain here was build on a Windows XP pro. SP2, without any problems (now).



Credits
--------

- Thanks to Pablo for providing the GNUARM toolchain from where I use the 
  t-arm-elf file, and get the motivation to create a MinGW based toolchain.
  
- A lot of information I have found in the internet

- Some of my scripts was derived from the devkitPro scripts (http://www.devkitpro.org/)

- Thanks to Dave from the devkitPro team (www.devkitpro.org) where I take the source for Insight from


(29.04.2010, Michael Fischer, www.yagarto.de)
