#!/usr/bin/python

"""
#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

Filename: report_server.py
Date: 2011-06-23
Author: tbriers, jasonrc

A simple "report server" for demonstrating usage of the WICED canned_send app.
The server prints data received in UDP frames to the console.

"""

import socket
import sys
import time
import ctypes


HTML_START = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Frameset//EN\"\n\
\"http://www.w3.org/TR/html4/frameset.dtd\">\n\
<HTML>\n\
<HEAD>\n\
<TITLE>A simple frameset document</TITLE>\n\
</HEAD>\n\
<FRAMESET cols=\"20%, 80%\">\n\
  <FRAME src=\"frame_a.htm\">\n\
  <FRAME src=\""
HTML_END = "\">\n\
  <NOFRAMES>\n\
      <P>This page needs frames\n\
  </NOFRAMES>\n\
</FRAMESET>\n\
</HTML>\n"

HOST = ''
PORT = 50007
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

try:
    s.bind((HOST, PORT))
except socket.error, msg:
    sys.exit('Error binding UDP server to port %d : %s' % (PORT, msg))

print
print "UDP server started. Waiting for data ..."

remote_frame = "http://www.google.com"
# Generate initial index.htm file
index_htm = open("index.htm", 'w')
index_htm.write(HTML_START + remote_frame + HTML_END)
index_htm.close()

while 1:
    data, address = s.recvfrom(1024)
    if not data:
        break
    index_htm = open("index.htm", 'w')
    index_htm.write(HTML_START + data + HTML_END)
    index_htm.close()
    print data

s.close()


