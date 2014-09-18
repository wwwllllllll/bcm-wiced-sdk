=====================================================================
Broadcom WICED Software Development Kit 3.1.0 BETA1 - README
=====================================================================

---------------------------------------------------------------------
\\\\\\\\\\\\\\\\\\\\\\\\\ IMPORTANT NOTICE //////////////////////////
---------------------------------------------------------------------
Early 3.1.x versions may not include all functionality provided 
in previous SDK releases. KNOWN LIMITATIONS for this release are 
described in the 'Known Limitations & Notes' section of this document.


---------------------------------------------------------------------

The WICED SDK provides a full compliment of application level APIs, 
libraries and tools needed to design & implement secure embedded wireless
networking applications. 

Major features of the WICED SDK include ...
  - Low-footprint embedded Wi-Fi Driver with Client (STA), softAP and Wi-Fi Direct
  - Wi-Fi <-> Bluetooth SmartBridge 
  - Various RTOS/TCP stack options including
    - ThreadX/NetX (IPv4), ThreadX/NetX Duo (IPv6), FreeRTOS/LwIP (IPv4)
  - Support for various Broadcom Wi-Fi & combo chips
    - BCM4390 Integrated Apps + Wi-Fi SoC
    - BCM43362 Wi-Fi SoC
    - BCM43341 Wi-Fi + Bluetooth combo SoC
  - Support for various MCU host platforms
    - ST Microelectronics : STM32F1xx, STM32F2xx, STM32F4xx
    - Atmel : AT91SAM4S16B
    - Freescale : K60
    - NXP : LPC17xx
  - RTOS & Network abstraction layer with a simple API for UDP, TCP, HTTP, HTTPS communications
  - SSL/TLS Security Library integrated with an HTTPS library for secure web transactions
  - WICED Application Framework including Bootloader, OTA Upgrade and Factory Reset
  - Automated Wi-Fi Easy Setup using one of several methods
    - SoftAP & Secure HTTP server
    - Wi-Fi Protected Setup
    - Apple Wireless Accessory Configuration (WAC) Protocol
    - Cooee(TM)
  - Simple API to provide access to MCU peripherals including UART, SPI, I2C, Timers, RTC, ADCs, DACs, etc
  - Support for multiple toolchains including GNU and IAR
 
The WICED SDK release is structured as follows:
  apps          : Example & Test Applications
  doc           : API & Reference Documentation
  include       : WICED API, constants, and defaults 
  libraries     : Bluetooth, daemons, drivers, file systems, inputs, and protocols
  platforms     : Evaluation board support package, including Eval Board and Module Schematics
  resources     : Resources used by the WICED webserver including scripts, styles, images and HTML
  tools         : Build tools, compilers, debugger, makefiles, programming tools etc.
  tools/drivers : Drivers for WICED evaluation boards
  WICED         : WICED core components (RTOS, Network Stack, Wi-Fi Driver, Security & Platform libraries)
  WICED/WWD     : The WICED Wi-Fi Driver (equivalent to the Wiced directory in previous SDK-1.x releases) 
  README.txt    : This file
  CHANGELOG.txt : A log of changes for each SDK revision
 

Getting Started
---------------------------------------------------------------------
If you are unfamiliar with the WICED SDK, please refer to the 
WICED Quickstart Guide located here: <WICED-SDK>/doc/WICED-QSG2xx-R.pdf
The WICED Quickstart Guide documents the process to setup a computer for
use with the WICED SDK, IDE and WICED Evaluation Board. 

The WICED SDK includes lots of sample applications in the <WICED-SDK>/Apps directory.
Applications included with the SDK are outlined below.
 apps/demo : Demonstration Applications
   - Applications demonstrating how to integrate various WICED API features 
 apps/snip : Application Snippets
   - Various applications to demonstrate usage of individual WICED APIs          
 apps/test : Test & Utility Applications
   - console      : Provides various test features including Iperf for throughput testing 
   - mfg_test     : Manufacturing Test application to enable radio performance and certification testing
 apps/waf  : WICED Application Framework
   - bootloader   : Bootloader application used in conjunction with the WICED Application Framework
   - ota_upgrade  : Over the Air Upgrade application
   - sflash_write : Serial flash library used to configure a serial flash for factory reset  
 apps/wwd : Wiced Wi-Fi Driver Applications to demonstrate advanced usage of the low layer Wi-Fi driver
    
To obtain a complete list of build commands and options, enter the following text in the
base WICED SDK directory on a command line:
$> make

To compile, download and run the Wi-Fi scan application on the Broadcom BCM943362WCD4 evaluation platform, 
enter the following text on a command line (a period character is used to reference applications 
in sub-directories) :
$> make snip.scan-BCM943362WCD4 download run

The default RTOS and Network Stack components are defined in the WICED configuration makefile  
at <WICED-SDK>/tools/makefiles/wiced_config.mk. The default I/O bus component is defined in the platform
makefile at <WICED-SDK>/platforms/<Platform>/<Platform>.mk. Defaults may be bypassed by specifying the 
component as part of the build string if desired as shown in the following example.
$> make snip.scan-BCM943362WCD4-FreeRTOS-LwIP-SDIO download run
       
Source code, headers and reference information for supported platforms are available 
in the <WICED-SDK>/platforms directory. Source code, headers, linker scripts etc that 
are common to all platforms are available in the <WICED-SDK>/WICED/platform directory.


Supported Features
---------------------------------------------------------------------
Wi-Fi & Bluetooth SmartBridge Features
 * Scan and associate to Wi-Fi access points
 * Authenticate to Wi-Fi Access Points with the following security types:
   Open, WEP-40, WEP-104, WPA (AES & TKIP), WPA2 (AES, TKIP & Mixed mode)
 * AP mode with support for security types : Open, WPA, WPA2
 * Concurrent AP & STA mode (AP mode limited to 5 concurrent connected clients)
 * Wi-Fi Direct
 * WPS 1.0 & 2.0 Enrollee & Registrar (Internal Registrar only)
 * Wi-Fi APIs : Network keep alive, packet filters
 * Host <-> Wi-Fi SDIO & SPI interface
 * Bluetooth SmartBridge with multiple connections including the
   following features: Whitelist, Bond Storage, Attribute Caching, 
   GATT Procedures, Configurable Maximum Concurrent Connections, Directed 
   Advertisements, Device address initialisation, Passkey entry

RTOS & Network Stack Support
 * FreeRTOS / LwIP    (full source)
 * ThreadX  / NetX    (object file; free for use with WICED *ONLY*)
 * ThreadX  / NetXDuo (object file; free for use with WICED *ONLY*)

Networking Features (IPv4 & IPv6)
 * ICMP (Ping)
 * ARP
 * TCP
 * UDP 
 * IGMP (Multicast)
 * IPv6 NDP, Multicast
 * DHCP (Client & Server)
 * DNS (Client & Redirect Server)
 * mDNS/DNS-SD Zeroconf Network Discovery (Broadcom Gedday)
 * SSL3.0 & TLS1.0/1.1 (object file with host abstraction layer; free for use with WICED *ONLY*)
 * HTTP / HTTPS (Client & Server)
 * SNTP
 * SMTP

Application Features
 * Apple AirPlay (requires Apple authentication co-processor; available to Apple MFi licensees *ONLY*) 
 * Bluetooth Audio
 * Peripheral interfaces
   * GPIO
   * Timer / PWM
   * UART
   * SPI
   * I2C
   * RTC (Real Time Clock)
 * Xively "Internet of Things" protocol

* WICED Application Framework
   * Bootloader
   * Device Configuration Table (region in flash to store AP, security credentials, TLS certs, serial number, Wi-Fi country code, etc)
   * OTA upgrade
   * Factory reset
   * Automated configuration via softAP & webserver
   * Wi-Fi Easy Setup
     * Cooee (BETA)
   * Apple Wireless Accessory Configuration (WAC) protocol (available to Apple MFi licensees *ONLY*)
   * System Monitor to manage the watchdog

Toolchains
 * GNU make
 * IAR

Hardware Platforms
 BCM43362
   * BCM943362WCD4  : Broadcom WICED Module with STM32F205 MCU mounted on BCM9WCD1EVAL1
   * BCM943362WCD6  : Broadcom WICED Module with STM32F415 MCU mounted on BCM9WCD1EVAL1
   * BCM9WCDPLUS114 : WICED+ Eval Board (includes BCM43362+STM32F205 WICED+ Module and BCM20702 Bluetooth module)
   * BCM9WCD1AUDIO  : Broadcom WICED Audio Evaluation Board (includes BCM43362, STM32F415, WM8533 audio DAC, and BCM20702 Bluetooth module)
   * BCM943362WCD4_LPCX1769 : BCM943362WCD4 module manually wired to an NXP LPCXpresso1769 Evaluation Board 
 BCM943341
   * BCM943341WCD1  : Broadcom BCM43341-based WICED Module with STM32F417 MCU mounted on BCM9WCD5EVAL1
 BCM4390
   * BCM94390WCD2   : Broadcom BCM4390 SiP-based WICED Module on BCM9WCD3EVAL1


Known Limitations & Notes
---------------------------------------------------------------------

 * Features not yet supported in WICED-SDK-3.1.0
   - Wi-Fi Direct
   - OTA upgrade functionality
   - IAR Embedded Workspace native support
   - wiced_wifi_get_counters() is not functional

 * Platform Limitations
   -----------------+-----------+-----------+-----------+-----------+-----+---------+---------+ 
   Platform Feature | STM32F1xx | STM32F2xx | STM32F4xx | AT91SAM4S | K60 | LPC17xx | BCM439x |
    Implementation  |           |           |           |           |     |         |         |
   -----------------|-----------+-----------+-----------+-----------+-----+---------+---------+
   MCU powersave    |     Y     |     Y     |     N     |     Y     |  N  |    N    |    N    |
   Wi-Fi Powersave  |     N(1)  |     Y     |     Y     |     Y     |  N  |    N    |    N    |
   I2C API          |     Y     |     Y     |     N     |     N     |  N  |    N    |    Y    |
   ADC/PWM API      |    Y/Y    |    Y/Y    |    Y/Y    |    Y/N    |  N  |    N    |    N    |
   OTA upgrade      |     N     |     Y     |     N     |     N     |  N  |    N    |    N    |
   Real Time Clock  |     N     |     Y     |     N     |     N     |  N  |    N    |    N    |
   -----------------+-----------+-----------+-----------+-----------+-----+---------+---------+  
   
   * WICED-SDK-3.1.0 Platform Restrictions
       STM32F1xx is not yet supported
       Freescale K60 is not yet supported
       BCM94390
         - Powersave for the applications processor is not yet supported
         - The SPI Slave peripherals are not yet supported
       BCM943341WCD1
         - WPS is not supported (unreliable)

   * BCM943362WCD2 Platform Restrictions (not applicable to WICED-SDK-3.1.0)
       The STM32F103 MCU on this platform only has 64kB RAM and 512kB Flash.
       Many applications that include more advanced networking features
       will NOT run on this platform! Either the application will not fit into
       Flash, or the application may run out of RAM at runtime and hang.
       Tips to use this platform:
         - Store the Wi-Fi firmware in external serial flash (or use the Wi-Fi
           firmware inside the Factory Reset image in serial flash)
         - Do not use advanced networking features like TLS & mDNS
         - Do not build applications using debug mode
     
   * Wi-Fi Powersave (1)
       The WLAN chip requires an external 32kHz sleep clock input during powersave.
       Platforms that do not support Wi-Fi powersave (per the table above) are
       not capable of driving the WLAN sleep clock. An external 32kHz clock is 
       required for these platforms.

 * libc does not include support for printing uint64_t (long long)
   
 * RTOS detection may cause OpenOCD to crash in the following situation:
     GDB has an elf containing a known RTOS loaded, and your app is using the 
     RTOS memory for other purposes. This situation may occur while debugging 
     the bootloader
     SOLUTION : Remove " -rtos auto " from the <WICED-SDK>/Tools/OpenCD/OpenOCD 
                .cfg file that matches your hardware debugger 
                (ie. BCM9WCD1EVAL1.cfg for WICED Evaluation Boards) 

 * AP mode when running with WPA/WPA2 encryption is limited to 4 STA clients

Tools
---------------------------------------------------------------------
The GNU ARM toolchain is from Yagarto, http://yagarto.de

Programming and debugging is enabled by OpenOCD, http://openocd.berlios.de

The standard WICED Evaluation board (BCM9WCD1EVAL1) provides two physical 
programming/debug interfaces for the STM32 host microprocessor: USB-JTAG and direct 
JTAG. The WICED Evaluation board driver additionally provides a single USB-serial 
port for debug printing or UART console purposes.

The USB-JTAG interface is enabled by the libftdi/libusb open source driver,
http://intra2net.com/en/developer/libftdi. 
The direct JTAG interface works with third party JTAG programmers including 
Segger, IAR J-Link and Olimex ARM-USB-TINY series. OpenOCD works with the libftdi/libusb 
USB-JTAG driver shipped with the WICED SDK and commercially available JTAG drivers 
available from third party vendors.

Building, programming and debugging of applications is achieved using either a 
command line interface or the WICED IDE as described in the Quickstart Guide.

Instructions to use the IAR toolchain are provided in a README located in the 
following directory: <WICED-SDK>/Tools/IAR 

                     
WICED Technical Support
---------------------------------------------------------------------
WICED support is available on the Broadcom forum at http://forum.broadcom.com/forum.php 

Sign-up is a two-step process: Firstly, sign up to the general Broadcom support 
forum, then apply to be a member of the WICED User Group. Be sure to identify 
yourself to the forum moderator, access to the WICED forum is restricted to 
bona-fide WICED customers only.

Broadcom provides customer access to a wide range of additional information, including 
technical documentation, schematic diagrams, product bill of materials, PCB layout 
information, and software updates through its customer support portal. For a CSP account, 
contact your Broadcom Sales or Engineering support representative.

                     
Further Information
---------------------------------------------------------------------
Further information about WICED and the WICED Development System is
available on the WICED website at http://broadcom.com/wiced or
by e-mailing Broadcom support at support@broadcom.com