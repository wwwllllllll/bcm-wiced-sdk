=====================================================================
Gedday : The WICED Network Discovery Library
=====================================================================

What is Gedday?
-------------------------------------------
Gedday is a network service discovery library that incorporates various
components of Zeroconf.

Zeroconf (Zero configuration networking) is a set of techniques that
automatically creates a usable Internet Protocol (IP) network without
manual operator intervention or special configuration servers.
 -- http://en.wikipedia.org/wiki/Zero_configuration_networking


Why do I need it? What is it used for?
-------------------------------------------
Adding network discovery to your application enables the application
to advertise services such as a web server. A client on the same
network (eg. a smartphone or tablet) can use a network discovery client
such as a Bonjour browser to find the service.


 What protocols are supported?
-------------------------------------------
- Multicast DNS / DNS Service Discovery (similar to the Apple 'Bonjour' protocol)
  A lightweight version of mDNS/DNS-SD is provided.
  A complete implementation of Bonjour suitable for WICED is
  available directly from Apple.


What additional protocols are planned?
-------------------------------------------
- uPnP SSDP (Microsoft protocol)
- Additions to work with the Android Network Service Discovery Manager


What are the limitations?
-------------------------------------------
The Gedday library is still under development and currently has a number of limitations:
- Maximum number of advertised services is limited. The total depends on
  the service name lengths; we recommend advertising no more than 2 services for now.
- Services may only be offered on one interface at a time (softAP or STA interface)
- There is no support for IPv6 ... yet.
- The service discovery API is declared but is not fully functional. 

