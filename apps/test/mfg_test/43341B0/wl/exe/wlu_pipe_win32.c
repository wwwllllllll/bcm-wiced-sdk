/*
 * Windows version of remote Wl transport mechanisms (pipes).
 *
 * $Copyright (C) 2008 Broadcom Corporation$
 *
 * $Id: wlu_pipe_win32.c 369367 2012-11-17 01:47:00Z jwang $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <typedefs.h>
#include <proto/ethernet.h>
#ifndef UNDER_CE
#include <errno.h>
#else
#include <winsock2.h>
#endif /* UNDER_CE */
#include <windows.h>
#include <bcmcdc.h>
#include "wlu_remote.h"
#include "wlu_pipe.h"

#if defined (RWL_SERIAL) || defined (RWL_DONGLE)
#define DONGLE_TIME_OUT 30000 /* 30 sec */
#define SERIAL_TIME_OUT 300   /* 300 ms */
#define MAX_SERIAL_READ_RETRY 20 /* max serial read retry limit */
extern char  *g_rwl_device_name_serial;
#endif

#ifdef RWL_SOCKET
#define MAX_IP_ADDR_LENGTH		32
#define MAX_HOST_NAME_LENGTH		255

int g_sockdes;

/* Function for initiates use of Ws2_32.dll for Windows OS
 * in remote socket transport
 */
int
rwl_init_ws2_32dll(void)
{
	int err;
	WORD wVersionRequested;
	WSADATA WSAdata;
	wVersionRequested = MAKEWORD(2, 2);
	memset(&WSAdata, 0, sizeof(WSAdata));
	err = WSAStartup(wVersionRequested, &WSAdata);
	return err;
}
/* Function to terminate the use of Ws2_32.dll for Windows OS
 * in remote socket transport
 */
int
rwl_terminate_ws2_32dll(void)
{
	int err;
	if ((err = WSACleanup()) != SUCCESS) {
		err = WSAGetLastError();
	}
	return err;
}

static int
rwl_opensocket(int AddrFamily, int Type, int Protocol)
{
	int SockDes;
	if ((SockDes = socket(AddrFamily, Type, Protocol)) == INVALID_SOCKET) {
		DPRINT_ERR(ERR, "\n rwl_opensocket Fails:%d\n", WSAGetLastError());
		return FAIL;
	}
	DPRINT_DBG(OUTPUT, "SockDes=%d\n", SockDes);
	return SockDes;

}

static int
rwl_set_socket_option(int SocketDes, int Level, int OptName, int Val)
{
	if (setsockopt(SocketDes, Level, OptName, (const char*)&Val, sizeof(int)) == SOCKET_ERROR) {
		DPRINT_ERR(ERR, "\n Error at rwl_set_socket_option:%d\n", WSAGetLastError());
		return FAIL;
	}
	return SUCCESS;
}

int
rwl_connectsocket(int SocketDes, struct sockaddr* SerAddr, int SizeOfAddr)
{
	if (connect(SocketDes, SerAddr, SizeOfAddr) == SOCKET_ERROR) {
		DPRINT_ERR(ERR, "rwl_connectsocket error:%d\n", WSAGetLastError());
		return FAIL;
	}
	return SUCCESS;
}

/*
 * Function for associating a local address with a socket.
 */
int
rwl_bindsocket(int SocketDes, struct sockaddr * MyAddr, int SizeOfAddr)
{
	if (bind(SocketDes, MyAddr, SizeOfAddr) == SOCKET_ERROR) {
		DPRINT_ERR(ERR, "\n Error at rwl_bindsocket:%d\n", WSAGetLastError());
		return FAIL;
	}

	return SUCCESS;
}

/*
 * Function for making the socket to listen for incoming connection.
 */
int
rwl_listensocket(int SocketDes, int BackLog)
{
	if (listen(SocketDes, BackLog) == SOCKET_ERROR) {
		DPRINT_ERR(ERR, "\n Error at rwl_listensocket:%d\n", WSAGetLastError());
		return FAIL;
	}
	return SUCCESS;
}

/*
 * Function for permitting an incoming connection attempt on a socket
 * Function  called by server
 */
int
rwl_acceptconnection(int SocketDes, struct sockaddr* ClientAddr, int *SizeOfAddr)
{
	int NewSockDes;

	if ((NewSockDes = accept(SocketDes, ClientAddr, SizeOfAddr)) == SOCKET_ERROR) {
		DPRINT_ERR(ERR, "\n rwl_acceptconnection Fails:%d\n", WSAGetLastError());
		return FAIL;
	}
	return NewSockDes;
}

static int
rwl_closesocket(int SocketDes)
{
	if (closesocket(SocketDes) == SOCKET_ERROR) {
		DPRINT_ERR(ERR, "\n Error at rwl_closesocket:%d\n", WSAGetLastError());
		return FAIL;
	}
	return SUCCESS;
}

int
rwl_send_to_streamsocket(int SocketDes, const char* SendBuff, int data_size, int Flag)
{
	int numwritten;

	if ((numwritten = send(SocketDes, SendBuff, data_size, Flag)) == SOCKET_ERROR) {
		DPRINT_ERR(ERR, "\n SendToStreamSocket Fails:%d\n", WSAGetLastError());
		return (FAIL);
	}

	if (numwritten != data_size) {
		DPRINT_ERR(ERR, "numwritten %d != data_size %d\n",
		numwritten, data_size);
		return (FAIL);
	}

	return numwritten;
}
int
rwl_receive_from_streamsocket(int SocketDes, char* RecvBuff, int data_size, int Flag)
{
	int numread = 0;
	int total_numread = 0;

	while (total_numread < data_size) {

		if ((numread = recv(SocketDes, RecvBuff, data_size - total_numread,
		Flag)) == SOCKET_ERROR) {
			DPRINT_ERR(ERR, "\n ReceiveFromStreamSocket Fails:%d\n", WSAGetLastError());
			return FAIL;
		}
		if (numread != data_size - total_numread) {
			DPRINT_DBG(OUTPUT, "asked %d bytes got %d bytes\n",
				data_size - total_numread, numread);
		}
		if (numread == 0)
			break;
		total_numread += numread;
		RecvBuff += numread;
	}
	return numread;
}
static char *
rwl_get_local_nic_ip()
{
	char	szHostName[MAX_HOST_NAME_LENGTH];
	struct hostent*	HostData;
	struct in_addr addr;

	/* init the rwl dll */
	rwl_init_ws2_32dll();
	if (gethostname(szHostName, MAX_HOST_NAME_LENGTH) == SOCKET_ERROR)
		return NULL;
	HostData = gethostbyname(szHostName);
	if (HostData == NULL)
		return NULL;

	addr.s_addr = *((unsigned int*)HostData->h_addr);
	rwl_terminate_ws2_32dll();
	return inet_ntoa(addr);
}

/* This function initializes the socket library binds and listens on the port
 *
 */
int
rwl_init_server_socket_setup(int argc, char** argv, uint remote_type)
{
	int err, SockDes, val;
	struct sockaddr_in ServerAddress;
	char ip_addres[MAX_IP_ADDR_LENGTH];
	unsigned short servPort;
	struct ipv4_addr temp;


	/* Default option */
	servPort = DEFAULT_SERVER_PORT;
	if (rwl_get_local_nic_ip() == NULL) {
		DPRINT_ERR(ERR, "\nunable to get the local ip\n");
		return FAIL;
	}
	strcpy(ip_addres, rwl_get_local_nic_ip());

	/* User option can override default arguments */
	if (argc == 3) {
		*argv++;

		if (!wl_atoip(*argv, &temp)) {
			DPRINT_ERR(ERR, "\n USAGE ERROR:Incorrect IPaddress\n");
			return FAIL;
		}

		strcpy(ip_addres, *argv);
		*argv++;

		if (isdigit(**argv) == FALSE) {
			DPRINT_ERR(ERR, "USAGE ERROR:Incorrect port\n");
			return FAIL;
		}
		servPort = atoi(*argv);
	}


	if (argc == 2) {

		*argv++;
		if (!wl_atoip(*argv, &temp)) {
			if (isdigit(**argv) == FALSE) {
				DPRINT_ERR(ERR, "USAGE ERROR\n");
				return FAIL;
			}
			else
				servPort = atoi(*argv);
		}
		else
			strcpy(ip_addres, *argv);
	}

	DPRINT_INFO(OUTPUT, "INFO: IP: %s, Port:%d\n",
		ip_addres, servPort);


	rwl_init_ws2_32dll();

	if ((SockDes = (*(int *)rwl_open_transport(remote_type, NULL, 0, 0))) == FAIL)
	   return FAIL;

	val = 1;
	if ((rwl_set_socket_option(SockDes, SOL_SOCKET, SO_REUSEADDR, val)) == -1)
		return FAIL;

	memset(&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET; /* host byte order */
	ServerAddress.sin_port = htons(servPort); /* short, network byte order */
	ServerAddress.sin_addr.s_addr = inet_addr(ip_addres);

	if (((err = rwl_bindsocket(SockDes, (struct sockaddr *)&ServerAddress,
	                                    sizeof(ServerAddress))) != 1))
		return err;
	if ((err = rwl_listensocket(SockDes, BACKLOG)) != 1)
		return err;

	DPRINT_DBG(OUTPUT, "Waiting for client to connect...\n");

	return SockDes;
}
#endif /* RWL_SOCKET */

#if defined (RWL_SERIAL) || defined (RWL_DONGLE)
static HANDLE
rwl_open_serial(char *port, int ReadTotalTimeout, int debug)
{
	DCB dcb;
	COMMTIMEOUTS timeouts;
	HANDLE hCom;

#ifdef UNDER_CE
	hCom = CreateFile((LPCWSTR)port,
		GENERIC_READ | GENERIC_WRITE,
		0,    		/* must be opened with exclusive-access */
		NULL, 		/* no security attributes */
		OPEN_EXISTING, 	/* must use OPEN_EXISTING */
		0,    		/* not overlapped I/O */
		NULL);  		/* hTemplate must be NULL for comm devices */
#else
	hCom = CreateFile((const char *)port,
		GENERIC_READ | GENERIC_WRITE,
		0,    		/* must be opened with exclusive-access */
		NULL, 		/* no security attributes */
		OPEN_EXISTING, 	/* must use OPEN_EXISTING */
		0,    		/* not overlapped I/O */
		NULL);  		/* hTemplate must be NULL for comm devices */
#endif /* UNDER_CE */

	if (hCom == INVALID_HANDLE_VALUE) {
		DPRINT_ERR(ERR, "CreateFile failed with error %d.\n", GetLastError());
		return (NULL);
	}

	if (GetCommState(hCom, &dcb) == 0) {
		DPRINT_ERR(ERR, "GetCommState failed with error %d.\n", GetLastError());
		return (NULL);
	}

	dcb.BaudRate = CBR_115200;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	/* no software flow control, will inherit hw flow contr0l settings */
	dcb.fOutX = 0;
	dcb.fInX = 0;

	if (SetCommState(hCom, &dcb) == 0) {
		DPRINT_ERR(ERR, "SetCommState failed with error %d.\n", GetLastError());
		return (NULL);
	}
	if (GetCommTimeouts(hCom, &timeouts) == 0) {
		DPRINT_ERR(ERR, "GetCommTimeouts failed with error %d.\n", GetLastError());
		return (NULL);
	}

	if (PurgeComm(hCom, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR) == 0) {
		DPRINT_ERR(ERR, "PurgeComm failed with error %d.\n", GetLastError());
		return (NULL);
	}

	if (remote_type == REMOTE_DONGLE) {
		timeouts.ReadIntervalTimeout = 0;
		timeouts.ReadTotalTimeoutMultiplier = 0;
		timeouts.ReadTotalTimeoutConstant = DONGLE_TIME_OUT; /* Timeout in milliseconds */
		timeouts.WriteTotalTimeoutMultiplier = 0;
		timeouts.WriteTotalTimeoutConstant = 0;
	} else if (remote_type == REMOTE_SERIAL) {
		/* SYSTEM SERIAL */
		/* The timeouts structure has to be set up (times in milliseconds) */
		timeouts.ReadIntervalTimeout = 0;
		timeouts.ReadTotalTimeoutMultiplier = 0;
		timeouts.ReadTotalTimeoutConstant = SERIAL_TIME_OUT; /* Timeout in milliseconds */
		timeouts.WriteTotalTimeoutMultiplier = 0;
		timeouts.WriteTotalTimeoutConstant = 5000;
#ifdef DBG_SERIAL
		if (debug) {
			printf("timeouts.ReadIntervalTimeout=%d\n",
				timeouts.ReadIntervalTimeout);
			printf("timeouts.ReadTotalTimeoutMultiplier=%d\n",
				timeouts.ReadTotalTimeoutMultiplier);
			printf("timeouts.ReadTotalTimeoutConstant=%d\n",
				timeouts.ReadTotalTimeoutConstant);
			printf("timeouts.WriteTotalTimeoutMultiplier=%d\n",
				timeouts.WriteTotalTimeoutMultiplier);
			printf("timeouts.WriteTotalTimeoutConstant=%d\n",
				timeouts.WriteTotalTimeoutConstant);
		}
#endif
	}

	if (SetCommTimeouts(hCom, &timeouts) == 0) {
		DPRINT_ERR(ERR, "SetCommTimeouts failed with error %d.\n", GetLastError());
		return (NULL);
	}
	return (hCom);
}
int
rwl_write_serial_port(void* hndle, char* write_buf, unsigned long size, unsigned long *numwritten)
{
#ifdef DEBUG_SERIAL
    printf( "\nw %04d > ", size);
    int i;
    for( i = 0; i < size; i++ )
    {
#ifdef READABLE_SERIAL_DUMP
        if ( ( write_buf[i] >= 0x20 ) && ( write_buf[i] <= 0x7E ) )
        {
            printf( "%c",write_buf[i]);
        }
        else
        {
            printf( "\\x%02X",(unsigned int)write_buf[i]);
        }
#else /* ifdef READABLE_SERIAL_DUMP */
        printf( " %02X",(unsigned char)write_buf[i]);
#endif /* ifdef READABLE_SERIAL_DUMP */
    }
    printf("\n");
#endif /* ifdef DEBUG_SERIAL */
	if (WriteFile(hndle, write_buf, size, (DWORD*)numwritten, NULL) == 0) {
		DPRINT_ERR(ERR, "rwl_write_serial_port failed with:%d\n", GetLastError());
		return FAIL;
	}

	if (*numwritten != size) {
		DPRINT_ERR(ERR, "rwl_write_serial_port failed numwritten %ld != len %d\n",
		                                                       *numwritten, size);
		return FAIL;
	}
	return SUCCESS;

}
int
rwl_read_serial_port(void* hndle, char* read_buf, uint data_size, uint *numread)
{
	uint total_numread = 0;
	int c = 0;

	while (total_numread < data_size) {
		if (ReadFile(hndle, read_buf, data_size - total_numread, (LPDWORD) numread, NULL) == 0) {
			DPRINT_ERR(ERR, "rwl_read_serial_port failed with:%d", GetLastError());
			return FAIL;
		}
		if (*numread != data_size - total_numread) {
			c++;
			DPRINT_DBG(OUTPUT, "asked %d bytes got %d bytes\n",
				data_size - total_numread, *numread);
			if (c > MAX_SERIAL_READ_RETRY) {
			        DPRINT_ERR(ERR, "rwl_read_serial_port failed: "
				           "reached max retry limit.\n");
				return FAIL;
			}
			Sleep(10);
		}

		total_numread += *numread;
		read_buf += *numread;
	}
#ifdef DEBUG_SERIAL
    printf( "\nr %04d < ", total_numread);
    int i;
    for( i = 0; i < total_numread; i++ )
    {
#ifdef READABLE_SERIAL_DUMP
        if ( ( read_buf[i] >= 0x20 ) && ( read_buf[i] <= 0x7E ) )
        {
            printf( "%c",read_buf[i]);
        }
        else
        {
            printf( "\\x%02X",(unsigned char)read_buf[i]);
        }
#else /* ifdef READABLE_SERIAL_DUMP */
        printf( " %02X",(unsigned char)read_buf[i]);
#endif /* ifdef READABLE_SERIAL_DUMP */
    }
    printf("\n");
#endif /* ifdef DEBUG_SERIAL */
	return SUCCESS;
}

void
rwl_sync_delay(uint noframes)
{
	/* not required for win32 */
}
#endif /* RWL_SERIAL  || RWL_DONGLE */

#if defined (RWL_SERIAL) ||defined (RWL_DONGLE) || defined (RWL_SOCKET)
void *
rwl_open_transport(int remote_type, char *port, int ReadTotalTimeout, int debug)
{
	HANDLE irh;
#ifdef RWL_SERIAL
	char port_str[10] = "\\\\.\\COM\0";

	strcat(port_str, port);

	if (debug) {
		printf("rwl_open_transpor: port_str=%s\n", port_str);
	}
#endif

	switch (remote_type) {
#ifdef RWL_SERIAL
	case REMOTE_SERIAL:
		if ((irh = rwl_open_serial(port_str, ReadTotalTimeout, debug)) == NULL) {
			printf("rwl_open_transport: Can't open serial port\n");
		}
		break;
#endif /* RWL_SERIAL */
#ifdef RWL_DONGLE
	case REMOTE_DONGLE:
		if ((irh = rwl_open_serial(g_rwl_device_name_serial, 0, 0)) == NULL) {
				printf("rwl_open_transport: Can't open serial port\n");
		}
		break;
#endif /* RWL_DONGLE */
#ifdef RWL_SOCKET
	case REMOTE_SOCKET:
		if ((g_sockdes = rwl_opensocket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == FAIL) {
			printf("rwl_open_transport: \nCan't open socket \n");
			return NULL;
		}
		irh = (void*) &g_sockdes;
		break;
#endif /* RWL_SOCKET */
	default:
		DPRINT_ERR(ERR, "rwl_open_transport: Unknown remote_type %d\n", remote_type);
		irh = NULL;
		break;
	}

	return irh;
}

int
rwl_close_transport(int remote_type, void* handle)
{
	switch (remote_type) {
#ifdef RWL_SOCKET
		case REMOTE_SOCKET:
			shutdown(*((int*)handle), 1);
			if (rwl_closesocket(*((int*)handle)) != 1) {
				 DPRINT_ERR(ERR, "rwl_close_transport: Can't Close socket \n");
				 return FAIL;
			}
			break;
#endif /* RWL_SOCKET */
#if defined (RWL_SERIAL) || defined (RWL_DONGLE)
		case REMOTE_SERIAL:
		case REMOTE_DONGLE:
			if (handle != NULL) {
				if (PurgeComm(handle, PURGE_TXABORT | PURGE_TXCLEAR |
				      PURGE_RXABORT | PURGE_RXCLEAR) == 0) {
					DPRINT_ERR(ERR, "rwl_close_transport: PurgeComm failed.\n");
				}
				Sleep(10);
				if (CloseHandle(handle) == 0) {
					DPRINT_ERR(ERR,
					   "rwl_close_transport: Can't Close serial port.\n");
					return FAIL;
				}
			}
			break;
#endif /* RWL_SERIAL  || RWL_DONGLE */
		default:
		DPRINT_ERR(ERR, "rwl_close_transport: Unknown remote_type %d\n", remote_type);
		break;
	}
	return SUCCESS;
}
#endif /* #if defined (RWL_SERIAL) ||defined (RWL_DONGLE) || defined (RWL_SOCKET) */

void
rwl_sleep(int delay)
{
	Sleep(delay);
}

#if defined(WLMDLL)
int
rwl_init_socket(void)
{
	return rwl_init_ws2_32dll();
}
#endif
