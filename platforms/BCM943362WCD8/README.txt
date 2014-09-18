--------------------------------------------
BCM94332WCD8 - README
--------------------------------------------

Provider    : Broadcom 
Website     : http://go.broadcom.com/wiced
Description : Broadcom BCM943362WCD8 mounted on a Broadcom BCM9WCD1EVAL1 evaluation board 
 
Module
  Mfr     : Broadcom
  P/N     : BCM943362WCD8
  MCU     : ATSAM4S16B-MU
  WLAN    : USI WM-N-BM-02 SiP (BCM43362 Wi-Fi)
  Antenna : Diversity with two printed antennae (and in-line switched Murata MM8430 RF connectors)

EVB Features
  JLINK-JTAG debug interface
  USB-JTAG debug interface
  USB-serial UART interface
  Power supply : USB and/or external +5v
  Reset button
  Module current monitor
  Sensors/Peripherals
     - 2 x Buttons
     - 2 x LEDs
     - 1 x Thermistor
     - 1 x 8Mbit serial flash
  18-pin Expansion header
  

OpenOCD Limitations with BCM943362WCD8 Powersave
  OpenOCD is unable to wake up and halt the ARM Cortex-M3 CPU of the AT91SAM4S
  MCU when the MCU enters deep sleep mode. Consequently, the powersave feature
  for BCM943362WCD8 is disabled by default. 
  
  It can be enabled by uncommenting GLOBAL_DEFINES += WAIT_MODE_SUPPORT in 
  <WICED-SDK>/Wiced/Platform/BCM943362WCD8/BCM943362WCD8.mk.
  
  If the BCM943362WCD8 board becomes disfunctional and no firmware can be downloaded,
  it can be recovered by holding the ATSAM4S16B MCU ERASE pin to 3V3 momentarily, 
  and then try downloading again.

            
