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
 * Client.cpp
 * by Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * A client thread initiates a connect to the server and handles
 * sending and receiving data, then closes the socket.
 * ------------------------------------------------------------------- */

#include "headers.h"
#include "Client.hpp"
#include "Thread.h"
#include "SocketAddr.h"
#include "PerfSocket.hpp"
#include "Extractor.h"
#include "delay.hpp"
#include "util.h"
#include "Locale.h"

/* -------------------------------------------------------------------
 * Store server hostname, optionally local hostname, and socket info.
 * ------------------------------------------------------------------- */
Client::Client( thread_Settings *inSettings ) {
    IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Creating new client.\r\n" ) );

    mSettings = inSettings;
    mBuf = NULL;

    // initialize buffer
    mBuf = (char*) malloc( mSettings->mBufLen );
	FAIL_errno( mBuf == NULL, ( "No memory for Client::mBuf.\r\n" ), inSettings );
    IPERF_DEBUGF( MEMALLOC_DEBUG | IPERF_DBG_TRACE, IPERF_MEMALLOC_MSG( mBuf, mSettings->mBufLen ) );
	
    IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Filling client buffer with data.\r\n" ) );
    pattern( mBuf, mSettings->mBufLen );
#ifndef NO_FILE_IO
    if ( isFileInput( mSettings ) ) {
        if ( !isSTDIN( mSettings ) )
            Extractor_Initialize( mSettings->mFileName, mSettings->mBufLen, mSettings );
        else
            Extractor_InitializeFile( stdin, mSettings->mBufLen, mSettings );

        if ( !Extractor_canRead( mSettings ) ) {
            fprintf(stderr, "Cannot read extractor... unsetting file input.\r\n");
            unsetFileInput( mSettings );
        }
    }
#endif /* NO_FILE_IO */

    // connect
    Connect( );

    if ( isReport( inSettings ) ) {
        IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Client is reporting settings.\r\n" ) );
        ReportSettings( inSettings );
        if ( mSettings->multihdr && isMultipleReport( inSettings ) ) {
            mSettings->multihdr->report->connection.peer = mSettings->peer;
            mSettings->multihdr->report->connection.size_peer = mSettings->size_peer;
            mSettings->multihdr->report->connection.local = mSettings->local;
            SockAddr_setPortAny( &mSettings->multihdr->report->connection.local );
            mSettings->multihdr->report->connection.size_local = mSettings->size_local;
        }
    }
} // end Client

/* -------------------------------------------------------------------
 * Delete memory (hostname strings).
 * ------------------------------------------------------------------- */
Client::~Client() {
    IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Destroying Client.\r\n" ) );

    if ( mSettings->mSock != (signed) INVALID_SOCKET ) {
        IPERF_DEBUGF( CLIENT_DEBUG | SOCKET_DEBUG | IPERF_DBG_TRACE, ( "Closing Client socket %d.\r\n", mSettings->mSock ) );
        int rc = close( mSettings->mSock );
        WARN_errno( rc == SOCKET_ERROR, ( "Socket close failed.\r\n" ) );
        mSettings->mSock = INVALID_SOCKET;
    }
    IPERF_DEBUGF( MEMFREE_DEBUG | IPERF_DBG_TRACE, IPERF_MEMFREE_MSG( mBuf ) );
    FREE_PTR( mBuf );
} // end ~Client

const double kSecs_to_usecs = 1e6; 
const int    kBytes_to_Bits = 8; 

void Client::RunTCP( void ) {
    IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "TCP client running.\r\n" ) );

	/**
	 * BUGFIX: This should not be declared unsigned.
	 * @see http://sourceforge.net/tracker/?func=detail&aid=3454612&group_id=128336&atid=711373
	 */
    long currLen = 0; 
#ifndef NO_ITIMER
    struct itimerval it;
#endif /* NO_ITIMER */
    max_size_t totLen = 0;

#ifndef NO_ITIMER
    int err;
#endif /* NO_ITIMER */

#ifndef NO_FILE_IO
    char* readAt = mBuf;
#endif /* NO_FILE_IO */

    // Indicates if the stream is readable 
#ifndef NO_FILE_IO
    bool canRead = true;
#endif /* NO_FILE_IO */
	bool mMode_Time = isModeTime( mSettings ); 

    ReportStruct *reportstruct = NULL;

    // InitReport handles Barrier for multiple Streams
    IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE, ( "Client is initializing report.\r\n" ) );
    mSettings->reporthdr = InitReport( mSettings );
    reportstruct = (ReportStruct*) malloc( sizeof( ReportStruct ) );
    FAIL_errno( reportstruct == NULL, ( "No memory for ReportStruct reportstruct.\r\n" ), mSettings );
    IPERF_DEBUGF( MEMALLOC_DEBUG | IPERF_DBG_TRACE, IPERF_MEMALLOC_MSG( reportstruct, sizeof( ReportStruct ) ) );
    reportstruct->packetID = 0;

    lastPacketTime.setnow();
#ifndef NO_ITIMER
    if ( mMode_Time ) {
        memset (&it, 0, sizeof (it));
        it.it_value.tv_sec = (int) (mSettings->mAmount / 100.0);
        it.it_value.tv_usec = ( (int) 10000 * (mSettings->mAmount -
            it.it_value.tv_sec * 100.0) );
        err = setitimer( ITIMER_REAL, &it, NULL );
        if ( err != 0 ) {
            perror("setitimer failed");
            exit(1);
        }
    }
#endif /* NO_ITIMER */

    IPERF_DEBUGF( CLIENT_DEBUG | SOCKET_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Client is beginning to transmit TCP packets.\r\n" ) );
    do {
#ifndef NO_FILE_IO
        // Read the next data block from 
        // the file if it's file input 
        if ( isFileInput( mSettings ) ) {
            Extractor_getNextDataBlock( readAt, mSettings ); 
            canRead = Extractor_canRead( mSettings ) != 0; 
        } else
            canRead = true; 
#endif /* NO_FILE_IO */

        // perform write
#if HAVE_QUAD_SUPPORT
        IPERF_DEBUGF_COUNTER( SOCKET_DEBUG | IPERF_DBG_TRACE, ( "Client is writing %d bytes to socket %d [total length=%llu].\r\n", mSettings->mBufLen, mSettings->mSock, (long long unsigned) totLen ) );
#else
        IPERF_DEBUGF_COUNTER( SOCKET_DEBUG | IPERF_DBG_TRACE, ( "Client is writing %d bytes to socket %d [total length=%lu].\r\n", mSettings->mBufLen, mSettings->mSock, (long unsigned) totLen ) );
#endif /* HAVE_QUAD_SUPPORT */
        currLen = write( mSettings->mSock, mBuf, mSettings->mBufLen );
        if ( currLen < 0 
#if !defined(WIN32) && !defined(NO_ITIMER)
                         && errno != ENOBUFS
#endif /* !defined(WIN32) && !defined(NO_ITIMER) */
            ) {
            WARN_errno( currLen < 0, ( "Unexpected return value from socket write operation: %ld.\r\n", currLen ) );
            break; 
        }
		totLen += currLen;

		if(mSettings->mInterval > 0) {
        	gettimeofday( &(reportstruct->packetTime), NULL );
        	reportstruct->packetLen = currLen;
            IPERF_DEBUGF_COUNTER( TIME_DEBUG | IPERF_DBG_TRACE, ( "Packet time for packet %d is %ld sec %ld usec.\r\n", reportstruct->packetID, reportstruct->packetTime.tv_sec, reportstruct->packetTime.tv_usec ) );
				
            IPERF_DEBUGF_COUNTER( CLIENT_DEBUG | IPERF_DBG_TRACE, ( "Client is reporting packet %d.\r\n", reportstruct->packetID ) );
            ReportPacket( mSettings->reporthdr, reportstruct );
        }	

        if ( !mMode_Time ) {
            /* mAmount may be unsigned, so don't let it underflow! */
            if( currLen > 0 && mSettings->mAmount >= (unsigned) currLen ) {
                mSettings->mAmount -= currLen;
            } else {
                mSettings->mAmount = 0;
            }
        }
        IPERF_DEBUGF_COUNTER_INCREMENT();
    } while ( ! (
#ifndef NO_INTERRUPTS
					sInterupted  || 
#endif /* NO_INTERRUPTS */
                   (!mMode_Time  &&  0 >= mSettings->mAmount)) 
#ifndef NO_FILE_IO
				   && canRead 
#endif /* NO_FILE_IO */
				   ); 

    // stop timing
    IPERF_DEBUGF( CLIENT_DEBUG | SOCKET_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Client has finished transmitting TCP packets.\r\n" ) );
    gettimeofday( &(reportstruct->packetTime), NULL );
    IPERF_DEBUGF( CLIENT_DEBUG | TIME_DEBUG | IPERF_DBG_TRACE, ( "Client has stopped timing. Stop time is %ld sec %ld us.\r\n", reportstruct->packetTime.tv_sec, reportstruct->packetTime.tv_usec ) );

    // if we're not doing interval reporting, report the entire transfer as one big packet
    if(0.0 == mSettings->mInterval) {
        reportstruct->packetLen = totLen;
        IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE, ( "Client is reporting packet %d.\r\n", reportstruct->packetID ) );
        ReportPacket( mSettings->reporthdr, reportstruct );
    }
    IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE, ( "Client is closing report with index %d.\r\n", mSettings->reporthdr->reporterindex ) );
    CloseReport( mSettings->reporthdr, reportstruct );

    IPERF_DEBUGF( MEMFREE_DEBUG | IPERF_DBG_TRACE, IPERF_MEMFREE_MSG( reportstruct ) );
    FREE_PTR( reportstruct );
	
    IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE, ( "Client is ending report with index %d.\r\n", mSettings->reporthdr->reporterindex ) );
    EndReport( mSettings->reporthdr );
} // end Client::RunTCP

/* ------------------------------------------------------------------- 
 * Send data using the connected UDP/TCP socket, 
 * until a termination flag is reached. 
 * Does not close the socket. 
 * ------------------------------------------------------------------- */ 
void Client::Run( void ) {
    IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Running Client.\r\n" ) );

    struct UDP_datagram* mBuf_UDP = (struct UDP_datagram*) mBuf; 
    unsigned long currLen = 0; 

    int delay_target = 0; 
    int delay = 0; 
    int adjust = 0; 

#ifndef NO_FILE_IO
    char* readAt = mBuf;
#endif /* NO_FILE_IO */

#if HAVE_THREAD
#if 0
    /**
     * BUGFIX: This causes iperf server to terminate unexpectedly.
     * @see http://sourceforge.net/tracker/index.php?func=detail&aid=1983829&group_id=128336&atid=711371
     */
    if ( !isUDP( mSettings ) ) {
		RunTCP();
		return;
    }
#endif /* 0 */
#endif /* HAVE_THREAD */
    
    // Indicates if the stream is readable 
#ifndef NO_FILE_IO
    bool canRead = true;
#endif /* NO_FILE_IO */
	bool mMode_Time = isModeTime( mSettings ); 

    // setup termination variables
    if ( mMode_Time ) {
        mEndTime.setnow();
        mEndTime.add( mSettings->mAmount / 100.0 );
        IPERF_DEBUGF( CLIENT_DEBUG | TIME_DEBUG | IPERF_DBG_TRACE, ( "Client set end time to %ld sec %ld usec.\r\n", mEndTime.getSecs( ), mEndTime.getUsecs( ) ) );
    }

    if ( isUDP( mSettings ) ) {
        // Due to the UDP timestamps etc, included 
        // reduce the read size by an amount 
        // equal to the header size
    
        // compute delay for bandwidth restriction, constrained to [0,1] seconds 
        delay_target = (int) ( mSettings->mBufLen * ((kSecs_to_usecs * kBytes_to_Bits) 
                                                     / mSettings->mUDPRate) ); 
        IPERF_DEBUGF( CLIENT_DEBUG | TIME_DEBUG | IPERF_DBG_TRACE, ( "Client calculated time delay for bandwidth restriction as %d usec.\r\n", delay_target ) );
        if ( delay_target < 0  || 
             delay_target > (int) 1 * kSecs_to_usecs ) {
            fprintf( stderr, warn_delay_large, delay_target / kSecs_to_usecs );
            delay_target = (int) kSecs_to_usecs * 1; 
        }
#ifndef NO_FILE_IO
        if ( isFileInput( mSettings ) ) {
            if ( isCompat( mSettings ) ) {
                Extractor_reduceReadSize( sizeof(struct UDP_datagram), mSettings );
                readAt += sizeof(struct UDP_datagram);
            } else {
                Extractor_reduceReadSize( sizeof(struct UDP_datagram) +
                                          sizeof(struct client_hdr), mSettings );
                readAt += sizeof(struct UDP_datagram) +
                          sizeof(struct client_hdr);
            }
        }
#endif /* NO_FILE_IO */
    }

    ReportStruct *reportstruct = NULL;

    // InitReport handles Barrier for multiple Streams
    IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE, ( "Client is initializing a report.\r\n" ) );
    mSettings->reporthdr = InitReport( mSettings );
    reportstruct = (ReportStruct*) malloc( sizeof( ReportStruct ) );
    FAIL_errno( reportstruct == NULL, ( "No memory for ReportStruct reportstruct.\r\n" ), mSettings );
    IPERF_DEBUGF( MEMALLOC_DEBUG | IPERF_DBG_TRACE, IPERF_MEMALLOC_MSG( reportstruct, sizeof( ReportStruct ) ) );
    reportstruct->packetID = 0;

    lastPacketTime.setnow();
    IPERF_DEBUGF( CLIENT_DEBUG | SOCKET_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Client is beginning to transmit packets.\r\n" ) );
    
    do {
        // Test case: drop 17 packets and send 2 out-of-order: 
        // sequence 51, 52, 70, 53, 54, 71, 72 
        //switch( datagramID ) { 
        //  case 53: datagramID = 70; break; 
        //  case 71: datagramID = 53; break; 
        //  case 55: datagramID = 71; break; 
        //  default: break; 
        //} 
        gettimeofday( &(reportstruct->packetTime), NULL );

        if ( isUDP( mSettings ) ) {
            // store datagram ID into buffer 
#if ( defined(WICED) && defined(NETWORK_NetX) || defined(NETWORK_NetX_Duo) )
            mBuf_UDP->id      = wiced_htonl( (reportstruct->packetID) );
            reportstruct->packetID++;
            mBuf_UDP->tv_sec  = wiced_htonl( reportstruct->packetTime.tv_sec );
            mBuf_UDP->tv_usec = wiced_htonl( reportstruct->packetTime.tv_usec );
#else
            mBuf_UDP->id      = htonl( (reportstruct->packetID) );
            reportstruct->packetID++;
            mBuf_UDP->tv_sec  = htonl( reportstruct->packetTime.tv_sec ); 
            mBuf_UDP->tv_usec = htonl( reportstruct->packetTime.tv_usec );
#endif
            // delay between writes 
            // make an adjustment for how long the last loop iteration took 
            // TODO this doesn't work well in certain cases, like 2 parallel streams 
            adjust = delay_target + lastPacketTime.subUsec( reportstruct->packetTime ); 
            lastPacketTime.set( reportstruct->packetTime.tv_sec, 
                                reportstruct->packetTime.tv_usec ); 

            if ( adjust > 0  ||  delay > 0 ) {
                delay += adjust; 
                IPERF_DEBUGF_COUNTER( TIME_DEBUG | IPERF_DBG_TRACE, ( "Client adjusted delay time by %d usec. Delay time is now %d usec.\r\n", adjust, delay ) );
            }
        }

#ifndef NO_FILE_IO
        // Read the next data block from 
        // the file if it's file input 
        if ( isFileInput( mSettings ) ) {
            Extractor_getNextDataBlock( readAt, mSettings ); 
            canRead = Extractor_canRead( mSettings ) != 0; 
        } else
            canRead = true; 
#endif /* NO_FILE_IO */
			
        // perform write 
        IPERF_DEBUGF_COUNTER( SOCKET_DEBUG | IPERF_DBG_TRACE, ( "Client is writing %d bytes to host %s through socket %d.\r\n", mSettings->mBufLen, mSettings->mHost, mSettings->mSock ) );
#if ( defined(WICED) && defined(NETWORK_NetX) || defined(NETWORK_NetX_Duo) )
        currLen = sendto( mSettings->mSock, mBuf, mSettings->mBufLen, 0,  (sockaddr*)&mSettings->peer, mSettings->size_peer);
#else
        currLen = write( mSettings->mSock, mBuf, mSettings->mBufLen );
#endif
        if ( currLen < 0 
#if !defined(WIN32) && !defined(NO_ITIMER)
		     && errno != ENOBUFS
#endif /* !defined(WIN32) && !defined(NO_ITIMER) */
			 ) {
            WARN_errno( currLen < 0, ( "Unexpected return value from socket write operation: %ld.\r\n", currLen ) );
            break; 
        }

        IPERF_DEBUGF_COUNTER( TIME_DEBUG | IPERF_DBG_TRACE, ( "Packet time for packet %d is %ld sec %ld usec.\r\n", reportstruct->packetID, reportstruct->packetTime.tv_sec, reportstruct->packetTime.tv_usec ) );

        // report packets 
        IPERF_DEBUGF_COUNTER( CLIENT_DEBUG | IPERF_DBG_TRACE, ( "Client is reporting packet %d.\r\n", reportstruct->packetID ) );
        reportstruct->packetLen = currLen;
        ReportPacket( mSettings->reporthdr, reportstruct );
        
        if ( delay > 0 ) {
#if HAVE_QUAD_SUPPORT
            IPERF_DEBUGF_COUNTER( CLIENT_DEBUG | TIME_DEBUG | IPERF_DBG_TRACE, ( "Client is delaying for %dms to constrain bandwidth to %llu bits/sec.\r\n", delay, (long long unsigned) mSettings->mUDPRate ) );
#else
            IPERF_DEBUGF_COUNTER( CLIENT_DEBUG | TIME_DEBUG | IPERF_DBG_TRACE, ( "Client is delaying for %dms to constrain bandwidth to %lu bits/sec.\r\n", delay, (long unsigned) mSettings->mUDPRate ) );
#endif /* HAVE_QUAD_SUPPORT */
            delay_loop( delay ); 
        }
        if ( !mMode_Time ) {
            /* mAmount may be unsigned, so don't let it underflow! */
            if( mSettings->mAmount >= currLen ) {
                mSettings->mAmount -= currLen;
            } else {
                mSettings->mAmount = 0;
            }
        }
        IPERF_DEBUGF_COUNTER_INCREMENT();
    } while ( ! (
#ifndef NO_INTERRUPTS
				 sInterupted  || 
#endif /* NO_INTERRUPTS */
                 (mMode_Time   &&  mEndTime.before( reportstruct->packetTime ))  || 
                 (!mMode_Time  &&  0 >= mSettings->mAmount))
#ifndef NO_FILE_IO
				 && canRead 
#endif /* NO_FILE_IO */
				 ); 

    // stop timing
    IPERF_DEBUGF( CLIENT_DEBUG | SOCKET_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Client has finished transmitting packets.\r\n" ) );
    gettimeofday( &(reportstruct->packetTime), NULL );
    IPERF_DEBUGF( CLIENT_DEBUG | TIME_DEBUG | IPERF_DBG_TRACE, ( "Client has stopped timing. Stop time is %ld sec %ld us.\r\n", reportstruct->packetTime.tv_sec, reportstruct->packetTime.tv_usec ) );

    IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE, ( "Client is closing report with index %d.\r\n", mSettings->reporthdr->reporterindex ) );
    CloseReport( mSettings->reporthdr, reportstruct );

    if ( isUDP( mSettings ) ) {
        // send a final terminating datagram 
        // Don't count in the mTotalLen. The server counts this one, 
        // but didn't count our first datagram, so we're even now. 
        // The negative datagram ID signifies termination to the server. 

        IPERF_DEBUGF( CLIENT_DEBUG | SOCKET_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Client is sending the final terminating datagram.\r\n" ) );
    
        // store datagram ID into buffer 
#if ( defined(WICED) && defined(NETWORK_NetX) || defined(NETWORK_NetX_Duo) )
        mBuf_UDP->id      = wiced_htonl( -(reportstruct->packetID) );
        mBuf_UDP->tv_sec  = wiced_htonl( reportstruct->packetTime.tv_sec );
        mBuf_UDP->tv_usec = wiced_htonl( reportstruct->packetTime.tv_usec );
#else
        mBuf_UDP->id      = htonl( -(reportstruct->packetID)  ); 
        mBuf_UDP->tv_sec  = htonl( reportstruct->packetTime.tv_sec ); 
        mBuf_UDP->tv_usec = htonl( reportstruct->packetTime.tv_usec ); 
#endif
        if ( isMulticast( mSettings ) ) {
            int rc;
#if ( defined(WICED) && defined(NETWORK_NetX) || defined(NETWORK_NetX_Duo) )
            rc = sendto( mSettings->mSock, mBuf, mSettings->mBufLen, 0,  (sockaddr*)&mSettings->peer, mSettings->size_peer);
#else
            rc = write( mSettings->mSock, mBuf, mSettings->mBufLen );
#endif
            (void) rc;
        } else {
            write_UDP_FIN( ); 
        }
    }
    IPERF_DEBUGF( MEMFREE_DEBUG | IPERF_DBG_TRACE, IPERF_MEMFREE_MSG( reportstruct ) );
    FREE_PTR( reportstruct );
	
    IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE, ( "Client is ending report with index %d.\r\n", mSettings->reporthdr->reporterindex ) );
    EndReport( mSettings->reporthdr ); 
} // end Client::Run

void Client::InitiateServer() {
    IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Client is initiating with the server.\r\n" ) );

    if ( !isCompat( mSettings ) ) {
        int currLen;
        client_hdr* temp_hdr;
        if ( isUDP( mSettings ) ) {
            UDP_datagram *UDPhdr = (UDP_datagram *)mBuf;
            temp_hdr = (client_hdr*)(UDPhdr + 1);
        } else {
            temp_hdr = (client_hdr*)mBuf;
        }
        IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE, ( "Client is generating the client header.\r\n" ) );
        Settings_GenerateClientHdr( mSettings, temp_hdr );
        if ( !isUDP( mSettings ) ) {
            IPERF_DEBUGF( SOCKET_DEBUG | IPERF_DBG_TRACE, ( "Client is sending its header to the server using socket %d.\r\n", mSettings->mSock ) );
            currLen = send( mSettings->mSock, mBuf, sizeof(client_hdr), 0 );
            WARN_errno( currLen < 0, ( "Unexpected return value from socket write operation: %d.\r\n", currLen ) );
        }
    }
} // end Client::InitiateServer

/* -------------------------------------------------------------------
 * Setup a socket connected to a server.
 * If inLocalhost is not null, bind to that address, specifying
 * which outgoing interface to use.
 * ------------------------------------------------------------------- */
void Client::Connect( ) {
    int rc;
    SockAddr_remoteAddr( mSettings );

    assert( mSettings->mHost != NULL );

    IPERF_DEBUGF( CLIENT_DEBUG | SOCKET_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Client is setting up a socket connected to server %s.\r\n", mSettings->mHost ) );

    // create an internet socket
    int type = ( isUDP( mSettings )  ?  SOCK_DGRAM : SOCK_STREAM);

    int domain = (SockAddr_isIPv6( &mSettings->peer ) ?
#ifdef HAVE_IPV6
                  AF_INET6
#else
                  AF_INET
#endif /* HAVE_IPV6 */
                  : AF_INET);

    mSettings->mSock = socket( domain, type, 0 );
    WARN_errno( mSettings->mSock == (signed) INVALID_SOCKET, ( "Invalid socket descriptor.\r\n" ) );

    IPERF_DEBUGF( SOCKET_DEBUG | IPERF_DBG_TRACE, ("Client is setting socket options for socket %d: {\r\n"
        "\tTCP window = %d\r\n"
        "\tCongestion control = %s\r\n"
        "\tMulticast = %s\r\n"
        "\tMulticast TTL = %d\r\n"
        "\tIP TOS = %d\r\n"
        "\tTCP MSS = %d\r\n"
		"\tTCP no delay = %s\r\n}\r\n",
		mSettings->mSock,
		mSettings->mTCPWin,
		isCongestionControl( mSettings ) ? "yes" : "no",
		isMulticast( mSettings ) ? "yes" : "no",
		mSettings->mTTL,
		mSettings->mTOS,
		mSettings->mMSS,
		isNoDelay( mSettings ) ? "yes" : "no" ) );
    SetSocketOptions( mSettings );

    SockAddr_localAddr( mSettings );
    if ( mSettings->mLocalhost != NULL ) {
        // bind socket to local address
        IPERF_DEBUGF( CLIENT_DEBUG | SOCKET_DEBUG | IPERF_DBG_TRACE, ( "Client is binding socket %d to localhost %s.\r\n", mSettings->mSock, mSettings->mLocalhost ) );
        rc = bind( mSettings->mSock, (sockaddr*) &mSettings->local,
                   SockAddr_get_sizeof_sockaddr( &mSettings->local ) );
        WARN_errno( rc == SOCKET_ERROR, ( "Socket bind failed.\r\n" ) );
    }

    // connect socket
    IPERF_DEBUGF( CLIENT_DEBUG | SOCKET_DEBUG | IPERF_DBG_TRACE, ( "Client is connecting to %s using socket %d.\r\n", mSettings->mHost, mSettings->mSock ) );

#if ( defined(WICED) && defined(NETWORK_NetX) || defined(NETWORK_NetX_Duo) )
    if ( type == SOCK_DGRAM )
    {
        //nx_udp_socket_bind((NX_UDP_SOCKET*)&mSettings->mSock, mSettings->mPort, TX_WAIT_FOREVER);
        bind(mSettings->mSock, (sockaddr*)&mSettings->peer, SockAddr_get_sizeof_sockaddr( &mSettings->peer ));
        rc = getsockname( mSettings->mSock, (sockaddr*) &mSettings->local,
                     &mSettings->size_local );
        //rc = getpeername( mSettings->mSock, (sockaddr*) &mSettings->peer,
          //           &mSettings->size_peer );
    }
    else
    {
        rc = connect( mSettings->mSock, (sockaddr*) &mSettings->peer,
                      SockAddr_get_sizeof_sockaddr( &mSettings->peer ));
        FAIL_errno( rc == SOCKET_ERROR, ( "Socket connect failed.\r\n" ), mSettings );
        rc = getsockname( mSettings->mSock, (sockaddr*) &mSettings->local,
                     &mSettings->size_local );
        rc = getpeername( mSettings->mSock, (sockaddr*) &mSettings->peer,
                     &mSettings->size_peer );
    }
#else
    rc = connect( mSettings->mSock, (sockaddr*) &mSettings->peer,
                  SockAddr_get_sizeof_sockaddr( &mSettings->peer ));
    FAIL_errno( rc == SOCKET_ERROR, ( "Socket connect failed.\r\n" ), mSettings );
    rc = getsockname( mSettings->mSock, (sockaddr*) &mSettings->local,
                 &mSettings->size_local );
    rc = getpeername( mSettings->mSock, (sockaddr*) &mSettings->peer,
                 &mSettings->size_peer );
#endif
} // end Client::Connect

/* ------------------------------------------------------------------- 
 * Send a datagram on the socket. The datagram's contents should signify 
 * a FIN to the application. Keep re-transmitting until an 
 * acknowledgement datagram is received. 
 * ------------------------------------------------------------------- */ 
void Client::write_UDP_FIN( ) {
    int rc; 
    fd_set readSet; 
    struct timeval timeout; 

    int count = 0; 
    while ( count < 10 ) {
        count++; 

        // write data 
#if ( defined(WICED) && defined(NETWORK_NetX) || defined(NETWORK_NetX_Duo) )
        rc = sendto( mSettings->mSock, mBuf, mSettings->mBufLen, 0,  (sockaddr*)&mSettings->peer, mSettings->size_peer);
#else
        rc = write( mSettings->mSock, mBuf, mSettings->mBufLen ); 
#endif
        // wait until the socket is readable, or our timeout expires 
        FD_ZERO( &readSet ); 
        FD_SET( mSettings->mSock, &readSet ); 
        timeout.tv_sec  = 0; 
        timeout.tv_usec = 250000; // quarter second, 250 ms 

        rc = select( mSettings->mSock+1, &readSet, NULL, NULL, &timeout ); 
        FAIL_errno( rc == SOCKET_ERROR, ( "Socket select failed.\r\n" ), mSettings );

        if ( rc == 0 ) {
            // select timed out 
            continue; 
        } else {
            // socket ready to read 
            rc = read( mSettings->mSock, mBuf, mSettings->mBufLen ); 
//            WARN_errno( rc < 0, ( "Unexpected value from socket read operation: %d.\r\n", rc ) );
    	    if ( rc < 0 ) {
                break;
            } else if ( rc >= (int) (sizeof(UDP_datagram) + sizeof(server_hdr)) ) {
                IPERF_DEBUGF( CLIENT_DEBUG | IPERF_DBG_TRACE | IPERF_DBG_STATE, ( "Client will generate a report of the UDP statistics as reported by the server.\r\n" ) );
                ReportServerUDP( mSettings, (server_hdr*) ((UDP_datagram*)mBuf + 1) );
            }

            return; 
        } 
    } 

    fprintf( stderr, warn_no_ack, mSettings->mSock, count ); 
} // end Client::write_UDP_FIN 
