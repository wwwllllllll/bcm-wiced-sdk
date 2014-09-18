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
# Adds an EPM (External Program Memory) header to a program
# ELF file, to allow it to be booted from serial flash
#
# EPM format:
#    UINT32 signature;      // "EPM header valid" signature = 'E','P','M','C'
#    UINT8  hdrLen;         // EPM header length
#    UINT8  flags;          // indicates which optional fields are valid  (1=FLAG_CRC)
#    UINT16 rsvd2;          // reserved, to ensure 4-Byte alignment
#    UINT32 *ldAddr;        // EPM FW image destination load address
#    UINT32 jmpOff;         // EPM FW image jump offset
#    UINT32 imgLen;         // EPM FW image length
#    UINT32 imgChk;         // EPM FW image check value (optional)
#    UINT32 pad1;
#    UINT32 pad2;
#
#
# ELF file header
#    uint32_t magic;  /* should be 0x7f 'E' 'L' 'F' */
#    uint8_t  elfclass;
#    uint8_t  data;
#    uint8_t  version;
#    uint8_t  reserved[9];
#    uint16_t type;
#    uint16_t machine;
#    uint32_t version;
#    uint32_t entry;
#    uint32_t program_header_offset; /* from start of file */
#    uint32_t section_header_table_offset; /* from start of file */
#    uint32_t flags;
#    uint16_t elf_header_size;
#    uint16_t program_header_entry_size;
#    uint16_t program_header_entry_count;
#    uint16_t section_header_entry_size;
#    uint16_t section_header_entry_count;
#    uint16_t string_table_index; /* index in the section header table */
#
# ELF Program Table Entry
#    uint32_t type;
#    uint32_t data_offset;
#    uint32_t virtual_address;
#    uint32_t physical_address;
#    uint32_t data_size_in_file;
#    uint32_t data_size_in_memory;
#    uint32_t flags;
#    uint32_t alignment;
#


if (! $ARGV[1] )
{
    print "Usage perl $0 <program_elf_name> <output_filename>";
    exit -1;
}

open(my $in_file,  '<:raw', $ARGV[0]) or die "Unable to open: $!";
open(my $out_file, '>:raw', $ARGV[1]) or die "Unable to open: $!";

seek $out_file, 32, SEEK_SET;

my ($ch1, $ch2, $ch3, $ch4) = unpack("CCCC", <$in_file>);
if ( ( $ch1 != 0x7f ) ||
     ( $ch2 != ord('E') ) ||
     ( $ch3 != ord('L') ) ||
     ( $ch4 != ord('F') ) )
{
    print "$ARGV[0] is not an ELF file.";
    exit -1;
}

# Read program entry point and program table details from ELF file header
seek $in_file, 24, SEEK_SET;
my ( $entry_pt, $program_header_offset ) = unpack("LL", <$in_file>);

seek $in_file, 42, SEEK_SET;
my ($program_header_entry_size, $program_header_entry_count)= unpack("SS", <$in_file>);


#printf "entry_pt                   0x%08X\n", $entry_pt;
#printf "program_header_offset      0x%08X\n", $program_header_offset;
#print  "program_header_entry_size  $program_header_entry_size\n";
#print  "program_header_entry_count $program_header_entry_count\n";
#print  "\n";
#print  "Program Table:\n";

my $next_address = 0;
my $start_address = 0;
my $total_size = 0;
my $file_data;
my $padding_needed = 0;

for ( my $entry_no = 0; $entry_no < $program_header_entry_count; $entry_no++ )
{
    seek $in_file, $program_header_offset + $entry_no * $program_header_entry_size, SEEK_SET;
    my ( $type, $data_offset, $virtual_address, $physical_address, $data_size_in_file, $data_size_in_memory, $flags, $alignment )
         = unpack( "LLLLLLLL", <$in_file> );

    if ( ( $data_size_in_file == 0 ) || ( $type & 0x01 == 0 ) )
    {
#        print "Non-loadable entry\n";
        next;
    }
    if ( $entry_no == 0 )
    {
        $start_address = $physical_address;
    }
    if ( ( $entry_no != 0 ) && ( $next_address > $physical_address ) )
    {
        print  "Error: Discontiguous ELF file - cannot convert to EPM binary\n";
        printf "       End of previous loadable entry: 0x%08X\n", $next_address;
        printf "       Start of entry %02d             : 0x%08X\n", $entry_no, $physical_address;
        exit -1;
    }

    #print "Padding with $padding_needed nulls \n";
    $total_size += $padding_needed;
    while ( $padding_needed > 0 )
    {
        print $out_file chr(0);
        $padding_needed--;
    }


    #printf "Program entry size 0x%08X, load loc 0x%08X\n", $data_size_in_file,  $physical_address;

    seek $in_file, $data_offset, SEEK_SET;
    read $in_file, $file_data, $data_size_in_file;
    print $out_file $file_data;






    $padding_needed = $data_size_in_memory - $data_size_in_file;
    $total_size += $data_size_in_file;
    $next_address = $physical_address + $data_size_in_file;
}




close($in_file);


seek $out_file, 0, SEEK_SET;

print $out_file pack('cccc', ord('E'), ord('P'), ord('M'), ord('C') );  # Signature
print $out_file pack('C', 32);  # Header Length
print $out_file pack('C', 0);  # Flags - Don't check CRC
print $out_file pack('S', 0);  # Reserved
print $out_file pack('L', $start_address);  # image destination load address
print $out_file pack('L', $entry_pt - $start_address);  # Jump offset
print $out_file pack('L', $total_size);  # Image length
print $out_file pack('L', 0);  # CRC value
print $out_file pack('LL', 0, 0);  # Padding


close($out_file);

exit 0;






