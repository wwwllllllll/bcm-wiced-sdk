---------------------------------------------------------------
Usage instructions for Olimex ARM USB TINY & TINY-H programmers
---------------------------------------------------------------

1. Plug the Olimex USB JTAG Tiny (or Tiny-H) programmer into a USB port on your PC

2. Download & extract http://sourceforge.net/projects/libusb-win32/files/libusb-win32-releases/1.2.6.0/libusb-win32-bin-1.2.6.0.zip
   to a temporary directory

3. Double-click the <TEMP_DIR>\libusb-win32-bin-1.2.6.0\bin\inf-wizard.exe to create a driver for the ARM-USB-TINY (or ARM-USB-TINY-H),
   save the driver to a temporary location, then install the driver. The GUI steps you through this process.

4. Ensure the WICED eval board is powered (by USB or external +5V), and that the Olimex programmer is plugged into the J-Link port on the board

5. Try downloading the scan app to the board using one of the following commands (change the build string to suit your app/platform):
     snip.scan-BCM943362WCD4 JTAG=Olimex_ARM-USB-TINY download run
     snip.scan-BCM943362WCD4 JTAG=Olimex_ARM-USB-TINY-H download run

6. If the driver installation worked, the blue JTAG blinks during the download process