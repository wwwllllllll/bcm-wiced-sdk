#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

#!/usr/bin/python

"""
A simple "udp echo server" for demonstrating UDP usage.
The server listens for UDP frames and echoes any received
frames back to the originating host.

"""

import socket
import string
import sys
import time
import getopt
import select


try:
    this_file = __file__
except NameError:
    this_file = "udp_echo_server.py"


def main(argv):

    HOST = ''
    PORT = 50007
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    no_echo = False

    try:
        opts, args = getopt.getopt(argv, "nhp:", ["no_echo", "help", "port="])
    except getopt.GetoptError:
        # print help information and exit:
        usage()
        sys.exit(2)

    for o, a in opts:
        if o in ("-n", "--no_echo"):
            no_echo = True
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        if o in ("-p", "--port"):
            print "port: %s => %s"%(PORT, a)
            PORT = string.atoi(a)

    try:
        s.bind((HOST, PORT))
    except socket.error, msg:
        sys.exit('Error binding UDP server to port %d : %s' % (PORT, msg))

    print "UDP echo server started."

    while 1:
        a,b,c = select.select([s], [], [], 0.01)
        if s in a:
            data, address = s.recvfrom(65536)   # This is the max. UDP frame size
            #print "dlen=", len(data)
            if not data:
                break
            if not no_echo:
                sent = 0;
                while sent < len( data ):
                    sent += s.sendto("echo: "+data[sent:], address)
            print time.strftime("%b %d %H:%M:%S ", time.localtime()), address[0], ":", repr(data.split("\x00")[0])
    s.close()


def usage():
    print "Report Server Usage:"
    print
    print "Options:"
    print "  -n or --no_echo    Optional.  When given, no data will be echoed."
    print "                     Data-echoing is enabled on default."
    print "  -h or --help       Display this help message and exit."
    print "  -p or --port       Change the port that the report server binds to."
    print

if __name__ == "__main__":
    main(sys.argv[1:])
