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
# build and install binutils
#---------------------------------------------------------------------------------

echo "Extracting $GMP_SRC"
tar -xjvf download/$GMP_SRC.tar.bz2 || { echo "Error extracting "$GMP_SRC; exit; }

echo "Start of build:" > 02-temp.txt
date >> 02-temp.txt 

mkdir -p gmp-build
cd gmp-build

if [ "$OSTYPE" == "msys" ]
then
export CFLAGS=-D__USE_MINGW_ACCESS
fi

../$GMP_SRC/configure $ABI_MODE \
	--prefix=$addon_tools_dir \
	|| { echo "Error configuring gmp"; exit 1; }

$MAKE || { echo "Error building gmp"; exit 1; }
$MAKE check || { echo "Error checking gmp"; exit 1; }
$MAKE install || { echo "Error installing gmp"; exit 1; }

#rm $addon_tools_dir/lib/*.dylib

cd ..

echo "End of build:" >> 02-temp.txt
date >> 02-temp.txt 
mv 02-temp.txt 02-ready.txt


