#!/bin/bash

set -e


export LIBUSB_WIN32_VER=1.2.2.0
export LIBUSB_WIN32_URL=http://nchc.dl.sourceforge.net/project/libusb-win32/libusb-win32-releases/$LIBUSB_WIN32_VER/libusb-win32-src-$LIBUSB_WIN32_VER.zip
export LIBUSB_VER=1.0.9
export LIBUSB_URL=http://netcologne.dl.sourceforge.net/project/libusb/libusb-1.0/libusb-$LIBUSB_VER/libusb-$LIBUSB_VER.tar.bz2
export LIBUSB_COMPAT_VER=0.1.5
export LIBUSB_COMPAT_URL=http://heanet.dl.sourceforge.net/project/libusb/libusb-compat-0.1/libusb-compat-$LIBUSB_COMPAT_VER/libusb-compat-$LIBUSB_COMPAT_VER.tar.bz2
export LIBFTDI_VER=0.19
export LIBFTDI_URL=http://www.intra2net.com/en/developer/libftdi/download/libftdi-$LIBFTDI_VER.tar.gz
export OPENOCD_REPO_URL=http://repo.or.cz/r/openocd.git
export OPENOCD_REPO_HASH=a4d344651473d55c88cdf8b0d8f30467d0093e18
#export OPENOCD_REPO_HASH=master

export INSTALL_DIR=install
export DOWNLOAD_DIR=download

#export DEBUG_OPENOCD=yes

export OLDPATH=$PATH

if [ "$OSTYPE" == "msys" ]; then
# Must not have spaces in path when set inside MinGW for some reason
export PATH=$PATH:/c/PROGRA~1/Git/bin/
export WGET=wget
if [ "$HOSTTYPE" == "x86_64" ]; then
export HOST_TYPE=Win64
else
export HOST_TYPE=Win32
fi
elif [ "$OSTYPE" == "linux-gnu" -o "$OSTYPE" == "linux" ]; then
export WGET=wget
if [ "$HOSTTYPE" == "x86_64" -o "$HOSTTYPE" == "x86_64-linux" ]; then
export HOST_TYPE=Linux64
else
export HOST_TYPE=Linux32
fi
elif [[ "$OSTYPE" == *darwin* ]]; then
export WGET="curl -O"
export HOST_TYPE=OSX
else
echo "Unknown OS $OSTYPE"
exit -1
fi



# Check git is installed
if ( ! ( git --version ) ); then echo "git not found! Ensure you have installed git and it is in the system path"; exit 1 ; fi


# Create download directory
if [ ! -d ./$DOWNLOAD_DIR ]
then
	echo "Creating download directory"
	mkdir -p $DOWNLOAD_DIR
fi

# Fetch OpenOCD
if [ ! -e ./$DOWNLOAD_DIR/openocd-$OPENOCD_REPO_HASH.tar.gz ]
then
	echo "Downloading OpenOCD"
	cd ./$DOWNLOAD_DIR
	rm -rf ./openocd/
	git clone $OPENOCD_REPO_URL
	cd openocd
	git checkout  $OPENOCD_REPO_HASH
	git submodule init
	git submodule update
	cd ..
	tar -zcvf openocd-$OPENOCD_REPO_HASH.tar.gz openocd/
	rm -rf openocd/
	cd ..
fi

if [ "$DEBUG_OPENOCD" == "yes" ]; then
	export EXTRA_CFLAGS=-g -O0
fi


# Extract OpenOCD
echo "Extracting OpenOCD"
rm -rf ./openocd/
tar -zxvf ./$DOWNLOAD_DIR/openocd-$OPENOCD_REPO_HASH.tar.gz

# Fetching OpenOCD submodules 
# cannot be done earlier as the following bug puts absolute paths in the git submodules:
# http://git.661346.n2.nabble.com/PATCH-0-2-submodules-Use-relative-paths-to-gitdir-and-work-tree-td7267485.html
echo "Fetching OpenOCD submodules"
cd openocd
git checkout -b Broadcom_only
#git submodule init
#git submodule update

# Patching OpenOCD
echo "Patching OpenOCD"
git am ../0001-Add-verify_image_checksum-command.patch
#git am ../0002-Change-STM32-Hack-to-allow-OpenOCD-to-work-whilst-th.patch
git am ../0003-Add-hack-to-turn-off-Control-C-handling-in-windows-t.patch
git am ../0004-FT2232-driver-simplification.patch
git am ../0005-Add-BCM9WCD1EVAL1-device-to-FT2232-driver.patch
git am ../0006-Fix-printf-format-errors-in-MinGW-32-bit.patch
#git am ../0007-Add-support-for-64-bit-parameter-to-irscan.patch
#git am ../0009-Allow-autoscan-up-to-64-bit-IR-lengths.patch
#git am ../0010-Ensure-Cortex-M-reset-wakes-device-from-sleep-wfi-wf.patch
#git am ../0011-cortex_a-optimize-apb-read-write-access.patch
#git am ../0012b-adi_v5-search-for-Debug-and-Memory-AP-support.patch
#git am ../0013-arch-Added-ARMv7R-and-Cortex-R4-support.patch
git am ../0014-Change-trace-command-to-avoid-clash-with-TCL-trace-c.patch
git am ../0015-Add-more-logging-for-GDB-arm-mode-error.patch
#git am ../0016-adapter-interface-Cleanup-of-jtag_interface-structur.patch
git am ../0019-Squashed-libSWD-changes.patch
git am ../0020-Fix-libswd-configure.patch
#git am ../0021-rtos-Fix-regression-which-was-preventing-use-of-firs.patch
git am ../0022-Change-RnW-signal-to-allow-for-opposite-polarity-by-.patch
git am ../0024-Add-initial-functions-for-memory-map-support.patch
#git am ../0025-Fixes-for-latest-GCC.patch
#git am ../0031-Add-abort-when-JTAG-DP-transaction-times-out.patch
git am ../0032-Fix-ThreadX-FreeRTOS-current-execution-when-there-ar.patch
git am ../0033-Allow-static-building-of-OpenOCD.patch
#git am ../0035-Fix-buffer-overrun-sprintf-appends-a-terminating-nul.patch
git am ../0036-RTOS-add-ability-to-wipe-RTOS-before-use-to-avoid-ol.patch
git am ../0037-jtag-Avoid-extra-srst-resets.patch
git am ../0038-Temporary-fix-of-mem_ap_read_buf_u32_swd.patch
git am ../0039-Fix-FreeRTOS-awareness-to-work-with-latest-FreeRTOS-.patch
#git am ../0040-libSWD-Add-patch-for-libswd-autoreconf-error.patch
git am ../0041-Update-JimTCL-to-revision-ab939f33357c7ee7825aff0ecf.patch
cd ..


if [ "$OSTYPE" == "msys" ]; then
	export EXTRA_OPENOCD_CFGOPTS="--enable-parport --enable-parport-giveio --enable-amtjtagaccel --enable-gw16012"
fi

if [ "$OSTYPE" == "linux-gnu" ]; then
    export EXTRA_OPENOCD_CFGOPTS="--enable-amtjtagaccel --enable-gw16012"
fi

if [[ "$OSTYPE" == *darwin* ]]; then
    export EXTRA_CFLAGS=" -framework IOKit -framework CoreFoundation"
    export EXTRA_CFLAGS="$EXTRA_CFLAGS -L`pwd`/libusb-install/lib/ -lusb-1.0"
else
    export EXTRA_OPENOCD_CFGOPTS="$EXTRA_OPENOCD_CFGOPTS --enable-static"
fi

# Build OpenOCD
echo "Building OpenOCD"
rm -rf openocd-build openocd-install
mkdir -p openocd-build
mkdir -p openocd-install
cd openocd
./bootstrap
export PATH=$OLDPATH

cd jimtcl
git reset --hard
git am ../../0008-JimTCL-Add-variable-tracing-functionality.patch
#git am ../../0017-JimTCL-Fix-bug-that-was-causing-the-system-environ-variable.patch
git am ../../0034-Allow-fully-static-linking-of-JimTCL.patch
cd ..

cd ../openocd-build
export LD_LIBRARY_PATH=`pwd`/../libftdi-install/lib/:$LD_LIBRARY_PATH
../openocd/configure $EXTRA_OPENOCD_CFGOPTS --enable-dummy --enable-ep93xx --enable-at91rm9200 --enable-usbprog --enable-jlink --enable-vsllink --enable-rlink --enable-arm-jtag-ew --enable-ft2232_libftdi --enable-usb_blaster_libftdi --enable-presto_libftdi --disable-option-checking  --prefix=`pwd`/../openocd-install/ --program-suffix=-all-brcm-libftdi --enable-maintainer-mode --enable-hndjtag CFLAGS="-g -L`pwd`/../libftdi-install/lib/ -I`pwd`/../libftdi-install/include/ -L`pwd`/../libusb-win32-src-$LIBUSB_WIN32_VER/ -L`pwd`/../libusb-compat-install/lib/ -I`pwd`/../libusb-compat-install/include/  -I`pwd`/../libusb-win32-src-$LIBUSB_WIN32_VER/src/  -I/opt/local/include -I`pwd`/../openocd/src/jtag/drivers/hndjtag/include/ $EXTRA_CFLAGS"
make
make install
cd ..

# Copying OpenOCD into install directory
echo "Copying OpenOCD into install directory"
cp openocd-install/bin/openocd-all-brcm-libftdi* $INSTALL_DIR/$HOST_TYPE/

# Strip OpenOCD
if [ ! "$DEBUG_OPENOCD" == "yes" ]; then
	echo "Stripping executable"
	for f in \
		`find ./$INSTALL_DIR/$HOST_TYPE/ -name openocd-all-brcm-libftdi*`
	do
		strip $f
	done
fi

# OSX cannot be built static, so make a script to force it to find the dynamic libs
if [[ "$OSTYPE" == *darwin* ]]; then
	mv install/OSX/openocd-all-brcm-libftdi install/OSX/openocd-all-brcm-libftdi_run
	cp `which dirname` install/OSX/openocd-all-brcm-libftdi_dirname

	echo "#!/bin/bash" > install/OSX/openocd-all-brcm-libftdi
	echo "export DYLD_LIBRARY_PATH=\`\$0_dirname \$0\`:$DYLD_LIBRARY_PATH" >> install/OSX/openocd-all-brcm-libftdi
	echo "\${0}_run \"\$@\"" >> install/OSX/openocd-all-brcm-libftdi
	chmod a+x install/OSX/openocd-all-brcm-libftdi
fi

echo
echo "Done! - Output is in $INSTALL_DIR"
echo

