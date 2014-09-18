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

open(FILE, $ARGV[0]) or die "Can't read file 'filename' $ARGV[0]\n";
$header = <FILE>;
@var_lines = <FILE>;
close (FILE);


foreach $var_line (@var_lines)
{
    if ( $var_line =~ m/^(0x\w+) \w (\S+)$/ )
    {
        print "$2 = ( DEFINED( $2 ) ? $2 : $1 );\n";
    }
    else
    {
        print "************** $var_line **************************\n";
    }
}
