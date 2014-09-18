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

Filename: webserver.py
Date: 2011-06-23
Author: chad

A simple web server that allows demonstration of IP discovery in the
Appliance app without a connection to the internet

"""

import sys
import BaseHTTPServer
from SimpleHTTPServer import SimpleHTTPRequestHandler


HandlerClass = SimpleHTTPRequestHandler
ServerClass  = BaseHTTPServer.HTTPServer
Protocol     = "HTTP/1.0"

port = 80
server_address = ('127.0.0.1', port)

HandlerClass.protocol_version = Protocol
httpd = ServerClass(server_address, HandlerClass)

sa = httpd.socket.getsockname()
print "Serving HTTP on", sa[0], "port", sa[1], "..."
httpd.serve_forever()


###################################################
#import SimpleHTTPServer
#import SocketServer

#PORT = 80

#Handler = SimpleHTTPServer.SimpleHTTPRequestHandler

#httpd = SocketServer.TCPServer(("", PORT), Handler)

#print "serving at port", PORT
#httpd.serve_forever()




##################################################
#import time
#import BaseHTTPServer


#HOST_NAME = '192.168.0.6' # !!!REMEMBER TO CHANGE THIS!!!
#PORT_NUMBER = 80 # Maybe set this to 9000.


#class MyHandler(BaseHTTPServer.BaseHTTPRequestHandler):
#    def do_HEAD(s):
#        s.send_response(200)
#        s.send_header("Content-type", "text/html")
#        s.end_headers()
#    def do_GET(s):
#        """Respond to a GET request."""
#        s.send_response(200)
#        s.send_header("Content-type", "text/html")
#        s.end_headers()
#        mainfile = open("main.htm", 'r')
#        mainfile_html = mainfile.read()
#        print mainfile_html
#        s.wfile.write(mainfile_html)
#        s.wfile.write("<frameset cols=\"25%,75%\"><frame src=\"frame_a.htm\" /><frame src=\"frame_b.htm\" /></frameset>")
        # If someone went to "http://something.somewhere.net/foo/bar/",
        # then s.path equals "/foo/bar/".
#        s.wfile.write("<p>You accessed path: %s</p>" % s.path)
#        s.wfile.write("</html>")

#if __name__ == '__main__':
#    server_class = BaseHTTPServer.HTTPServer
#    httpd = server_class((HOST_NAME, PORT_NUMBER), MyHandler)
#    print time.asctime(), "Server Starts - %s:%s" % (HOST_NAME, PORT_NUMBER)
#    try:
#        httpd.serve_forever()
#    except KeyboardInterrupt:
#        pass
#    httpd.server_close()
#    print time.asctime(), "Server Stops - %s:%s" % (HOST_NAME, PORT_NUMBER)
