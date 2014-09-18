----------------------------------------------------------
WICED Application Directory - README
----------------------------------------------------------

This directory contains various sub-directories with applications
that demonstrate how to use the WICED API and how to build
advanced applications. Several applications are also provided to
test WICED hardware.

Sub-directories
------------------------------
 - demo : Contains advanced demonstration applications that combine
          various WICED features into a single application
 - snip : Application snippets used to demonstrate various WICED APIs
 - test : Test applications used for manufacturing and certification
 - waf  : Application components that are part of the WICED Application Framework
 - wwd  : WICED Wi-Fi Driver example applications. These applications demonstrate
          how to bypass the WICED API and use the low-layer WICED Wi-Fi Driver directly

 Pre-Requisites for running WICED Applications
 ---------------------------------------------
 ***PLEASE*** read through the WICED Quickstart Guide located
 in the <WICED-SDK>/Doc directory before attempting to use any of
 the applications in the application sub-directories!


Application Demonstration Requirements
--------------------------------------
* A development computer with the following software:
    * The WICED IDE & SDK installed
    * A terminal application (such as PuTTY). The WICED Quickstart Guide
      located in the <WICED-SDK>/Doc directory describes how to configure
      a terminal application.
    * An up-to-date web browser such as Chrome, Safari or Internet Explorer
    * Python 2.7. Some applications require Python to enable the development
      computer to act as a UDP or TCP server. Python is available for download here:
      http://www.python.org/getit/releases/2.7.3/
* A Wi-Fi Access Point with internet connectivity
    All applications that demonstrate client (STA) mode assume the Wi-Fi AP is
    configured to use WPA2 security and has the following credentials:
    * SSID       : YOUR_AP_SSID
    * Passphrase : YOUR_AP_PASSPHRASE






