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
# build and install the final compiler
#---------------------------------------------------------------------------------

echo "Start of build:" > 08-temp.txt
date >> 08-temp.txt

cd gcc-build

export PATH=$prefix/bin/Linux32:$PATH
export LD_LIBRARY_PATH=`pwd`/../gmp-build/.libs/:$LD_LIBRARY_PATH

echo LD_LIBRARY_PATH = $LD_LIBRARY_PATH

$MAKE || { echo "Error building gcc2"; exit 1; }
$MAKE install || { echo "Error installing gcc2"; exit 1; }

cd ..

if [ "$OSTYPE" == "linux-gnu"  -o "$OSTYPE" == "linux" ]
then
if [ "$HOSTTYPE" == "x86_64" ]
then
export HOST_DIR=Linux64
else
export HOST_DIR=Linux32
fi
export LIB_PATH_VAR=LD_LIBRARY_PATH
elif [[ "$OSTYPE" == *darwin* ]]
then
export HOST_DIR=OSX
export LIB_PATH_VAR=DYLD_LIBRARY_PATH
fi

if [ "$OSTYPE" != "msys" ]
then
mv install/bin/$HOST_DIR/internal/cc1 install/bin/$HOST_DIR/internal/cc1_run
cp `which dirname` install/bin/$HOST_DIR/internal/cc1_dirname

echo "#!/bin/bash" > install/bin/$HOST_DIR/internal/cc1
echo "export LIB_PATH_VAR=\`\$0_dirname \$0\`:\$LIB_PATH_VAR" >> install/bin/$HOST_DIR/internal/cc1
echo "\${0}_run \"\$@\"" >> install/bin/$HOST_DIR/internal/cc1
chmod a+x install/bin/$HOST_DIR/internal/cc1
cp -P addontools/lib/libgmp.s* install/bin/$HOST_DIR/internal/
fi

rm -rf install/arm-none-eabi

echo "End of build:" >> 08-temp.txt
date >> 08-temp.txt
mv 08-temp.txt 08-ready.txt
