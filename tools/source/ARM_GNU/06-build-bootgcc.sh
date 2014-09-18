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
# build and install just the c compiler
#---------------------------------------------------------------------------------

echo "Extracting $GCC_SRC"
tar -xjvf download/$GCC_SRC.tar.bz2 || { echo "Error extracting "$GCC_SRC; exit; }

echo
echo "Patching $GCC_SRC"
patch -p1 -d $GCC_SRC < patches/$GCC_SRC.patch || { echo "Error patching "$GCC_SRC; exit; }
echo

echo "Start of build:" > 06-temp.txt
date >> 06-temp.txt 

mkdir -p gcc-build
cd gcc-build


if [ "$OSTYPE" == "msys" ]; then
export CFLAGS=-D__USE_MINGW_ACCESS
if [ "$HOSTTYPE" == "x86_64" ]; then
export HOST_DIR=Win64
else
export HOST_DIR=Win32
fi
elif [ "$OSTYPE" == "linux-gnu"  -o "$OSTYPE" == "linux" ]; then
export zlib="--with-system-zlib"
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

export BINDIR=$prefix/bin/$HOST_DIR
export PATH=$BINDIR:$PATH
export AR_FOR_TARGET=$BINDIR/$target-ar
export RANLIB_FOR_TARGET=$BINDIR/$target-ranlib


CFLAGS="-I$addon_tools_dir/include -B $BINDIR -DLINKER_NAME=\\\\\\\"$target-ld\\\\\\\"" \
CPPFLAGS="-I$addon_tools_dir/include -B $prefix/lib -B $BINDIR -isystem $prefix/include" \
CXXFLAGS="-I$addon_tools_dir/include -B $prefix/lib -B $BINDIR -isystem $prefix/include -DLINKER_NAME=\\\\\\\"$target-ld\\\\\\\"" \
CFLAGS_FOR_TARGET="-B $prefix/lib -B $BINDIR -isystem $prefix/include" \
CPPFLAGS_FOR_TARGET="-B $prefix/lib -B $BINDIR -isystem $prefix/include" \
LDFLAGS="-L$addon_tools_dir/lib" \
../$GCC_SRC/configure \
	--target=$target --prefix=$prefix \
	--bindir=$BINDIR \
	--infodir=$(pwd)/junk \
	--mandir=$(pwd)/junk \
	--datarootdir=$(pwd)/junk \
	--with-build-time-tools=$BINDIR \
	--with-slibdir=$prefix/lib \
	--libexecdir=$BINDIR/internal \
	--with-gxx-include-dir=$prefix/include \
	--libdir=$prefix/lib \
	--with-as=$target-as \
	--with-ld=$target-ld \
	--disable-plugin \
	--disable-sjlj-exceptions \
	--disable-nls --disable-shared --disable-threads \
	--with-gcc --with-gnu-ld --with-gnu-as --with-dwarf2 \
	--enable-languages=c,c++ --enable-interwork --enable-multilib \
	--with-newlib --with-headers=../newlib-$NEWLIB_VER/newlib/libc/include \
	--disable-libssp --disable-libstdcxx-pch --disable-libmudflap \
	--disable-libgomp -v \
	$zlib \
	|| { echo "Error configuring gcc"; exit 1; }

mkdir -p libiberty libcpp fixincludes

$MAKE all-gcc || { echo "Error building gcc"; exit 1; }
$MAKE install-gcc || { echo "Error installing gcc"; exit 1; }

cd ..

echo "End of build:" >> 06-temp.txt
date >> 06-temp.txt 
mv 06-temp.txt 06-ready.txt

