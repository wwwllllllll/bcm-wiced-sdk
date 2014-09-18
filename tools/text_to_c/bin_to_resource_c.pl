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
    print "Usage ./text_to_c.pl  <MEM|FILESYS> <variable name> <text file>";
    exit;
}

# Print start of output
my $location = shift @ARGV;
my $variable_name = shift @ARGV;
my $original_variable_name = $variable_name;
my $file = shift @ARGV;

#open the file
open INFILE, "<:raw", $file or die "cant open " . $file;
@file_cont_array = <INFILE>;
close INFILE;
$file_cont = join('',@file_cont_array);


print "#include \"wiced_resource.h\"\n";
print "\n";
$name = $file;
$name=~s/^.+\/(.*?)/$1/;

if ( $location ne "MEM" )
{
    print "const resource_hnd_t ${variable_name} = { RESOURCE_IN_FILESYSTEM,  " . (length( $file_cont )) . ", {.fs = { 0, \"$name\" } }};\n";
    print "\n";
}
else
{
    print "const char ${variable_name}_data[" . (length( $file_cont )) . "] = {\n";
    my @vals = unpack( "C*", $file_cont );

    my $linepos;
    my $firstval = 1;

    foreach $val (@vals)
    {
        my $valsize = 1;
        if ( ( $val >= 10 ) && ( $val < 100 ) )
        {
            $valsize = 2;
        }
        elsif ( $val >= 100 )
        {
            $valsize = 3;
        }

        if ( $firstval == 1 )
        {
            print "        $val";
            $linepos = 8 + $valsize;
            $firstval = 0;
        }
        elsif ( $linepos + 5 >= 79 )
        {
            print ",\n        $val";
            $linepos = 8 + $valsize;
        }
        else
        {
            print ", $val";
            $linepos += 2 + $valsize;
        }
    }

    print "\n};\n";
    print "const resource_hnd_t ${variable_name} = { RESOURCE_IN_MEMORY, " . (length( $file_cont )) . ", {.mem = { (const char *) ${variable_name}_data }}};\n";
    print "\n";
}

