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

echo "Extracting $MPFR_SRC"
tar -xjvf download/$MPFR_SRC.tar.bz2 || { echo "Error extracting "$MPFR_SRC; exit; }

echo "Start of build:" > 03-temp.txt
date >> 03-temp.txt 

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

mkdir -p mpfr-build
cd mpfr-build



if [ "$OSTYPE" == "msys" ]
then
export CFLAGS="-D__USE_MINGW_ACCESS -I$SCRIPTDIR/$GMP_SRC -I$SCRIPTDIR/gmp-build"
else
export CFLAGS="-I$SCRIPTDIR/$GMP_SRC -I$SCRIPTDIR/gmp_build"
fi

../$MPFR_SRC/configure $ABI_MODE \
	--prefix=$addon_tools_dir --disable-shared \
	--with-gmp-build=$SCRIPTDIR/gmp-build \
	|| { echo "Error configuring mpfr"; exit 1; }

$MAKE || { echo "Error building mpfr"; exit 1; }
$MAKE install || { echo "Error installing mpfr"; exit 1; }

cd ..

echo "End of build:" >> 03-temp.txt
date >> 03-temp.txt 
mv 03-temp.txt 03-ready.txt


