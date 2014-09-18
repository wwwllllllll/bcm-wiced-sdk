/*--------------------------------------------------------------- 
 * Copyright (c) 1999,2000,2001,2002,2003                              
 * The Board of Trustees of the University of Illinois            
 * All Rights Reserved.                                           
 *--------------------------------------------------------------- 
 * Permission is hereby granted, free of charge, to any person    
 * obtaining a copy of this software (Iperf) and associated       
 * documentation files (the "Software"), to deal in the Software  
 * without restriction, including without limitation the          
 * rights to use, copy, modify, merge, publish, distribute,        
 * sublicense, and/or sell copies of the Software, and to permit     
 * persons to whom the Software is furnished to do
 * so, subject to the following conditions: 
 *
 *     
 * Redistributions of source code must retain the above 
 * copyright notice, this list of conditions and 
 * the following disclaimers. 
 *
 *     
 * Redistributions in binary form must reproduce the above 
 * copyright notice, this list of conditions and the following 
 * disclaimers in the documentation and/or other materials 
 * provided with the distribution. 
 * 
 *     
 * Neither the names of the University of Illinois, NCSA, 
 * nor the names of its contributors may be used to endorse 
 * or promote products derived from this Software without
 * specific prior written permission. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE CONTIBUTORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 * ________________________________________________________________
 * National Laboratory for Applied Network Research 
 * National Center for Supercomputing Applications 
 * University of Illinois at Urbana-Champaign 
 * http://www.ncsa.uiuc.edu
 * ________________________________________________________________ 
 *
 * delay.c
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * accurate microsecond delay
 * ------------------------------------------------------------------- */

#include "Timestamp.hpp"
#include "util.h"
#include "delay.hpp"
#include "platform_config.h"

/* -------------------------------------------------------------------
 * A micro-second delay function using POSIX nanosleep(). This allows a
 * higher timing resolution (under Linux e.g. it uses hrtimers), does not
 * affect any signals, and will use up remaining time when interrupted.
 * ------------------------------------------------------------------- */
void delay_loop(unsigned long usec) {
#if defined(WIN32)
	/** @see http://stackoverflow.com/questions/85122/sleep-less-than-one-millisecond */
    static bool winsock_initialised = false;

    if (!winsock_initialised) {
        WORD wVersionRequested = MAKEWORD(1,0);
        WSADATA wsaData;
        WSAStartup(wVersionRequested, &wsaData);
        winsock_initialised = true;
    }

	struct timeval tv;
    fd_set dummy;
    SOCKET s = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
    FD_ZERO( &dummy );
    FD_SET( s, &dummy );
    tv.tv_sec = usec / 1000000L;
    tv.tv_usec = usec % 1000000L;
    select(0, 0, 0, &dummy, &tv);
    closesocket(s);
#elif defined(WICED)
    uint32_t remainder = usec % 1000;
    uint32_t msec = usec / 1000;

    host_rtos_delay_milliseconds( msec );

#ifdef RTOS_FreeRTOS
    #define _TICK_RATE_HZ   configTICK_RATE_HZ
    #define _CPU_CLOCK_HZ   configCPU_CLOCK_HZ
#elif RTOS_ThreadX
    #define _TICK_RATE_HZ   SYSTICK_FREQUENCY
    #define _CPU_CLOCK_HZ   CPU_CLOCK_HZ
#endif

    remainder = usec % ( 1000000 / _TICK_RATE_HZ );
    if ( remainder != 0 )
    {
        volatile uint32_t clock_in_megahertz = (uint32_t) ( _CPU_CLOCK_HZ / 1000000 );
        for ( ; clock_in_megahertz != 0; clock_in_megahertz-- )
        {
            volatile uint32_t tmp_us = remainder;
            for ( ; tmp_us != 0; tmp_us-- ) /* do nothing */;
        }
    }
#else
    struct timespec requested, remaining;

    requested.tv_sec  = 0;
    requested.tv_nsec = usec * 1000L;

    while (nanosleep(&requested, &remaining) == -1)
        if (errno == EINTR)
            requested = remaining;
        else {
            WARN_errno(1, ( "nanosleep encountered an error.\r\n" ) );
            break;
        }
#endif /* WIN32 */
} // end delay_loop
