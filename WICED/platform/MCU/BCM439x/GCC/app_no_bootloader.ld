/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

reset_handler = _start;

ENTRY( _start );



MEMORY
{
    ROM         (rx)  : ORIGIN = 0x00000000, LENGTH = 576K
    SRAM_32K    (rwx) : ORIGIN = 0x000D0000, LENGTH = 32K
    SRAM_128k_1 (rwx) : ORIGIN = 0x00140000, LENGTH = 128K
    SRAM_128k_2 (rwx) : ORIGIN = 0x001E0000, LENGTH = 128K
    SRAM_160K   (rwx) : ORIGIN = 0x00200000, LENGTH = 160K
}

SECTIONS
{


    /* Data transfer area for serial flash writing app  - needs to be at start of memory */
    /* Only exists if building serial flash writer app */
    .sflash_trnsf :
    {
        KEEP(*(*.data_config))
        KEEP(*(*.data_transfer))
    }>SRAM_160K AT> SRAM_160K


    .vectors :
    {
        /* Vector table must be aligned: (from CM3 manual)
         * When setting TBLOFF, you must align the offset to the number of exception
         * entries in the vector table. The minimum alignment is 32 words, enough for
         * up to 16 interrupts. For more interrupts, adjust the alignment by rounding
         * up to the next power of two. For example, if you require 21 interrupts, the
         * alignment must be on a 64-word boundary because the required table size is 37
         * words, and the next power of two is 64. See your vendor documentation for
         * the alignment details for your device.
         */
        . = . +100;
        . = ALIGN(512);
        link_interrupt_vectors_location = .;
        KEEP(*(*.interrupt_vector_table))
    }>SRAM_32K  AT> SRAM_32K

    .entry_pt :
    {
        . = ALIGN(4);
      KEEP (*(.text._start))

    } >SRAM_32K AT>SRAM_32K

    .rodata :
    {
        . = ALIGN(0x4);

        link_const_variable_data_location = .;
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        link_const_variable_data_end = .;
    } >SRAM_128k_2 AT>SRAM_128k_2

    .text :
    {
        . = ALIGN(4);

        link_code_location = .;
        KEEP(*(.text.irq ))
        *(.text .text.* .gnu.linkonce.t.*)
        link_code_end = .;

        . = ALIGN(0x4);

        link_constructors_location = .;
        KEEP(*(.preinit_array))
        KEEP(*(.init_array))
        KEEP (*crtbegin.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*crtend.o(.ctors))
        link_constructors_end = .;

        . = ALIGN(0x4);

        link_destructors_location = .;
        KEEP (*crtbegin.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*crtend.o(.dtors))
        link_destructors_end = .;

        . = ALIGN(16);

    }>SRAM_32K  AT> SRAM_32K  OVERFLOW_INTO>.text_128k_area2


    .text_128k_area2 :
    {
    } >SRAM_128k_2 AT>SRAM_128k_2 OVERFLOW_INTO> .text_128k_area1


    .text_128k_area1 :
    {

    } >SRAM_128k_1 AT>SRAM_128k_1 OVERFLOW_INTO>.text_160k_area


    .text_160k_area :
    {
    } >SRAM_160K AT>SRAM_160K

    /*
     * The .ARM.exidx and .ARM.extab sections are used for C++ exception handling.
     * It is located here for completeness. Bare-metal ARM projects
     * typically cannot afford the overhead associated with C++
     * exceptions handling.
     */
    .ARM.exidx :
    {
        __exidx_start = ALIGN(4);
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > SRAM_160K  AT> SRAM_160K

    .ARM.extab :
    {
        __extab_start = ALIGN(4);
        *(.ARM.extab*)
        __extab_end = .;
    } > SRAM_160K  AT> SRAM_160K

    .fast : /* This section contains code that is run from RAM after being loaded from flash - functions can be put in this section with the C attribute: __attribute__ ((section (".fast"))) */
    {
        link_run_from_ram_code_flash_location = LOADADDR( .fast ); /* This is the location in flash of the code */
        link_run_from_ram_code_ram_location = .;
        *(.fast .fast.* .text.fastcode)
        link_run_from_ram_code_ram_end = .;
    }> SRAM_160K AT> SRAM_160K



    .bss : /* Zero initialised memory used for zero initialised variables */
    {
        link_bss_location = ALIGN(., 4);
        *(*pxCurrentTCB)
        *(*pxReadyTasksLists)
        *(*xDelayedTaskList1)
        *(*xDelayedTaskList2)
        *(*pxDelayedTaskList)
        *(*pxOverflowDelayedTaskList)
        *(*xPendingReadyList)
        *(*xTasksWaitingTermination)
        *(*xSuspendedTaskList)
        *(*uxCurrentNumberOfTasks)
        *(*uxTopUsedPriority)


        *(*_tx_thread_current_ptr)
        *(*_tx_thread_created_ptr)
        *(*_tx_thread_created_count)

        *(.bss*)
        *(COMMON)
        link_bss_end = .;
        . = ALIGN(., 4);
    }> SRAM_160K AT>SRAM_160K


    .data : /* Contains the non-zero initialised global variables */
    {
        link_global_data_initial_values = LOADADDR( .data ); /* This is the location in flash of the initial values of global variables */
        link_global_data_start = .;
        *(.data*)
        link_global_data_end = .;
        . = ALIGN(., 4);
    }> SRAM_160K AT> SRAM_160K


    .stack : /* Contains the initial stack */
    {
        link_stack_location = ALIGN(., 4);
        *(.stack)
        . = ALIGN(MAX(link_stack_location + __STACKSIZE__ , .), 4);
        link_stack_end = .;
    }> SRAM_160K AT>SRAM_160K

    /DISCARD/ :
    {
        *(.ARM.attributes*)
        *(.comment)
        *(.init)
        *(.preinit)
        *(.fini)
        *(.fini_array)
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidx.*)
        *(.eh_frame_hdr)
        *(.eh_frame)
        *(.gnu.linkonce.armextab.*)
        *(.v4_bx)
        *(.vfp11_veneer)
        *(.gcc_except_table)
        *(.eh_frame_hdr)
        *(.eh_frame)
        *(.glue*)
    }
}



/* Declare libc Heap to start at end of allocated RAM */

PROVIDE( _heap = link_stack_end );

/* End of the heap is top of RAM, aligned 8 byte */

PROVIDE( _eheap = ALIGN( ORIGIN( SRAM_160K ) + LENGTH( SRAM_160K ) - 8, 8 ) );

/* ThreadX aliases */
PROVIDE( __RAM_segment_used_end__ = link_stack_end );
PROVIDE( __tx_free_memory_start = link_stack_end );
PROVIDE( __tx_vectors = link_interrupt_vectors_location );

