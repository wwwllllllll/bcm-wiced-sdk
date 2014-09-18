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

echo "Removing expat"
rm -rf $EXPAT_SRC/
rm -rf expat-build/
rm -f 01-temp.txt
rm -f 01-ready.txt

echo "Removing gmp"
rm -rf $GMP_SRC/
rm -rf gmp-build/
rm -f 02-temp.txt
rm -f 02-ready.txt

echo "Removing mpfr"
rm -rf $MPFR_SRC/
rm -rf mpfr-build/
rm -f 03-temp.txt
rm -f 03-ready.txt

echo "Removing mpc"
rm -rf $MPC_SRC/
rm -rf mpc-build/
rm -f 04-temp.txt
rm -f 04-ready.txt

echo "Removing binutils"
rm -rf $BINUTILS_SRC/
rm -rf binutils-build/
rm -f 05-temp.txt
rm -f 05-ready.txt

echo "Removing gcc"
rm -rf $GCC_SRC/
rm -rf gcc-build/
rm -f 06-temp.txt
rm -f 06-ready.txt
rm -f 08-temp.txt
rm -f 08-ready.txt

echo "Removing newlib"
rm -rf $NEWLIB_SRC/
rm -rf newlib-build/
rm -f 07-temp.txt
rm -f 07-ready.txt

echo "Removing gdb"
rm -rf $GDB_SRC/
rm -rf gdb-build/
rm -f 09-temp.txt
rm -f 09-ready.txt

echo "Removing strip results"
rm -f 10-temp.txt
rm -f 10-ready.txt


echo "Removing addontools"
rm -rf addontools/

echo "Removing install"
rm -rf install/

echo "Removing xx-ready"
rm -f xx-ready.txt


