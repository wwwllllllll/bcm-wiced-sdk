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

if (! ($ARGV[0] &  $ARGV[1]))
{
    print "Usage ./nxp_checksum.pl <readelf output text file> <elf file> <vector section name>";
    exit;
}
#command line parameters
$printall = 0;
$warn = 0;

$read_txt_elf_filename = $ARGV[0];
$binary_filename = $ARGV[1];
$vector_section_name = $ARGV[2];

$BYTE0_POS=0;
$BYTE1_POS=8;
$BYTE2_POS=16;
$BYTE3_POS=24;


##################################################
# Find vector table offset
##################################################
open INFILE, $read_txt_elf_filename or die "cant open " . $read_txt_elf_filename;
@file = <INFILE>;
close INFILE;

$file_cont = join('',@file);
#Look for "Section Headers:"
$file_cont =~ m/Section Headers:.*\s$vector_section_name\s*PROGBITS\s*\S{8}\s\S{6}\s/s;
$section_table =$&;
#print sprintf("%s\n", $section_table);

#Isolate vector table entry
$section_table =~ m/\s$vector_section_name\s*PROGBITS\s*\S{8}\s\S{6}\s/s;
$vectors_line =$&;

#Find vector table offset
if($vectors_line =~ m/\s(\d|[A-F]){6}\s/i)
{
    $vector_offset =$&;
    #strip the whitespace
    $vector_offset=~ m/(\d|[A-F]){6}/i;
    $vector_offset =hex($&);
    $vector_offset=sprintf("%d",$vector_offset);
}
else
{
     print "ERROR: Could not find vector table offset";
     exit;
}

##################################################
# Calculate and insert checksum
##################################################
#open the file
open INFILE,'+<:raw', $binary_filename or die "cant open " . $binary_filename;
binmode(INFILE) || die "can't binmode $binary_filename";
#calculate checksum
seek INFILE, $vector_offset, SEEK_CUR or die "could not seek: $!";

#Read the first 7 interrupt vectors (@4 bytes each=28 bytes total)
for ( my $i = 0; $i < 28; $i++ )
{
          read INFILE, $data_array[$i], 1 or die "could not read: $!";
}

#unpack each vector and sum with previous vector value
for ( my $i = 0; $i < 7; $i++ )
{
     $current_vector=unpack('C',@data_array[4*$i])<<$BYTE0_POS | unpack('C',@data_array[4*$i+1])<<$BYTE1_POS | unpack('C',@data_array[4*$i+2])<<$BYTE2_POS | unpack('C',@data_array[4*$i+3])<<$BYTE3_POS;
     #print sprintf("Current Vector = %x\n",$current_vector);
     $checksum+=$current_vector;
     $checksum-=2**32 if $checksum >= 2**31;
}
$checksum=(0-$checksum);
$checksum-=2**32 if $checksum >= 2**31;
print sprintf("Calculated checksum: 0x%x\r\n",$checksum);
print INFILE pack('l',$checksum);
close INFILE;

