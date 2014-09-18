#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
###############################################################################
#
# sflash_write.tcl
#
# This TCL OpenOCD script runs on a PC and communicates with the embedded
# sflash_write app to allow the PC to write data into a serial flash chip
# attached to the target processor
#
# Usage example:
#
# source [find mfg_spi_flash/write_sflash.tcl]
# sflash_init "BCMUSI11-SDIO-debug"
# sflash_write_file "example.bin" 0x10 "BCMUSI11-SDIO-debug" 1
# shutdown
#
###############################################################################

# CHIP_RAM_START must be supplied by target specific TCL script
set MemoryStart $CHIP_RAM_START

###############################################################################
#
# These variables must match the ones in Apps/waf/sflash_write/sflash_write.c
#
# They rely on the linker script placing the data_config_area_t and
# data_transfer_area_t structures at the start of RAM
#
###############################################################################

# This must match data_config_area_t
set entry_address_loc  [expr $MemoryStart + 0x00 ]
set stack_address_loc  [expr $MemoryStart + 0x04 ]
set buffer_size_loc    [expr $MemoryStart + 0x08 ]

# This must match data_transfer_area_t
set data_size_loc      [expr $MemoryStart + 0x0C ]
set dest_address_loc   [expr $MemoryStart + 0x10 ]
set command_loc        [expr $MemoryStart + 0x14 ]
set result_loc         [expr $MemoryStart + 0x18 ]
set data_loc           [expr $MemoryStart + 0x1C ]


# These must match the MFG_SPI_FLASH_COMMAND defines
set COMMAND_INITIAL_VERIFY            (0x01)
set COMMAND_ERASE                     (0x02)
set COMMAND_WRITE                     (0x04)
set COMMAND_POST_WRITE_VERIFY         (0x08)
set COMMAND_VERIFY_CHIP_ERASURE       (0x10)
set COMMAND_WRITE_DCT                 (0x20)
set COMMAND_READ                      (0x40)
set COMMAND_WRITE_ERASE_IF_NEEDED     (0x80)

# These must match the mfg_spi_flash_result_t enum
set RESULT(0xffffffff)      "In Progress"
set RESULT(4294967295)      "In Progress"
set RESULT(0)               "OK"
set RESULT(1)               "Erase Failed"
set RESULT(2)               "Verify after write failed"
set RESULT(3)               "Size too big for buffer"
set RESULT(4)               "Size too big for chip"
set RESULT(5)               "DCT location not found - has factory reset app been written?"
set RESULT(6)               "Error during Write"
set RESULT(7)               "Error during Read"


###############################################################################
# memread32
#
# Helper function that reads a 32 bit value from RAM and returns it
#
# address - the RAM address to read from
###############################################################################
proc memread32 {address} {
    mem2array memar 32 $address 1
    return $memar(0)
}

###############################################################################
# load_image_bin
#
# Loads part of a binary file into RAM
#
# fname   - filename of binary image file
# foffset - offset from the start of the binary file where data will be read
# address - the destination RAM address
# length  - number of bytes to transfer
###############################################################################
proc load_image_bin {fname foffset address length } {
  # Load data from fname filename at foffset offset to
  # target at address. Load at most length bytes.
  puts "loadimage address $address foffset $foffset $length"
  load_image $fname [expr $address - $foffset] bin $address $length
}



###############################################################################
# sflash_init
#
# Prepares for writing to serial flashby loading the sflash_write app into
# memory and setting it running.
# This function assumes the following target has already been built:
#      waf_sflash_write-NoOS-NoNS-<PlatBusDebug>
#
# PlatBusDebug   - The platform, bus and debug part of the build target
# init4390       - run initialisation for the 4390
###############################################################################
proc sflash_init { PlatBusDebug init4390 } {
    global entry_address_loc
    global stack_address_loc
    global buffer_size_loc
    global entry_address
    global stack_address
    global buffer_size

    init

    if { $init4390 == 1 } {
        post_init_setup
    }

  #  reset halt

 #   halt
    load_image build/waf_sflash_write-NoOS-NoNS-$PlatBusDebug/binary/waf_sflash_write-NoOS-NoNS-$PlatBusDebug.elf

    #mdw 0x000D0000 100
    #mdw $entry_address_loc 100

    set entry_address [memread32 $entry_address_loc]
    set stack_address [memread32 $stack_address_loc ]
    set buffer_size   [memread32 $buffer_size_loc]

    puts "entry_address= $entry_address"
    puts "stack_address= $stack_address"
    puts "buffer_size= $buffer_size"
    if { $buffer_size == 0 } {
        puts "Error: Buffer size read from address $buffer_size_loc on target is zero"
        exit -1;
    }


    # Setup start address
    reg pc $entry_address

    resume

}


###############################################################################
# program_sflash
#
# Executes a serial command by communicating to the sflash_write app
#
# fname    - filename of binary image file (if command requires it)
# foffset  - offset from the start of the binary file where data will be read  (if command requires it)
# dataSize - number of bytes to transfer (if command requires it)
# destaddr - the destination serial flash address (if command requires it)
# cmd      - The commmand to execute (see list above)
###############################################################################
proc program_sflash { filename foffset dataSize destaddr cmd } {
    global PlatBusDebug MemoryStart data_size_loc data_loc dest_address_loc command_loc result_loc entry_address RESULT  entry_address_loc

    # Load the binary data into the RAM
    if { ( $dataSize != 0 ) && ( $filename != "" ) } {
       load_image_bin $filename $foffset $data_loc $dataSize
    }

    # Write the details of the data

    mww $data_size_loc    $dataSize
    mww $dest_address_loc $destaddr
    mww $result_loc       0xffffffff


    #mdw $entry_address_loc 100

    # Write the command - This causes the writing to start
    mww $command_loc      $cmd


    set loops  0
    while { ([memread32 $result_loc]  == 0xffffffff) && ( $loops < 1000 ) } {
     sleep 10
     incr loops
    }

    set resultval [memread32 $result_loc]
    puts "****************** Result: $RESULT($resultval)"

    if { $resultval != 0 } {
        exit -1;
    }

}


###############################################################################
# sflash_write_file
#
# Writes an entire binary image file to a serial flash address
# This function assumes the following target has already been built:
#      waf_sflash_write-NoOS-NoNS-<PlatBusDebug>
#
# filename     - filename of binary image file
# destAddress  - the destination serial flash address
# PlatBusDebug - The platform, bus and debug part of the build target
# erasechip    - If 1, Erase the chip before writing.
# init4390     - run initialisation for the 4390
###############################################################################
proc sflash_write_file { filename destAddress PlatBusDebug erasechip init4390 } {
    global COMMAND_ERASE COMMAND_INITIAL_VERIFY COMMAND_WRITE COMMAND_POST_WRITE_VERIFY buffer_size COMMAND_WRITE_ERASE_IF_NEEDED

    sflash_init $PlatBusDebug $init4390

    set binDataSize [file size $filename]
    # set erase_command_val [expr $COMMAND_ERASE ]
    set write_command_val [expr $COMMAND_WRITE_ERASE_IF_NEEDED | $COMMAND_POST_WRITE_VERIFY ]
    set pos 0

    # if { $erasechip } {
        # puts "Erasing Chip"
        # program_sflash $filename $pos 0 $destAddress $erase_command_val
        # puts "Chip Erase Done"
    # }

    puts "Total write size is $binDataSize"
    while { $pos < $binDataSize } {
        if { ($binDataSize - $pos) <  $buffer_size } {
            set writesize [expr ($binDataSize - $pos)]
        } else {
            set writesize $buffer_size
        }
        puts "writing $writesize bytes at [expr $destAddress + $pos]"
        program_sflash $filename $pos $writesize [expr $destAddress + $pos] $write_command_val
        set pos [expr $pos + $writesize]
    }
}



###############################################################################
# sflash_read_file
#
# Reads data from a serial flash address
# This function assumes the following target has already been built:
#      waf_sflash_write-NoOS-NoNS-<PlatBusDebug>
#
# filename     - output filename for binary image file
# srcAddress   - the destination serial flash address
# PlatBusDebug - The platform, bus and debug part of the build target
# length       - number of bytes to read
# init4390     - run initialisation for the 4390
###############################################################################
proc sflash_read_file { filename srcAddress PlatBusDebug length init4390 } {
    global COMMAND_ERASE COMMAND_INITIAL_VERIFY COMMAND_WRITE COMMAND_POST_WRITE_VERIFY buffer_size COMMAND_READ data_loc

    sflash_init $PlatBusDebug $init4390
    set temp_file "temp.bin"
    exec tools/common/Win32/echo > $filename

    set read_command_val [expr $COMMAND_READ ]
    set pos 0

    puts "Total read size is $length"
    while { $pos < $length } {
        if { ($length - $pos) <  $buffer_size } {
            set readsize [expr ($length - $pos)]
        } else {
            set readsize $buffer_size
        }
        puts "reading $readsize bytes from [expr $srcAddress + $pos]"
        program_sflash "" $pos $readsize [expr $srcAddress + $pos] $read_command_val
#        mem2array memar 8 [expr $data_loc-8] 1024
#        puts "$memar(0) $memar(1) $memar(2) $memar(3) $memar(4) $memar(5) $memar(6) $memar(7) $memar(8)"
        puts "dumping image from $data_loc $readsize"
        dump_image  $temp_file $data_loc $readsize

        exec cat $temp_file >> $filename
        set pos [expr $pos + $readsize]
    }
}



proc sflash_erase { PlatBusDebug init4390 } {
    global COMMAND_ERASE COMMAND_INITIAL_VERIFY COMMAND_WRITE COMMAND_POST_WRITE_VERIFY buffer_size COMMAND_WRITE_ERASE_IF_NEEDED

    sflash_init $PlatBusDebug $init4390

    set erase_command_val [expr $COMMAND_ERASE ]

    puts "Erasing Chip"
    program_sflash "" 0 0 0 $erase_command_val
    puts "Chip Erase Done"
}


proc post_init_setup { } {

#mww 0xE000EDF0 0xa05f000b
#mww 0xE000EDF0 0xa05f000d
mww 0xE000EDF0 0xa05f0003

mww 0xE000ED0C 0x05FA0002

reset halt
halt
mww 0xE000EDF0 0xa05f0003

mww 0xE000ED0C 0x05FA0002

#request reset
#mww 0xE000ED0C 0x5FA0004
#step
#step

# Disable the Watchdog
# See section 4.4.1 of AMBA Design Kit Technical Reference Manual r3p0 - Watchdog Control Register
# http://infocenter.arm.com/help/topic/com.arm.doc.ddi0243c/DDI0243C_adk_r3p0_trm.pdf
mww 0x00329008 0

# Setup the SCB VTOR (Vector Table Offset Register) to point to our SRAM vector table
# this cannot be done in the code as there may already be pending interrupts.
#mww 0xE000ED08 [expr 0x00200000 | 0x20000000 ]
#mww 0xE000ED08 0x00216000
#mww 0xE000ED08 0x00740000
# Clear SysTick Control and Status Register
#mww 0xE000E010 0

# Clear interrupt enables using the Interrupt Clear-Enable Registers
for { set addr 0xE000E180 } { $addr <= 0xE000E19C } { incr $addr } {
    mww $addr 0xFFFFFFFF
}

# Clear pending interrupts using the Interrupt Clear-Pending Registers
for { set addr 0xE000E280 } { $addr <= 0xE000E29C } { incr $addr } {
    mww $addr 0xFFFFFFFF
}

# Clear Interrupt Control State Register
mww 0xE000ED04 0

# Clear active interrupt vector using Application Interrupt and Reset Control Register
set VECTKEY       0x05FA0000
set VECTCLRACTIVE 0x00000002
#mww 0xE000ED0C [expr $VECTKEY | $VECTCLRACTIVE ]
mww 0xe000ed24 0x70000

# Clear Configuration Control Register
mww 0xE000ED14 0x0

#clear RAM so that the rtos awareness does not detect threads left in memory from previous run
#for { set addr $CHIP_SRAM1_START } { $addr <= [expr $CHIP_SRAM1_START + $CHIP_SRAM1_SIZE] } { set addr [expr $addr + 0x100] } {
#puts "write $addr"
#    mww $addr 0x00000000 0x100
#}
#for { set addr $CHIP_SRAM2_START } { $addr <= [expr $CHIP_SRAM2_START + $CHIP_SRAM2_SIZE] } { set addr [expr $addr + 0x100] } {
#puts "write $addr"
#    mww $addr 0x00000000 0x100
#}
#mww 0x002008fc 0x00000000 88

# clear threadx locations
mww 0x00200cc8 0x0  12
mww 0x00203698 0x0  12
mww 0x00204630 0x0  1
mww 0x002076e0 0x0  212

# clear FreeRTOS locations
mww 0x00215b6c 0x0  80


#mww 0x00215B6C 0x0  18725



#mww 0xE000ED08 0x00216000
#mww $addr 0x00000000 0x1000

#Turn off LPO_CLK on SPI2_MOSI pin
mww 0x32150c 0x4


mww 0x320078 0x100
mww 0x32008c 0x00000



#reg sp 0x0200960
#reg sp 0x750970



#mww 0x320100 0xffffffff
#mww 0x32005c 0xffffffff
#mww 0x35002c 0xffffffff

#reset halt


# turn on debug message
#debug_level 3
# enable WLAN power
mww 0x0064015c 0x7

# Set flags to cause timer based peripherals to stop during breakpoints.
#mww 0xE0042008 0xffffffff

#jtag_khz 1000
#verify_ircapture disable

#Enable Timer1 interrupt #17
#mww 0xe000e10 0x20000

#cortex_m3 maskisr on

#Enable memory direct path
# cr_pds_ctl_adr mask bit18
mww 0x321520 0x18003e0
#wlan_mem_set_adr
mww 0x3201fc 0x45
#gci_gciindirectaddress 8
mww 0x650040 8
#Set last 256K RAM to Apps
mww 0x650200 0x30
#mww 0x640160 0x7
mww 0x3201fc 0xd5

adapter_khz 1000

}
