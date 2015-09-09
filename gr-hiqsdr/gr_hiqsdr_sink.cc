/* -*- c++ -*- */
/*
 * Copyright 2007,2008,2009,2010 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gr_hiqsdr_sink.h>
#include <gr_io_signature.h>
#include <stdexcept>
#include <errno.h>
#include <stdio.h>
#include <string.h>


#define HAVE_NETDB_H
//#define HAVE_SYS_TYPES_H
#define HAVE_SYS_SOCKET_H

#if defined(HAVE_NETDB_H)
#include <netdb.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>  //usually included by <netdb.h>?
#endif
typedef void* optval_t;
#elif defined(HAVE_WINDOWS_H)
// if not posix, assume winsock
#define USING_WINSOCK
#include <winsock2.h>
#include <ws2tcpip.h>
#define SHUT_RDWR 2
typedef char* optval_t;
#endif

#include <gruel/thread.h>

#define SNK_VERBOSE 0

static int is_error( int perr )
{
  // Compare error to posix error code; return nonzero if match.
#if defined(USING_WINSOCK)
#define ENOPROTOOPT 109
#define ECONNREFUSED 111
  // All codes to be checked for must be defined below
  int werr = WSAGetLastError();
  switch( werr ) {
  case WSAETIMEDOUT:
    return( perr == EAGAIN );
  case WSAENOPROTOOPT:
    return( perr == ENOPROTOOPT );
  case WSAECONNREFUSED:
    return( perr == ECONNREFUSED );
  default:
    fprintf(stderr,"gr_hiqsdr_source/is_error: unknown error %d\n", perr );
    throw std::runtime_error("internal error");
  }
  return 0;
#else
  return( perr == errno );
#endif
}

static void report_error( const char *msg1, const char *msg2 )
{
  // Deal with errors, both posix and winsock
#if defined(USING_WINSOCK)
  int werr = WSAGetLastError();
  fprintf(stderr, "%s: winsock error %d\n", msg1, werr );
#else
  perror(msg1);
#endif
  if( msg2 != NULL )
    throw std::runtime_error(msg2);
  return;
}

gr_hiqsdr_sink::gr_hiqsdr_sink (size_t itemsize, 
			  const char *host, unsigned short port,
			  int payload_size, bool eof)
  : gr_sync_block ("udp_sink",
		   gr_make_io_signature (1, 1, itemsize),
		   gr_make_io_signature (0, 0, 0)),
    d_itemsize (itemsize), d_payload_size(payload_size), d_eof(eof),
    d_socket(-1), d_connected(false)
{
#if defined(USING_WINSOCK) // for Windows (with MinGW)
  // initialize winsock DLL
  WSADATA wsaData;
  int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
  if( iResult != NO_ERROR ) {
    report_error( "gr_hiqsdr_source WSAStartup", "can't open socket" );
  }
#endif

  // create socket
  d_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(d_socket == -1) {
    report_error("socket open","can't open socket");
  }

  // Don't wait when shutting down
  linger lngr;
  lngr.l_onoff  = 1;
  lngr.l_linger = 0;
  if(setsockopt(d_socket, SOL_SOCKET, SO_LINGER, (optval_t)&lngr, sizeof(linger)) == -1) {
    if( !is_error(ENOPROTOOPT) ) {  // no SO_LINGER for SOCK_DGRAM on Windows
      report_error("SO_LINGER","can't set socket option SO_LINGER");
    }
  }

  // Get the destination address
  connect(host, port);
}

// public constructor that returns a shared_ptr

gr_hiqsdr_sink_sptr
gr_make_hiqsdr_sink (size_t itemsize, 
		  const char *host, unsigned short port,
		  int payload_size, bool eof)
{
  return gnuradio::get_initial_sptr(new gr_hiqsdr_sink (itemsize, 
					    host, port,
					    payload_size, eof));
}

gr_hiqsdr_sink::~gr_hiqsdr_sink ()
{
  if (d_connected)
    disconnect();

  if (d_socket != -1){
    shutdown(d_socket, SHUT_RDWR);
#if defined(USING_WINSOCK)
    closesocket(d_socket);
#else
    ::close(d_socket);
#endif
    d_socket = -1;
  }

#if defined(USING_WINSOCK) // for Windows (with MinGW)
  // free winsock resources
  WSACleanup();
#endif
}

int 
gr_hiqsdr_sink::work (int noutput_items,
		   gr_vector_const_void_star &input_items,
		   gr_vector_void_star &output_items)
{
    const char *in = (const char *) input_items[0];
    static unsigned char udpBuf[600*2+2];
    static int pos = 0;
    int i,itemsToSend = noutput_items;
    if (pos==0) {
	udpBuf[pos++]=0;
	udpBuf[pos++]=0;
    }

    gruel::scoped_lock guard(d_mutex);  // protect d_socket

    if(d_connected) {
	for (i=0;i<itemsToSend;i++) {
	    float re = *((float*)(in+i*sizeof(gr_complex)));
	    float im = *((float*)(in+i*sizeof(gr_complex)+4));
	    unsigned int i_re = re * 0x10000;
	    unsigned int i_im = im * 0x10000;
	    udpBuf[pos++] = (i_re>>0) & 0xff;
	    udpBuf[pos++] = (i_re>>8) & 0xff;
	    udpBuf[pos++] = (i_im>>0) & 0xff;
	    udpBuf[pos++] = (i_im>>8) & 0xff;
	    if (pos >= sizeof(udpBuf)) {
		send(d_socket, udpBuf, pos, 0);
		pos = 2;
	    }

	}
    }
    return noutput_items;
}

void gr_hiqsdr_sink::connect( const char *host, unsigned short port )
{
  if(d_connected)
    disconnect();

  if(host != NULL ) {
    // Get the destination address
    struct addrinfo *ip_dst;
    struct addrinfo hints;
    memset( (void*)&hints, 0, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    char port_str[12];
    sprintf( port_str, "%d", port );

    // FIXME leaks if report_error throws below
    int ret = getaddrinfo( host, port_str, &hints, &ip_dst );
    if( ret != 0 )
      report_error("gr_hiqsdr_source/getaddrinfo",
		   "can't initialize destination socket" );

    // don't need d_mutex lock when !d_connected
    if(::connect(d_socket, ip_dst->ai_addr, ip_dst->ai_addrlen) == -1) {
      report_error("socket connect","can't connect to socket");
    }
    d_connected = true;

    freeaddrinfo(ip_dst);
  }

  return;
}

void gr_hiqsdr_sink::disconnect()
{
  if(!d_connected)
    return;

  #if SNK_VERBOSE
  printf("gr_hiqsdr_sink disconnecting\n");
  #endif

  gruel::scoped_lock guard(d_mutex);  // protect d_socket from work()

  // Send a few zero-length packets to signal receiver we are done
  if(d_eof) {
    int i;
    for( i = 0; i < 3; i++ )
      (void) send( d_socket, NULL, 0, 0 );  // ignore errors
  }

  // Sending EOF can produce ERRCONNREFUSED errors that won't show up
  //  until the next send or recv, which might confuse us if it happens
  //  on a new connection.  The following does a nonblocking recv to
  //  clear any such errors.
  timeval timeout;
  timeout.tv_sec = 0;    // zero time for immediate return
  timeout.tv_usec = 0;
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(d_socket, &readfds);
  int r = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
  if(r < 0) {
      #if SNK_VERBOSE
      report_error("udp_sink/select",NULL);
      #endif
  }
  else if(r > 0) {  // call recv() to get error return
    r = recv(d_socket, (char*)&readfds, sizeof(readfds), 0);
    if(r < 0) {
	#if SNK_VERBOSE
	report_error("udp_sink/recv",NULL);
	#endif
    }
  }

  // Since I can't find any way to disconnect a datagram socket in Cygwin,
  // we just leave it connected but disable sending.
#if 0
  // zeroed address structure should reset connection
  struct sockaddr addr;
  memset( (void*)&addr, 0, sizeof(addr) );
  // addr.sa_family = AF_UNSPEC;  // doesn't work on Cygwin
  // addr.sa_family = AF_INET;  // doesn't work on Cygwin

  if(::connect(d_socket, &addr, sizeof(addr)) == -1)
    report_error("socket connect","can't connect to socket");
#endif

  d_connected = false;

  return;
}
