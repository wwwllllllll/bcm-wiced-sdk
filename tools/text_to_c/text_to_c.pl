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
    print "Usage ./text_to_c.pl  <variable name> <text file>";
    exit;
}

# Print start of output
$variable_name = shift @ARGV;
$original_variable_name = $variable_name;
$file = shift @ARGV;

#open the file
open INFILE, "<:raw", $file or die "cant open " . $file;
@file_cont_array = <INFILE>;
close INFILE;
$file_cont = join('',@file_cont_array);


if ( ( $file =~ m/\.html$/sgi ) ||
     ( $file =~ m/\.txt$/sgi ) )
{
  while ( $file_cont =~ s/^(.*?)\r?\n?\<WICED\:section\s+suffix=\"(\S+)\"\s*\/\>\r?\n?(.*)$/$3/sgi )
  {
    my $substr = $1;
    my $variable_suffix = $2;
    print "const char ${variable_name}[" . (length( $substr )+1) . "] = ";
    while ( $substr =~ s/^(.*?\n)(.*)$/$2/sgi )
    {
        print "\"" . escape_string( $1 ) . "\" \\\n";
    }
    print "\"" . escape_string( $substr ) . "\";\n\n";
    $variable_name = $original_variable_name . $variable_suffix;
  }
}



print "const char ${variable_name}[" . (length( $file_cont )+1) . "] = ";
while ( $file_cont =~ s/^(.*?\n)(.*)$/$2/sgi )
{
    print "\"" . escape_string( $1 ) . "\" \\\n";
}
print "\"" . escape_string( $file_cont ) . "\";\n";




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
