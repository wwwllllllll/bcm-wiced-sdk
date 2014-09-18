----------------------------------------------------------
WICED Wi-Fi Driver (WWD) Application Directory - README
----------------------------------------------------------

Applications in this directory use low layer WICED WWD APIs
and DO NOT rely on the infrastructure provided by the
WICED Application Framework and high level WICED APIs

Unless you *REALLY* need to access the WICED Wi-Fi driver directly,
it should NOT be necessary to use WICED WWD APIs. Instead, please use
the standard WICED API in the <WICED-SDK>/include directory.

These applications are virtually identical to example applications
provided in earlier WICED-SDK-1.x releases which did not include an
RTOS/Network abstraction layer or the (now) extensive range of WICED
API features.

Reasons to use these examples and lower layer WICED WWD API:
  1. Your microprocessor has severe memory limitations and you need
     to minimise the size of the application and Wi-Fi driver
  2. You have your own application framework (or don't want to use
     the WICED Application Framework and want to write your own).
  3. You are looking for an example of how to use the WICED Wi-Fi driver
     with your own embedded system

Apps in this directory include:
  - appliance    : Demonstrates configuration of a device using AP/client mode, DHCP Server, DNS, WPS, webserver
  - canned_send  : Connects to an AP and sends a canned UDP packet to a fixed IP address using a
                   minimum amount of Flash and RAM. Does not use an RTOS or Network Stack.
  - ping         : Connects to an AP and regularly sends an ICMP ping to the gateway
  - scan         : Scans for Wi-Fi access points in range and prints results to the console
