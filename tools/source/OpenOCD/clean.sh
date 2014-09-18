#!/bin/bash

set -e

export LIBUSB_WIN32_VER=1.2.2.0
export LIBUSB_VER=1.0.9
export LIBUSB_COMPAT_VER=0.1.5
export LIBFTDI_VER=0.19

rm -rf install libftdi-$LIBFTDI_VER libftdi-build libftdi-install libusb-win32-src-$LIBUSB_WIN32_VER openocd openocd-build openocd-install libusb-$LIBUSB_VER libusb-build libusb-install libusb-compat-$LIBUSB_COMPAT_VER libusb-compat-build libusb-compat-install
