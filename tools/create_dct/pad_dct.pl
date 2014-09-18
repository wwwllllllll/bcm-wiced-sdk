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
    print "Usage ./pad_dct.pl <dct bin file>";
    exit;
}

my $filename = $ARGV[0];

my $DCT_file_len =-s $filename;

open(my $DCTFILE, '+<', $filename) || die("Error opening file: $!\n");

# read DCT header

my $buffer;
read( $DCTFILE, $buffer, 4 );
my $DCT_total_len    = unpack("I", $buffer);
read( $DCTFILE, $buffer, 4 );

seek( $DCTFILE, 0, 2 );

if ( ( $DCT_total_len - $DCT_file_len ) > 0 )
{
    # pad with 0xff
    print $DCTFILE "\xff" x ($DCT_total_len - $DCT_file_len);
}

close( $DCTFILE );

exit(0);