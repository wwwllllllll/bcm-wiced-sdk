#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

#!/usr/bin/env python
import socket
import optparse
import time
import sys


BUFFER_SIZE = 1024
MESSAGE = "Hello Wiced!"

# IP details for the WICED TCP server
DEFAULT_IP   = '172.16.61.243'   # IP address of the WICED TCP server
DEFAULT_PORT = 50007             # Port of the WICED TCP server

DEFAULT_KEEP_ALIVE = 0           # Keep the connection alive (=1), or close the connection (=0)


def tcp_client( server_ip, server_port, test_keepalive ):
    message_count=0;
    print "Starting tcp client"
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((server_ip, server_port))
    print "Connected.."
    print "Send message: %s\r\n" % MESSAGE
    s.send(MESSAGE)
    data = s.recv(BUFFER_SIZE)
    if ( test_keepalive == 1 ):
        time.sleep(10000)
        s.close()


if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option("--hostip", dest="hostip", default=DEFAULT_IP, help="Hostip to listen on.")
    parser.add_option("-p", "--port", dest="port", type="int", default=DEFAULT_PORT, help="Port to listen on [default: %default].")
    parser.add_option("--test_keepalive", dest="test_keepalive", type="int", default=DEFAULT_KEEP_ALIVE, help="Test keepalive capability")
    (options, args) = parser.parse_args()
    #start tcp client
    tcp_client(options.hostip, options.port, options.test_keepalive)

