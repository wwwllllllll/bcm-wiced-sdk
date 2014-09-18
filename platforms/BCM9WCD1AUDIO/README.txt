--------------------------------------------
BCM9WCD1AUDIO - README
--------------------------------------------

Provider    : Broadcom
Website     : http://broadcom.com/wiced
Description : Broadcom Audio Evaluation Board

Schematics & Photo : /platforms/BCM9WCD1AUDIO/schematics/

**** NOTE : This board requires extra hardware for debugging.
            You will need a BCM9FTDIAD_3 board and a matching
            10 way micro ribbon cable (see /platforms/evaluation_boards/

WLAN + MCU SIP details
  Mfr : SPIL (Siliconware Precision Industries Co., Ltd)
  P/N : SSB-WM-N02
  MCU     : STM32F417 Bare Die
  WLAN    : BCM43362KUBG

Bluetooth Module details
  Mfr     : USI (Universal Global Scientific Industrial Co., Ltd)
  P/N     : BM-GP-BR-65
  BT      : BCM20702A1KWFBG
  Antenna : Chip Antenna

EVB details
  Clock Generator : AK8128C from Asahi Kasei Microdevices (AKM)
  Serial Flash    : 16Mbit Macronix MX25L1606EZUI-12G
  Audio Codec     : Wolfson WM8533ECSN
  Extra RAM       : 64Mbit PSRAM   IS66WVE4M16BLL-70BLI from Integrated Silicon Solution, Inc. (ISSI)
  WLAN Antennas   : Diversity with two printed antennae (and in-line switched Murata MM8130-2600 RF connectors)

EVB Features
  USB device/host/OTG port for STM32F417
  Power supply : USB or via ribbon cable from BCM9FTDIAD_3
  Reset button
  WPS button
  Module current monitor
  Peripherals
     - 2 x Buttons
     - 2 x LEDs
  10-pin Debug/UART micro header for ribbon cable to BCM9FTDIAD_3
  Break-away wing board
     - MCU I/O header
     - BT I/O header
     - WLAN JTAG header
     - I2S Audio header
     - Footprint & header for authentication chip
     - Alternate Debug UART headers for different size/gender cables.

