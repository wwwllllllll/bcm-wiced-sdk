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

echo "Extracting $EXPAT_SRC"
tar -xzvf download/$EXPAT_SRC.tar.gz || { echo "Error extracting "$EXPAT_SRC; exit; }

echo "Start of build:" > 01-temp.txt
date >> 01-temp.txt 

mkdir -p expat-build
cd expat-build

if [ "$OSTYPE" == "msys" ]
then
export CFLAGS=-D__USE_MINGW_ACCESS
fi

../$EXPAT_SRC/configure \
	--prefix=$addon_tools_dir --disable-shared \
	|| { echo "Error configuring expat"; exit 1; }

$MAKE || { echo "Error building expat"; exit 1; }
$MAKE install || { echo "Error installing expat"; exit 1; }

cd ..

echo "End of build:" >> 01-temp.txt
date >> 01-temp.txt 
mv 01-temp.txt 01-ready.txt


