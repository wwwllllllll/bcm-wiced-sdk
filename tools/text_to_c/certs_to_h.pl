#!/usr/bin/perl

#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

if (! $ARGV[0] )
{
    print "Usage ./certs_to_h.pl  <variable name> <cert file 1> <cert file 2> ...";
    print "Outputs a C header containing a #defined macro string of the concatenated certificate files";
    exit;
}

# Print start of output
$variable_name = shift @ARGV;
print "#define $variable_name  ";

foreach $file (@ARGV)
{
    #open the file
    open INFILE, "<:raw", $file or die "cant open " . $file;
    print "\\\n";
    while ($line = <INFILE>)
    {
        $line =~ s/\r//;
        $line =~ s/\n//;
        print "\"$line\\r\\n\"\\\n";
    }
    print "\"\\0\"";
    close INFILE;
}

print "\\\n\"\\0\"\n"

