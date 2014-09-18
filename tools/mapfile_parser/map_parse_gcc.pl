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
    print "Usage ./map_parse.pl [-a] [-w] <map file filename>";
    print "           -a = show all sections";
    print "           -w = show more warnings";
    exit;
}

#command line parameters
$printall = 0;
$warn = 0;
foreach (@ARGV)
{
    if ( $_ eq "-a" )
    {
        $printall = 1;
    }
    elsif ( $_ eq "-w" )
    {
        $warn = 1;
    }
    else
    {
        $filename = $_;
    }
}

#open the file
open INFILE, $filename or die "cant open " . $filename;
@file = <INFILE>;
close INFILE;

$file_cont = join('',@file);

#Get the start of FLASH and ram

$file_cont =~ m/.*Memory Configuration.*Attributes(.*)Linker script and memory map.*/sgi;

$mem_config = $1;

my $all_ram;
$all_ram =1;

#Capture all flash sections (look for xr attribute, without w, to signify flash)
while( $mem_config =~ m/(\S*)\s+0x(\S*)\s+0x(\S*)\s+xr[^w]/sgi )
{
    push(@flash_section_name,$1);
    push(@start_flash_section, hex($2));
    push(@length_flash_section, hex($3));
    $all_ram =0;
}
#Capture all ram sections (look for xrw attribute to signify ram)
while( $mem_config =~ m/(\S*)\s+0x(\S*)\s+0x(\S*)\s+xrw/sgi )
{
    push(@ram_section_name,$1);
    push(@start_ram, hex($2));
    push(@length_ram, hex($3));
}

#remove stuff above the memory map
$file_cont =~ s/.*Linker script and memory map(.*)/$1/sgi;

#remove stuff below the memory map
$file_cont =~ s/(.*)Cross Reference Table.*/$1/sgi;


our $network;

if ( $file_cont =~ m/\/NetX_Duo.a/si )
{
    $network = "NetX_Duo";
}
elsif ( $file_cont =~ m/\/NetX.a/si )
{
    $network = "NetX";
}
elsif ( $file_cont =~ m/\/LwIP.a/si )
{
    $network = "LwIP";
}
else
{
    $network = "Unknown";
}


%module_totals = ( );

$total_flash = 0;
$total_ram = 0;
$total_other = 0;

$max_flash = 0;
$max_ram = 0;
$max_other = 0;

if ( $printall == 1 )
{
    print "Memory Area, Module, Name, Size, Address, Decimal Address, Filename, See bottom for collated totals\n";
}

$ram_known_diff = 0;
$flash_known_diff = 0;
$other_known_diff = 0;

while ( $file_cont =~ m/\n [\.\*]?(\S+?)\*?\s+0x(\S+)\s+0x(\S+)\s+(\S+)/gi )
{
    $section = $1;
    $hexaddress = $2;
    $decaddress = hex($2);

    $size = hex($3);
    $modulefn = $4;
    if   ( ( $section ne "debug_info" ) &&
         ( $section ne "debug_macinfo" ) &&
         ( $section ne "debug_str" ) &&
         ( $section ne "debug_line" ) &&
         ( $section ne "debug_loc" ) &&
         ( $section ne "debug_frame" ) &&
         ( $section ne "debug_abbrev" ) &&
         ( $section ne "debug_pubnames" ) &&
         ( $section ne "debug_aranges" ) &&
         ( $section ne "ARM.attributes" ) &&
         ( $section ne "comment" ) &&
         ( $section ne "debug_ranges" ) &&
         ( $section ne "debug_pubtypes" ) &&
         ( $size != 0 ) &&
         ( $decaddress != 0 ) )
     {
        if( $all_ram == 1 )
        {
            $area = "RAM";
            $total_ram += $size;
        }
        else
        {
            my $loop_index=0;
            foreach $flash_section_name (@flash_section_name)
            {
                $start_flash_section =  @start_flash_section[$loop_index];
                $length_flash_section = @length_flash_section[$loop_index];
                $end_flash_section = $start_flash_section + $length_flash_section;
                if( ( $decaddress >= $start_flash_section) && ( $decaddress < $end_flash_section) )
                {
                    $area = "FLASH";
                    $total_flash += $size;
                }
                $loop_index++;
            }
            my $loop_index=0;
            foreach $ram_section_name (@ram_section_name)
            {
                $start_ram = @start_ram[$loop_index];
                $length_ram = @length_ram[$loop_index];
                $end_ram = $start_ram + $length_ram;
                if( ( $decaddress >= $start_ram) && ( $decaddress < $end_ram) )
                {
                    $area = "RAM";
                    $total_ram += $size;
                }
                $loop_index++;
            }
        }
        $module = process_section( $area, $section, $size, $modulefn, $decaddress, $hexaddress );
     }
}

# Process sections that are in both flash and ram due to having inital values load from flash into ram variables
while ( $file_cont =~ m/\n.(\S+)\s+0x(\S+)\s+0x(\S+) load address 0x(\S+)/gi )
{
#.data           0x20000000      0xc10 load address 0x08054af0
    $area = "FLASH";
    $section = "RAM Initialisation - $1";
    $size = hex($3);
    $hexaddress = $4;
    $decaddress = hex($4);
    $modulefn = "";

    if ( $decaddress < $start_flash + $length_flash)
    {
        $total_flash += $size;

        if ( $max_flash < $decaddress + $size )
        {
            $max_flash = $decaddress + $size;
        }

        if ( $warn && ( ( $total_flash != ($max_flash - $start_flash) ) && ( $flash_known_diff != (($max_flash - $start_flash) - $total_flash)) ) )
        {
            $flash_known_diff = ($max_flash - $start_flash) - $total_flash;
            print "WARNING: FLASH Max mismatch @ 0x$hexaddress. Max Flash = " . ($max_flash - $start_flash) . ". Total Flash = $total_flash. \n";
        }
    }

    if ( $size != 0 )
    {
#        print "section $section size $size address 0x$hexaddress\n";
        $module = process_section( $area, $section, $size, $modulefn, $decaddress, $hexaddress );
    }
}


if ( $printall == 1 )
{
    print "\n";
}

# Get target name from map filename

$filename       =~ m/^(.*)\.map/;
my $target_name = $filename;
$target_name    =~ s/^(.*)\.map/$1/i;
$target_name    =~ s/.*\/(.*)$/$1/i;
print "$target_name\n";


print_module_totals( );


sub print_sections
{
    print "Memory Area, Section Name, Address (Hex), Address (decimal), Size (bytes), Module filename\n";
    foreach $module (sort keys %module_totals)
    {
           print "$module, $module_totals{$module}\n";
    }
    print "$area, $section , 0x$hexaddress , $decaddress, $size, $module, $modulefn\n";
}


sub process_section( $area, $section, $size, $modulefn, $decaddress, $hexaddress )
{
    $module = "Other";

    if ( ( $section eq "rodata.wifi_firmware_image_data" ) ||
            ( $section eq "rodata.wifi_firmware_image_size" ) )
    {
        $module = "Wi-Fi Firmware";
    }
    elsif ( $section =~ m/^rodata.resources/sgi )
    {
        $module = "resources";
    }
    elsif ( $modulefn =~ m/\/App_\S+\.a/sgi )
    {
        $module = "App";
    }
    elsif ( $modulefn =~ m/platform_vector_table/sgi ||
            $modulefn =~ m/\/platform_isr\.o/sgi ||
            $modulefn =~ m/\/platform_unhandler_isr\.o/sgi ||
            $modulefn =~ m/\/hardfault_handler\.o/sgi )
    {
        $module = "Interrupt Vectors";
    }
    elsif ( $modulefn =~ m/\/ThreadX\.a/sgi ||
            $modulefn =~ m/\/ThreadX-\d.\d.\w+\.a/sgi )
    {
        $module = "ThreadX";
    }
    elsif ( $modulefn =~ m/\/SPI_Flash_Library/sgi )
    {
        $module = "SPI Flash Library";
    }
    elsif ( $modulefn =~ m/\/STM32F1xx_lib\.a/sgi ||
            $modulefn =~ m/\/STM32F1xx_Drv\.a/sgi ||
            $modulefn =~ m/\/STM32F2xx\.a/sgi ||
            $modulefn =~ m/\/STM32F2xx_Peripheral_Drivers\.a/sgi ||
            $modulefn =~ m/\/STM32F2xx_StdPeriph_Libraries\.a/sgi ||
            $modulefn =~ m/\/STM32F2xx_Peripheral_Libraries\.a/sgi ||
            $modulefn =~ m/\/STM32F4xx\.a/sgi ||
            $modulefn =~ m/\/STM32F4xx_Peripheral_Drivers\.a/sgi ||
            $modulefn =~ m/\/STM32F4xx_StdPeriph_Libraries\.a/sgi ||
            $modulefn =~ m/\/STM32F4xx_Peripheral_Libraries\.a/sgi ||
            $modulefn =~ m/\/BCM439x\.a/sgi ||
            $modulefn =~ m/\/BCM439x_Peripheral_Drivers\.a/sgi ||
            $modulefn =~ m/\/BCM4390A1_ROM\.a/sgi ||
            $modulefn =~ m/\/ASF\.a/sgi ||
            $modulefn =~ m/\/SAM4S\.a/sgi ||
            $modulefn =~ m/\/K60_?D?r?v?\.a/sgi ||
            $modulefn =~ m/\/LPC17xx\.a/sgi ||
            $modulefn =~ m/\/LPC17xx_Peripheral_Drivers\.a/sgi ||
            $modulefn =~ m/\/LPC17xx_StdPeriph_Libraries\.a/sgi)
    {
        $module = "Host MCU-family library";
    }
    elsif ( $section =~ m/\.app_thread_stack/sgi )
    {
        $module = "App Stack";
    }
    elsif ( $modulefn =~ m/arm-none-eabi/sgi ||
            $modulefn =~ m/[\\\/]libc\.a/sgi ||
            $modulefn =~ m/[\\\/]libgcc\.a/sgi ||
            $modulefn =~ m/[\\\/]libm\.a/sgi ||
            $modulefn =~ m/[\\\/]common_GCC\.a/sgi ||
            $modulefn =~ m/\/common\/GCC\/\S+\.o/sgi)
    {
        $module = "libc";
    }
    elsif ( $modulefn =~ m/\/WICED\.a/sgi ||
            $modulefn =~ m/\/Wiced_(ThreadX|FreeRTOS)_Interface\.a/sgi ||
            $modulefn =~ m/\/Wiced_\w+\_Interface_\w+\.a/sgi ||
            $modulefn =~ m/\/Wiced_Network_\w+_\w+\.a/sgi )
    {
        $module = "WICED";
    }
    elsif ( $modulefn =~ m/\w+_Interface_(SPI|SDIO)\.a/sgi ||
            $modulefn =~ m/\/WWD_for_(SPI|SDIO|SoC_4390)_\w+\.a/sgi ||
            $modulefn =~ m/\/WWD_\w+\_Interface\.a/sgi ||
            $modulefn =~ m/\/WWD_\w+\_Interface_\w+\.a/sgi )
    {
        $module = "WWD";
    }
    elsif ( $modulefn =~ m/\/crt0_gcc\.o/sgi ||
            $modulefn =~ m/\/Platform_\S+\.a/sgi )
    {
        $module = "platform";
    }
    elsif ( $modulefn =~ m/\/Lib_(.+)\.a/sgi )
    {
        $module = $1;
    }
    elsif ( $modulefn =~ m/\/libraries\/.+\/([^.]+)\.[^\/]*\.a/sgi )
    {
        $module = $1;
    }
    elsif ( $modulefn =~ m/\/resources\.a/sgi )
    {
        $module = "resources";
    }
    elsif ( $modulefn =~ m/\/Supplicant_uSSL\.a/sgi ||
            $modulefn =~ m/\/Supplicant_uSSL.\w+\.a/sgi ||
            $modulefn =~ m/\/uSSL\.\w+\.\w+\.\w+\.\w+\.a/sgi )
    {
        $module = "Supplicant - SSL/TLS";
    }
    elsif ( $modulefn =~ m/\/Supplicant_besl\.a/sgi ||
            $modulefn =~ m/\/Supplicant_besl.\w+\.a/sgi ||
            $modulefn =~ m/\/BESL\.\w+\.\w+\.a/sgi )
    {
        $module = "Supplicant - BESL";
    }
    elsif ( ( $section eq  "rodata.vars" ) ||
            ( $section eq "rodata.wifi_nvram_image" ) )
    {
        $module = "NVRam";
    }
    elsif ( $section eq  "fill" )
    {
        $module = "Startup Stack & Link Script fill";
    }
    elsif ( ( $section =~ m/.*tx_buffer_pool_memory/ ) ||
         ( $section =~ m/.*rx_buffer_pool_memory/ ) ||
         ( $section =~ m/.*memp_memory/ ) )
    {
        $module = "Packet Buffers";
    }
    elsif ( $section =~ m/.*xHeap/ )
    {
        $module = "FreeRTOS heap (inc Stacks & Semaphores)";
    }
    elsif ( $section =~ m/.*xHeap/ )
    {
        $module = "FreeRTOS heap (inc Stacks & Semaphores)";
    }
    elsif ( $section =~ m/.*lwip_stats/ )
    {
        $module = "LwIP stats";
    }
    elsif ( $section =~ m/.*ram_heap/ )
    {
        $module = "LwIP Heap";
    }
    elsif ( $modulefn =~ m/.*app_header\.o/sgi )
    {
        $module = "Bootloader";
    }
    elsif ( $modulefn =~ m/\/Gedday\.\w+\.\w+\.\w+\.\w+\.a/sgi )
    {
        $module = "Gedday";
    }
    elsif ( $modulefn =~ m/\/NetX\.a/sgi ||
         $modulefn =~ m/\/NetX.\w+\.a/sgi )
    {
        $module = "NetX";
    }
    elsif ( $modulefn =~ m/\/NetX_Duo\.a/sgi ||
            $modulefn =~ m/\/NetX_Duo-\d.\d.\w+\.a/sgi )
   {
       $module = "NetX-Duo - Code";
   }
   elsif ( ($network eq "NetX_Duo" ) && ( $section =~ m/\.wiced_ip_handle/sgi ) )
   {
       $module = "NetX-Duo - Interfaces & Stacks";
   }
   elsif ( $section =~ m/RAM Initialisation/sgi )
   {
       $module = "RAM Initialisation";
   }
    elsif ( $modulefn =~ m/\/LwIP\.a/sgi )
    {
        $module = "LwIP";
    }
    elsif ( $modulefn =~ m/\/FreeRTOS\.a/sgi )
    {
        $module = "FreeRTOS";
    }
    elsif ( $modulefn =~ m/NoOS\.a/sgi )
    {
        $module = "NoOS";
    }
    elsif ( $modulefn =~ m/\/Wiced_(NetX|NetX_Duo|LwIP)_Interface\.a/sgi )
    {
        $module = "Networking";
    }
    elsif ( $modulefn =~ m/\/resources\.a/sgi )
    {
        $module = "Resources";
    }
    elsif ( $modulefn =~ m/\/Nfc\.a/sgi )
    {
        $module = "NFC";
    }
    #else
    #{
    #    print "++$area, $section, $size, $modulefn, $decaddress, $hexaddress\n";
    #}

    if ( $printall == 1 )
    {
        print "$area,$module, $section, $size, 0x$hexaddress, $decaddress, $modulefn\n";
    }

    $module_totals{"$module"}{"$area"} += $size;

    return $module;


}

sub print_module_totals( )
{
    if ( $all_ram != 1 )
    {
        if ( $printall == 1 )
        {
            print "----------------------------------,--------,---------\n";
            print "                                  ,        ,  Static \n";
            print "Module                            , Flash  ,   RAM   \n";
            print "----------------------------------,--------,---------\n";
            foreach $module (sort {"\L$a" cmp "\L$b"} keys %module_totals)
            {
                   print sprintf("%-34.34s, %7d, %7d\n", $module, $module_totals{$module}{'FLASH'},  $module_totals{$module}{'RAM'});
            }
            print         "----------------------------------,--------,---------\n";
            print sprintf("TOTAL (bytes)                     , %7d, %7d\n", $total_flash,  $total_ram);
            print         "----------------------------------,--------,---------\n";
        }
        else
        {
            print "----------------------------------|---------|---------|\n";
            print "                                  |         |  Static |\n";
            print "              Module              |  Flash  |   RAM   |\n";
            print "----------------------------------+---------+---------|\n";
            foreach $module (sort {"\L$a" cmp "\L$b"} keys %module_totals)
            {
                   print sprintf("%-34.34s| %7d | %7d |\n", $module, $module_totals{$module}{'FLASH'},  $module_totals{$module}{'RAM'});
            }
            print         "----------------------------------+---------+---------|\n";
            print sprintf("TOTAL (bytes)                     | %7d | %7d |\n", $total_flash,  $total_ram);
            print         "----------------------------------|---------|---------|\n";
        }
    }
    else
    {
        if ( $printall == 1 )
        {
            print "----------------------------------,---------\n";
            print "                                  ,  Static \n";
            print "Module                            ,   RAM   \n";
            print "----------------------------------,---------\n";
            foreach $module (sort {"\L$a" cmp "\L$b"} keys %module_totals)
            {
                   print sprintf("%-34.34s, %7d\n", $module, $module_totals{$module}{'RAM'});
            }
            print         "----------------------------------,---------\n";
            print sprintf("TOTAL (bytes)                     , %7d\n", $total_ram);
            print         "----------------------------------,---------\n";
        }
        else
        {
            print "----------------------------------|---------|\n";
            print "                                  |  Static |\n";
            print "              Module              |   RAM   |\n";
            print "----------------------------------+---------|\n";
            foreach $module (sort {"\L$a" cmp "\L$b"} keys %module_totals)
            {
                   print sprintf("%-34.34s| %7d |\n", $module,  $module_totals{$module}{'RAM'});
            }
            print         "----------------------------------+---------|\n";
            print sprintf("TOTAL (bytes)                     | %7d |\n", $total_ram);
            print         "----------------------------------|---------|\n";
        }
    }
    if ( $total_other != 0)
    {
        print "WARNING: $total_other bytes unaccounted for (neither FLASH or RAM)\n";
    }
    print "\n"
}


