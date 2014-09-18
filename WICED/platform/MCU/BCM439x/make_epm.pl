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

my ($ch1, $ch2, $ch3, $ch4) = unpack("CCCC", <$in_file>);
if ( ( $ch1 != 0x7f ) ||
     ( $ch2 != ord('E') ) ||
     ( $ch3 != ord('L') ) ||
     ( $ch4 != ord('F') ) )
{
    print "$ARGV[0] is not an ELF file.";
    exit -1;
}

# parse the input Elf file
my %elf = parse_elf( $in_file, 0 );

# Find the sections marked as ProgBits + Alloc + ExecInstr
# Ensure they are contiguous and save the first address and total size

my @progbits_sections = ();
my $prev_section_end = -1;
my $total_size = 0;
my $first_address = -1;
for ( my $section_no = 0; $section_no < $elf{"section_header_count"}; $section_no++ )
{
#    print "section $section_no\n";
    if ( ( $elf{"section_headers"}->[$section_no]->{"section_type"} == 0x1 ) &&
         ( $elf{"section_headers"}->[$section_no]->{"section_flags"} & 0x2 ) &&
         ( $elf{"section_headers"}->[$section_no]->{"section_flags"} & 0x4 ) )
    {
        push ( @progbits_sections, $section_no );
#        printf "type %d\n", $elf{"section_headers"}->[$section_no]->{"section_type"};

        if ( ( $prev_section_end != -1 ) &&
             ( $prev_section_end != $elf{"section_headers"}->[$section_no]->{"section_address"} ) )
        {
            die( "Program sections not contiguous\n" );
        }
        if ( $first_address == -1 )
        {
            $first_address = $prev_section_end = $elf{"section_headers"}->[$section_no]->{"section_address"};
        }
        $prev_section_end = $elf{"section_headers"}->[$section_no]->{"section_address"} + $elf{"section_headers"}->[$section_no]->{"section_size"};
        $total_size += $elf{"section_headers"}->[$section_no]->{"section_size"};
#        printf "address 0x%08X\n", $elf{"section_headers"}->[$section_no]->{"section_address"};
#        printf "size 0x%08X\n", $elf{"section_headers"}->[$section_no]->{"section_size"};

    }
}


#printf "entry_pt                   0x%08X\n", $elf{"entry_point"};
#printf "program_header_offset      0x%08X\n", $elf{"program_header_offset"};
#print  "\n";





seek $out_file, 0, SEEK_SET;

print $out_file pack('cccc', ord('E'), ord('P'), ord('M'), ord('C') );  # Signature
print $out_file pack('C', 32);  # Header Length
print $out_file pack('C', 0);  # Flags - Don't check CRC
print $out_file pack('S', 0);  # Reserved
print $out_file pack('L', $first_address);  # image destination load address
print $out_file pack('L', $elf{"entry_point"} - $first_address);  # Jump offset
print $out_file pack('L', $total_size);  # Image length
print $out_file pack('L', 0);  # CRC value
print $out_file pack('LL', 0, 0);  # Padding


# Write app data

foreach $section ( @progbits_sections )
{
#    print "writing ". $elf{"section_headers"}->[$section]->{"section_size"} . " from offset ". $elf{"section_headers"}->[$section]->{"section_offset"} . "\n";
#    printf "    start 0x%08X end 0x%08X\n", $elf{"section_headers"}->[$section]->{"section_address"}, $elf{"section_headers"}->[$section]->{"section_size"} + $elf{"section_headers"}->[$section]->{"section_address"};
    seek $in_file, $elf{"section_headers"}->[$section]->{"section_offset"}, SEEK_SET;
    read $in_file, $file_data, $elf{"section_headers"}->[$section]->{"section_size"};
#    print length( $file_data );
    print $out_file $file_data;
}


close($out_file);
close($in_file);
exit 0;












sub parse_elf( $file_handle, $verbose )
{
    my $file_handle = shift;
    my $verbose = shift;

    my %elf = ();

    seek $file_handle, 0, SEEK_SET;
    my ( $ch1, $ch2, $ch3, $ch4, $class, $byteorder, $hversion, $pad1, $pad2, $pad3, $filetype, $archtype, $fversion, $entry_point, $program_header_offset, $section_header_offset, $flags, $header_size, $program_header_entry_size, $program_header_count, $section_header_entry_size, $section_header_count, $string_section ) = unpack("CCCCCCCLLCSSLLLLLSSSSSS", <$file_handle>);

    if ( ( $ch1 != 0x7f ) ||
         ( $ch2 != ord('E') ) ||
         ( $ch3 != ord('L') ) ||
         ( $ch4 != ord('F') ) )
    {
        print "File is not an ELF file.";
        exit -1;
    }

    $elf{"class"} = $class;
    $elf{"byteorder"} = $byteorder;
    $elf{"hversion"} = $hversion;
    $elf{"filetype"} = $filetype;
    $elf{"archtype"} = $archtype;
    $elf{"fversion"} = $fversion;
    $elf{"entry_point"} = $entry_point;
    $elf{"program_header_offset"} = $program_header_offset;
    $elf{"section_header_offset"} = $section_header_offset;
    $elf{"flags"} = $flags;
    $elf{"header_size"} = $header_size;
    $elf{"program_header_entry_size"} = $program_header_entry_size;
    $elf{"program_header_count"} = $program_header_count;
    $elf{"section_header_entry_size"} = $section_header_entry_size;
    $elf{"section_header_count"} = $string_section;

    if ( $verbose )
    {
        print "ELF HEADER\n";
        print "-----------------------------------------\n";
        print "magic : $magic\n";
        print "class : $class\n";
        print "byteorder : $byteorder\n";
        print "hversion : $hversion\n";
        print "filetype : $filetype\n";
        print "archtype : $archtype\n";
        print "fversion : $fversion\n";
        printf "entry_point : 0x%08X\n", $entry_point;
        print "program_header_offset : $program_header_offset\n";
        print "section_header_offset : $section_header_offset\n";
        print "flags : $flags\n";
        print "header_size : $header_size\n";
        print "program_header_entry_size : $program_header_entry_size\n";
        print "program_header_count : $program_header_count\n";
        print "section_header_entry_size : $section_header_entry_size\n";
        print "section_header_count : $section_header_count\n";
        print "string_section : $string_section\n";
        print "-----------------------------------------\n";
    }

    my @section_headers = ();

    my $string_table_location = -1;

    for ( my $section_no = 0; $section_no < $section_header_count; $section_no++ )
    {
        seek $file_handle, $section_header_offset + $section_no * $section_header_entry_size, SEEK_SET;
        my ( $section_name, $section_type, $section_flags, $section_address, $section_offset, $section_size, $section_link, $section_info, $section_align, $section_entsize ) = unpack("LLLLLLLLLL", <$file_handle>);

        my %section = ();
        $section{"section_name_index"} = $section_name;
        $section{"section_type"} = $section_type;
        $section{"section_flags"} = $section_flags;
        $section{"section_address"} = $section_address;
        $section{"section_offset"} = $section_offset;
        $section{"section_size"} = $section_size;
        $section{"section_link"} = $section_link;
        $section{"section_info"} = $section_info;
        $section{"section_align"} = $section_align;
        $section{"section_entsize"} = $section_entsize;
        $section{"section_type_description"} = (($section_type == 0x0)?"Null":
                                                 ($section_type == 0x1)?"ProgBits":
                                                 ($section_type == 0x2)?"Symbol Table":
                                                 ($section_type == 0x3)?"String Table":
                                                 ($section_type == 0x4)?"Rela":
                                                 ($section_type == 0x5)?"Hash":
                                                 ($section_type == 0x6)?"Dynamic":
                                                 ($section_type == 0x7)?"Note":
                                                 ($section_type == 0x8)?"NoBits":
                                                 ($section_type == 9)?"Rel":
                                                 ($section_type == 10)?"ShLib":
                                                 ($section_type == 11)?"DynLib": $section_type );
        $section{"section_flags_description"} = (($section_flags & 0x1)?"Write ":"") . (($section_flags & 0x2)?"Alloc ":"") . (($section_flags & 0x4)?"ExecInstr ":"") . (($section_flags & 0xf0000000)?"MaskProc ":"");
        $section_headers[$section_no] = \%section;



        if ( ( $section_type == 0x3 ) && ( $string_table_location == -1 ) )
        {
            $string_table_location = $section_offset;
        }
    }
    $elf{"section_headers"} = \@section_headers;

    for ( my $section_no = 0; $section_no < $section_header_count; $section_no++ )
    {
        seek $file_handle, $string_table_location + $section_headers[$section_no]->{"section_name_index"}, SEEK_SET;
        my ( $string ) = unpack( "Z*", <$file_handle> );
        $section_headers[$section_no]->{"section_name"} = $string;

        if ( $verbose )
        {
            print "\n\nSection Header $section_no\n";
            print "-----------------------------------------\n";
            print "name : " . $section_headers[$section_no]->{"section_name"} . "\n";
            print "name index: ". $section_headers[$section_no]->{"section_name_index"} . "\n";
            print "type : " . $section_headers[$section_no]->{"section_type_description"} . "\n";
            print "flags : " . $section_headers[$section_no]->{"section_flags_description"}  . "\n";
            printf "Address : 0x%08X\n", $section_headers[$section_no]->{"section_address"};
            print "Offset : " . $section_headers[$section_no]->{"section_offset"} . "\n";
            print "Size : " . $section_headers[$section_no]->{"section_size"} . "\n";
            print "Link : " . $section_headers[$section_no]->{"section_link"} . "\n";
            print "Info : " . $section_headers[$section_no]->{"section_info"} . "\n";
            print "Align : " . $section_headers[$section_no]->{"section_align"} . "\n";
            print "Entry Size : " . $section_headers[$section_no]->{"section_entsize"} . "\n";
            print "-----------------------------------------\n";
        }
    }

    my @program_headers = ();

    for ( my $program_no = 0; $program_no < $program_header_count; $program_no++ )
    {
        seek $file_handle, $program_header_offset + $program_no * $program_header_entry_size, SEEK_SET;
        my ( $program_type, $program_offset, $program_virtual_address, $program_physical_address, $program_file_size, $program_memory_size, $program_flags, $program_align ) = unpack("LLLLLLLL", <$file_handle>);

        my %program = ();
        $program{"program_type"} = $program_type;
        $program{"program_offset"} = $program_offset;
        $program{"program_virtual_address"} = $program_virtual_address;
        $program{"program_physical_address"} = $program_physical_address;
        $program{"program_file_size"} = $program_file_size;
        $program{"program_memory_size"} = $program_memory_size;
        $program{"program_flags"} = $program_flags;
        $program{"program_align"} = $program_align;
        $program{"program_type_description"} = (($program_type == 0x0)?"Null":
                                                 ($program_type == 0x1)?"Load":
                                                 ($program_type == 0x2)?"Dynamic":
                                                 ($program_type == 0x3)?"Interp":
                                                 ($program_type == 0x4)?"Note":
                                                 ($program_type == 0x5)?"ShLib":
                                                 ($program_type == 0x6)?"PHdr": $program_type );
        $section_headers[$section_no] = \%program;

        if ( $verbose )
        {
            print "\n\nProgram Header $program_no\n";
            print "-----------------------------------------\n";
            print "type : " . $program{"program_type_description"} . "\n";
            print "offset : " . $program{"program_offset"} . "\n";
            print "flags : $program_flags\n";
            printf "Virtual Address : 0x%08X\n", $program_virtual_address;
            printf "Physical Address : 0x%08X\n", $program_physical_address;
            print "Size in file: $program_file_size\n";
            print "Size in memory: $program_memory_size\n";
            print "Align : $program_align\n";
            print "-----------------------------------------\n";
        }
    }

    $elf{"program_headers"} = \@program_headers;

    return %elf;
}