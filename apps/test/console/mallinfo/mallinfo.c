/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include <malloc.h>
#include <stdio.h>
#include "../console.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

/*!
 ******************************************************************************
 * Print memory allocation information.
 *
 * @param[in] argc  Unused.
 * @param[in] argv  Unused.
 *
 * @return    Console error code indicating if the command ran correctly.
 */

int malloc_info_command( int argc, char *argv[] )
{
    volatile struct mallinfo mi = mallinfo( );

    printf( "malloc_info {\r\n"
            "\tarena:   \t%5d;\t/* total space allocated from system */\r\n"
            "\tordblks: \t%5d;\t/* number of non-inuse chunks */\r\n"
            "\tsmblks:  \t%5d;\t/* unused -- always zero */\r\n"
            "\thblks:   \t%5d;\t/* number of mmapped regions */\r\n"
            "\thblkhd:  \t%5d;\t/* total space in mmapped regions */\r\n"
            "\tusmblks: \t%5d;\t/* unused -- always zero */\r\n"
            "\tfsmblks: \t%5d;\t/* unused -- always zero */\r\n"
            "\tuordblks:\t%5d;\t/* total allocated space */\r\n"
            "\tfordblks:\t%5d;\t/* total non-inuse space */\r\n"
            "\tkeepcost:\t%5d;\t/* top-most, releasable (via malloc_trim) space */\r\n"
            "};\r\n",
            mi.arena,
            mi.ordblks,
            mi.smblks,
            mi.hblks,
            mi.hblkhd,
            mi.usmblks,
            mi.fsmblks,
            mi.uordblks,
            mi.fordblks,
            mi.keepcost );

    return ERR_CMD_OK;
} /* malloc_info_command */
