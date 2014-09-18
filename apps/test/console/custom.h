/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef CONSOLE_CUSTOM_H_
#define CONSOLE_CUSTOM_H_

#include "iperf/iperf.h"
#include "mallinfo/mallinfo.h"
#include "thread/thread.h"
#include "trace/console_trace.h"
#include "platform/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/
#define CUSTOM_COMMANDS  \
    IPERF_COMMANDS \
    MALLINFO_COMMANDS \
    THREAD_COMMANDS \
    TRACE_COMMANDS \
    PLATFORM_COMMANDS \

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
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ifdef CONSOLE_CUSTOM_H_ */

