#!/bin/bash

# 
# YAGARTO toolchain                                                       
#                                                                            
# Copyright (C) 2006-2011 by Michael Fischer                                      
# Michael.Fischer@yagarto.de                                                 
#
# Copyright (C) 2011 by Broadcom Inc.
#                                                                            
# This program is free software; you can redistribute it and/or modify       
# it under the terms of the GNU General Public License as published by       
# the Free Software Foundation; either version 2 of the License, or          
# (at your option) any later version.                                        
#                                                                            
# This program is distributed in the hope that it will be useful,            
# but WITHOUT ANY WARRANTY; without even the implied warranty of             
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              
# GNU General Public License for more details.                               
#                                                                            
# You should have received a copy of the GNU General Public License          
# along with this program; if not, write to the Free Software                
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA 
# 

set -e

#---------------------------------------------------------------------------------
# Setup the environment
# 
# This is done by a separate script, because if you have close your shell
# you can set the environment again with this script only.
#---------------------------------------------------------------------------------

if [ "$OSTYPE" == "msys" ]
then
. _env-only.sh
else
source ./_env-only.sh
fi

#---------------------------------------------------------------------------------
# build and install newlib
#---------------------------------------------------------------------------------

echo "Extracting $NEWLIB_SRC"
tar -xzvf download/$NEWLIB_SRC.tar.gz || { echo "Error extracting "$NEWLIB_SRC; exit; }

echo
echo "Patching $NEWLIB_SRC"
patch -p1 -d $NEWLIB_SRC < patches/$NEWLIB_SRC.patch || { echo "Error patching "$NEWLIB_SRC; exit; }
echo

echo "Start of build:" > 07-temp.txt
date >> 07-temp.txt 

mkdir -p newlib-build
cd newlib-build
mkdir -p etc

if [ "$OSTYPE" != "msys" ]
then
export LD_LIBRARY_PATH=$(pwd)/../addontools/lib/
fi


if [ "$OSTYPE" == "msys" ]; then
export CFLAGS=-D__USE_MINGW_ACCESS
if [ "$HOSTTYPE" == "x86_64" ]; then
export HOST_DIR=Win64
else
export HOST_DIR=Win32
fi
elif [ "$OSTYPE" == "linux-gnu" -o "$OSTYPE" == "linux" ]; then
if [ "$HOSTTYPE" == "x86_64" -o "$HOSTTYPE" == "x86_64-linux" ]; then
export HOST_DIR=Linux64
else
export HOST_DIR=Linux32
fi
elif [[ "$OSTYPE" == *darwin* ]]; then
export zlib="--with-system-zlib"
export HOST_DIR=OSX
else
echo Unknown OS $OSTYPE
exit -1
fi

# note - GCC optimisation fno-tree-loop-distribute-patterns causes libc functions to call themselves in an endless loop
# see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=56888 for details

export CC_FOR_TARGET="$prefix/bin/$HOST_DIR/$target-gcc -fno-tree-loop-distribute-patterns"
export AS="$prefix/bin/$HOST_DIR/$target-as"
export PATH=$prefix/bin/$HOST_DIR:$PATH

../$NEWLIB_SRC/configure --target=$target \
						 --prefix=$prefix \
						 --enable-interwork \
						 --enable-multilib \
						 --disable-newlib-supplied-syscalls \
						 CFLAGS="-O3 \
						         -D_ELIX_LEVEL=0 \
								 -ffunction-sections \
								 -fdata-sections \
								 -B$prefix/bin/$HOST_DIR \
								 -I$prefix/lib/include \
								 -I$prefix/lib/include-fixed" \
				 || { echo "Error configuring newlib"; exit 1; }

$MAKE || { echo "Error building newlib"; exit 1; }
$MAKE install || { echo "Error installing newlib"; exit 1; }

cd ..

echo "End of build:" >> 07-temp.txt
date >> 07-temp.txt 
mv 07-temp.txt 07-ready.txt
