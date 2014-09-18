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
# build and install insight
#---------------------------------------------------------------------------------

echo "Extracting $GDB_SRC"
tar -xjvf download/$GDB_SRC.tar.bz2 || { echo "Error extracting "$GDB_SRC; exit; }

echo
echo "Patching $GDB_SRC"
patch -p1 -d $GDB_SRC < patches/$GDB_SRC.patch || { echo "Error patching "$GDB_SRC; exit; }
echo

echo "Start of build:" > 09-temp.txt
date >> 09-temp.txt 

mkdir -p gdb-build
cd gdb-build

if [ "$OSTYPE" == "msys" ]; then
export CFLAGS=-D__USE_MINGW_ACCESS
export CFLAGS="$CFLAGS -static  -static-libgcc"
if [ "$HOSTTYPE" == "x86_64" ]; then
export BINDIR=$prefix/bin/Win64
else
export BINDIR=$prefix/bin/Win32
fi
elif [ "$OSTYPE" == "linux-gnu" -o "$OSTYPE" == "linux" ]; then
export zlib="--with-system-zlib"
export CFLAGS="$CFLAGS -static  -static-libgcc"
if [ "$HOSTTYPE" == "x86_64" -o "$HOSTTYPE" == "x86_64-linu" ]; then
export BINDIR=$prefix/bin/Linux64
export PATH=$prefix/bin/Linux64:$PATH
else
export BINDIR=$prefix/bin/Linux32
export PATH=$prefix/bin/Linux32:$PATH
fi
elif [[ "$OSTYPE" == *darwin* ]]; then
export BINDIR=$prefix/bin/OSX
export PATH=$prefix/bin/OSX:$PATH
else
echo Unknown OS $OSTYPE
exit -1
fi


../$GDB_SRC/configure --target=$target --prefix=$prefix --disable-nls \
       --with-libexpat-prefix=$addon_tools_dir \
       --with-jit-reader-dir=$(pwd)/junk \
       --with-gdb-datadir=$(pwd)/junk \
	--infodir=$(pwd)/junk \
	--mandir=$(pwd)/junk \
	--includedir=$(pwd)/junk \
	--libdir=$(pwd)/junk \
	--bindir=$BINDIR \
	|| { echo "Error configuring gdb"; exit 1; }

$MAKE || { echo "Error building gdb"; exit 1; }
$MAKE install || { echo "Error installing gdb"; exit 1; }

cd ..

echo "End of build:" >> 09-temp.txt
date >> 09-temp.txt 
mv 09-temp.txt 09-ready.txt

