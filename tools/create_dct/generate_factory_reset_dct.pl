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


my %value_table = ();

# Read in STDIN
my %input = "";
while ( <STDIN> )
{
    $input = $input.$_;
}

# Extract all the matching pairs
while ($input =~ m/^(\w+?)\s*=(.*?)(?:\Z|(?=\n^\w))/smg)
{
   $value_table{$1} = $2;
}

# Open the source factory reset file and read content
open FACTORY_RESET_FILE, $ARGV[0] or die "Couldn't open file ($ARGV[0]): $!";
@file = <FACTORY_RESET_FILE>;
close FACTORY_RESET_FILE;

# Print out new version with dynamic content replaced
$file_cont = join('',@file);
$file_cont =~ s/_DYNAMIC_(\w*)/$value_table{$1}/g;
print "$file_cont";
