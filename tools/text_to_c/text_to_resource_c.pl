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

my $pos = 0;

if ( ( $file =~ m/\.html$/sgi ) ||
     ( $file =~ m/\.txt$/sgi ) )
{
  while ( $file_cont =~ s/^(.*?)\r?\n?\<WICED\:section\s+suffix=\"(\S+)\"\s*\/\>\r?\n?(.*)$/$3/sgi )
  {
    my $substr = $1;
    my $variable_suffix = $2;
    my $nextpos = $-[3];

    print "\n";
    if ( $location ne "MEM" )
    {
        $name = $file;
        $name=~s/^.+\/(.*?)/$1/;
        print "const resource_hnd_t ${variable_name} = { RESOURCE_IN_FILESYSTEM, " . (length( $substr )) . ", { .fs = { $pos, \"$name\" }}};\n";
        print "\n";
    }
    else
    {
        print "const char ${variable_name}_data[" . (length( $substr )+1) . "] = ";
        my $section_length = length( $substr );
        while ( $substr =~ s/^(.*?\n)(.*)$/$2/sgi )
        {
            print "\"" . escape_string( $1 ) . "\" \\\n";
        }
        print "\"" . escape_string( $substr ) . "\";\n";

        print "const resource_hnd_t ${variable_name} = { RESOURCE_IN_MEMORY, $section_length, { .mem = { ${variable_name}_data }}};";
        print "\n";
    }

    $variable_name = $original_variable_name . $variable_suffix;
    $pos += $nextpos;
  }
}

if ( $location ne "MEM" )
{
    print "\n";
    $name = $file;
    $name=~s/^.+\/(.*?)/$1/;
    print "const resource_hnd_t ${variable_name} = { RESOURCE_IN_FILESYSTEM, " . (length( $file_cont )) . ", { .fs = { $pos, \"$name\" }}};\n";
    print "\n";
}
else
{
    print "const char ${variable_name}_data[" . (length( $file_cont ) + 1) . "] = ";
    my $section_length = length( $file_cont );
    while ( $file_cont =~ s/^(.*?\n)(.*)$/$2/sgi )
    {
        print "\"" . escape_string( $1 ) . "\" \\\n";
    }
    print "\"" . escape_string( $file_cont ) . "\";\n";
    print "const resource_hnd_t ${variable_name} = { RESOURCE_IN_MEMORY, $section_length, { .mem = { ${variable_name}_data }}};";
    print "\n";
}

sub escape_string( $escstring )
{
  my $escstring = shift;
  # Escape characters for C string
  $escstring =~ s/\\/\\\\/sgi; # backslash
  $escstring =~ s/\a/\\a/sgi;  # bell
  $escstring =~ s/\x8/\\b/sgi; # backspace
  $escstring =~ s/\f/\\f/sgi;  # formfeed
  $escstring =~ s/\n/\\n/sgi;  # linefeed
  $escstring =~ s/\r/\\r/sgi;  # carriage return
  $escstring =~ s/\t/\\t/sgi;  # tab
  $escstring =~ s/\xB/\\v/sgi; # vertical tab
  $escstring =~ s/\'/\\'/sgi;  # single quote
  $escstring =~ s/\"/\\"/sgi;  # double quote
  return $escstring;
}
