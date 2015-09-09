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
#include <gr_hiqsdr_source.h>
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
#include <sys/socket.h>
#endif
typedef void* optval_t;

// ntohs() on FreeBSD may require both netinet/in.h and arpa/inet.h, in order
#if defined(HAVE_NETINET_IN_H)
#include <netinet/in.h>
#endif
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>
#endif
#endif

#define USE_SELECT    	1  // non-blocking receive on all platforms

static int is_error( int perr )
{
	// Compare error to posix error code; return nonzero if match.
	return( perr == errno );
}

static void report_error( const char *msg1, const char *msg2 )
{
	// Deal with errors, both posix and winsock
	perror(msg1);
	if( msg2 != NULL )
		throw std::runtime_error(msg2);
	return;
}

int gr_hiqsdr_source::openSocket (const char *host, unsigned short port) {
	int sock;
	// --- data-socket

	// create socket
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock == -1) {
		report_error("socket open","can't open socket");
		return 0;
	}

	// Don't wait when shutting down
	linger lngr;
	lngr.l_onoff  = 1;
	lngr.l_linger = 0;
	if(setsockopt(sock, SOL_SOCKET, SO_LINGER, (optval_t)&lngr, sizeof(linger)) == -1) {
		if( !is_error(ENOPROTOOPT) ) {  // no SO_LINGER for SOCK_DGRAM on Windows
			report_error("SO_LINGER","can't set socket option SO_LINGER");
			return 0;
		}
	}

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

		if (getaddrinfo( host, port_str, &hints, &ip_dst )!=0) {
			report_error("socket getaddrinfo","getaddrinfo");
			return 0;
		}

		if(::connect(sock, ip_dst->ai_addr, ip_dst->ai_addrlen) == -1) {
			report_error("socket connect","can't connect to socket");
			return 0;
		}

		freeaddrinfo(ip_dst);

	}
	return sock;
}


gr_hiqsdr_source::gr_hiqsdr_source(size_t itemsize, const char *host, 
		unsigned short port, unsigned short c_port, unsigned short rx_fir_port, unsigned short tx_fir_port, int payload_size,
		bool eof, bool wait, int rxfreq, int txfreq, int rate,
		bool ant, int presel, int att, int txLevel, bool ptt, int txRate, int clockDiff, 
		std::vector<gr_complex> rxFirTaps, std::vector<gr_complex> txFirTaps):
	gr_sync_block ("udp_source", gr_make_io_signature(0, 0, 0),
			gr_make_io_signature(1, 1, itemsize)),
	d_itemsize(itemsize), d_payload_size(payload_size),
	d_eof(eof), d_socket(-1)
{

	cwMode = false;
	clockCorr = clockDiff;
	firmVersion = 1; 
	memset(ctlBuf, 0, sizeof(ctlBuf));
	strncpy((char*)ctlBuf, "St", 2);
	ctlBuf[13]=firmVersion;

	d_temp_buff = new unsigned char[d_payload_size];   // allow it to hold up to payload_size bytes

	d_socket = openSocket(host, port); 
	c_socket = openSocket(host, c_port); 
	rx_fir_socket = openSocket(host, rx_fir_port); 
	tx_fir_socket = openSocket(host, tx_fir_port); 

	for (int i=0;i<3;i++) {
		(void) send( d_socket, "rr", 2, 0 );  // send return addr 
		usleep(2000);
	}
	
	outBufPos = 0;

	setRXFreq(rxfreq);
	setTXFreq(txfreq);
	setSampleRate(rate);
	setAnt(ant);
	setPresel(presel);
	setAttenuator(att);
	setTXLevel(txLevel);
	setTXRate(txRate);
	setPtt(ptt);
	setFilter(rxFirTaps, FILTER_RX);
	setFilter(txFirTaps, FILTER_TX);
}

gr_hiqsdr_source_sptr gr_make_hiqsdr_source (size_t itemsize, const char *ipaddr, 
		unsigned short port, unsigned short c_port, unsigned short rx_fir_port, unsigned short tx_fir_port, int payload_size, 
		bool eof, bool wait, int rxfreq, int txfreq, int rate,
		bool ant, int presel, int att, int txLevel, bool ptt, int txRate, int clockCorr,
		std::vector<gr_complex> rxFirTaps, std::vector<gr_complex> txFirTaps)
{
	return gnuradio::get_initial_sptr(new gr_hiqsdr_source (itemsize, ipaddr, 
				port, c_port, rx_fir_port, tx_fir_port, payload_size, eof, wait, rxfreq, txfreq, 
				rate, ant, presel, att, txLevel, ptt, txRate, clockCorr, rxFirTaps, txFirTaps));
}

gr_hiqsdr_source::~gr_hiqsdr_source ()
{
	disconnect();
	delete [] d_temp_buff;

	if (d_socket != 0){
		shutdown(d_socket, SHUT_RDWR);
		::close(d_socket);
	}

	if (c_socket != 0){
		shutdown(d_socket, SHUT_RDWR);
		::close(d_socket);
	}

	if (rx_fir_socket != 0){
		shutdown(rx_fir_socket, SHUT_RDWR);
		::close(rx_fir_socket);
	}

	if (tx_fir_socket != 0){
		shutdown(tx_fir_socket, SHUT_RDWR);
		::close(tx_fir_socket);
	}
}

int gr_hiqsdr_source::work (int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items) {
	gr_complex *out = (gr_complex *) output_items[0];
	ssize_t r=0;
	int i;
	int ret=0;

	while (1) {
#if USE_SELECT
		// RCV_TIMEO doesn't work on all systems (e.g., Cygwin)
		// use select() instead of, or in addition to RCV_TIMEO
		fd_set readfds;
		timeval timeout;
		timeout.tv_sec = 1;	  // Init timeout each iteration.  Select can modify it.
		timeout.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(d_socket, &readfds);
		r = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
		if(r < 0) {
			report_error("udp_source/select",NULL);
			return -1;
		}
		else if(r == 0 ) {  // timed out
			return -1;
		}
#endif // USE_SELECT

		// This is a non-blocking call with a timeout set in the constructor
		r = recv(d_socket, d_temp_buff, d_payload_size, 0);  // get the entire payload or the what's available

		// Check if there was a problem; forget it if the operation just timed out
		if(r == -1) {
			if( is_error(EAGAIN) ) {  // handle non-blocking call timeout
				return -1;
			}
			else {
				report_error("udp_source/recv",NULL);
				return -1;
			}
		} else {
			if (++seqNr != d_temp_buff[0])
				printf("%i: lost packets %u\n",seqNr, abs((unsigned char)d_temp_buff[0]-seqNr));
			seqNr =  d_temp_buff[0];
			// Calculate the number of bytes we can take from the buffer in this call
			r-=2;	// skip header
			int bi = 2;
			static const int NBITS = 32;          // # of bits in a sample

			if (byteMode==1) {
				for (i=0;i < r/2; i++) {
					// 8 Bit -> Float 
					int re = d_temp_buff[bi+0]<<24;
					int im = d_temp_buff[bi+1]<<24;
					re = re >> 8;
					im = im >> 8;
					float ref = (float) re * (1.0 / (float) ((1LL << (NBITS-1)) - 1));
					float imf = (float) im * (1.0 / (float) ((1LL << (NBITS-1)) - 1));
					bi += 2;
					outBuf[outBufPos++] = gr_complex(ref, imf);
					if (outBufPos >= noutput_items) {
						memcpy(out, outBuf, noutput_items*sizeof(gr_complex));
						if (outBufPos - noutput_items > 0)
							memmove(outBuf, &outBuf[noutput_items], (outBufPos - noutput_items)*sizeof(gr_complex));
						outBufPos -= noutput_items;
						ret = noutput_items;
					}
				}
				if (ret == 0)
					continue;
				return ret;
			} else  if (byteMode == 2) {
				// 16 Bit -> Float 
				for (i=0;i < r/4; i++) {
					int re = (d_temp_buff[bi+0]<<16) + (d_temp_buff[bi+1]<<24);
					int im = (d_temp_buff[bi+2]<<16) + (d_temp_buff[bi+3]<<24);
					float ref = (float) re * (1.0 / (float) ((1LL << (NBITS-1)) - 1));
					float imf = (float) im * (1.0 / (float) ((1LL << (NBITS-1)) - 1));
					bi += 4;
					outBuf[outBufPos++] = gr_complex(ref, imf);
					if (outBufPos >= noutput_items) {
						memcpy(out, outBuf, noutput_items*sizeof(gr_complex));
						if (outBufPos - noutput_items > 0)
							memmove(outBuf, &outBuf[noutput_items], (outBufPos - noutput_items)*sizeof(gr_complex));
						outBufPos -= noutput_items;
						ret = noutput_items;
					} 
				}
				if (ret == 0)
					continue;
				return ret;
			} else {
				for (i=0;i < r/6; i++) {
					// 24 Bit -> Float 
					int re = (d_temp_buff[bi+0]<<8) + (d_temp_buff[bi+1]<<16) + (d_temp_buff[bi+2]<<24);
					int im = (d_temp_buff[bi+3]<<8) + (d_temp_buff[bi+4]<<16) + (d_temp_buff[bi+5]<<24);
					float ref = (float) re * (1.0 / (float) ((1LL << (NBITS-1)) - 1));
					float imf = (float) im * (1.0 / (float) ((1LL << (NBITS-1)) - 1));
					bi += 6;
					outBuf[outBufPos++] = gr_complex(ref, imf);
					if (outBufPos >= noutput_items) {
						memcpy(out, outBuf, noutput_items*sizeof(gr_complex));
						if (outBufPos - noutput_items > 0) 
							memmove(outBuf, &outBuf[noutput_items], (outBufPos - noutput_items)*sizeof(gr_complex));
						outBufPos -= noutput_items;
						ret = noutput_items;
					} 
				}
				if (ret == 0)
					continue;
				return ret;
			}
		}
	}

	return -1;
}

// Return port number of d_socket
int gr_hiqsdr_source::get_port(void)
{
	sockaddr_in name;
	socklen_t len = sizeof(name);
	int ret = getsockname( d_socket, (sockaddr*)&name, &len );
	if( ret ) {
		report_error("gr_hiqsdr_source/getsockname",NULL);
		return -1;
	}
	return ntohs(name.sin_port);
}


void gr_hiqsdr_source::closeSocket(int socket) {
	gruel::scoped_lock guard(d_mutex);  // protect d_socket from work()

	// Sending EOF can produce ERRCONNREFUSED errors that won't show up
	//  until the next send or recv, which might confuse us if it happens
	//  on a new connection.  The following does a nonblocking recv to
	//  clear any such errors.
	timeval timeout;
	timeout.tv_sec = 0;    // zero time for immediate return
	timeout.tv_usec = 0;
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(socket, &readfds);
	int r = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
	if(r < 0) {
#if SNK_VERBOSE
		report_error("udp_sink/select",NULL);
#endif
	}
	else if(r > 0) {  // call recv() to get error return
		r = recv(socket, (char*)&readfds, sizeof(readfds), 0);
		if(r < 0) {
#if SNK_VERBOSE
			report_error("udp_sink/recv",NULL);
#endif
		}
	}
}

void gr_hiqsdr_source::disconnect()
{
	(void) send( d_socket, "ss", 2, 0 );  // stop 

	// Send a few zero-length packets to signal receiver we are done
	if(d_eof) {
		int i;
		for( i = 0; i < 3; i++ )
			(void) send( d_socket, NULL, 0, 0 );  // ignore errors
	}
	usleep(1000);

	closeSocket(d_socket);
	closeSocket(c_socket);
	closeSocket(rx_fir_socket);
	closeSocket(tx_fir_socket);

	return;
}

void gr_hiqsdr_source::hiqSend() {
	if (ctlBuf[13]>0)
		send(c_socket, ctlBuf, 22, 0 );
	else
		send(c_socket, ctlBuf, 14, 0 );
}

void gr_hiqsdr_source::setRXFreq(int f) {
	f = f % RX_CLOCK;
	if (f > RX_CLOCK/2)
		f = RX_CLOCK - f;

	unsigned int ph = ((int)((float)(f) / (RX_CLOCK+clockCorr)*0x100000000LL+0.5)) & 0xffffffff;

	ctlBuf[2] = (ph >> 0 ) & 0xff; 
	ctlBuf[3] = (ph >> 8 ) & 0xff; 
	ctlBuf[4] = (ph >> 16) & 0xff; 
	ctlBuf[5] = (ph >> 24) & 0xff; 

	hiqSend();
}

void gr_hiqsdr_source::setTXFreq(int f) {
	unsigned int ph = ((int)((float)(f) / (RX_CLOCK+clockCorr)*0x100000000LL+0.5)) & 0xffffffff;

	ctlBuf[6] = (ph >> 0 ) & 0xff; 
	ctlBuf[7] = (ph >> 8 ) & 0xff; 
	ctlBuf[8] = (ph >> 16) & 0xff; 
	ctlBuf[9] = (ph >> 24) & 0xff; 

	hiqSend();
}

void gr_hiqsdr_source::setSampleRate(int rate) {
	byteMode = 3;
	switch (rate) {
		case 8000:
		case 9600:
		case 12000:
		case 16000:
		case 19200:
		case 24000:
		case 38400:
			ctlBuf[12] = (2<<6) | ((RX_CLOCK / 320 / rate) - 1);
			hiqSend();
			break;
		case 48000:
		case 60000:
		case 96000:
		case 120000:
		case 192000:
		case 240000:
		case 320000:
		case 384000:
		case 480000:
		case 640000:
		case 960000:
			ctlBuf[12] = (RX_CLOCK / 64 / rate) - 1;
			hiqSend();
			break;
		case 1280000:
		case 1536000:
		case 1920000:
			byteMode = 2;
			ctlBuf[12] = (1<<6) | ((RX_CLOCK / 16 / rate) - 1);
			hiqSend();
			break;
		case 2560000:
		case 3840000:
			byteMode = 1;
			ctlBuf[12] = (3<<6) | ((RX_CLOCK / 16 / rate) - 1);
			hiqSend();
			break;
		default:
			;
	}
}

void gr_hiqsdr_source::setPtt(bool on) {
	ctlBuf[11] &= 0xf0;
	ctlBuf[11] |= (cwMode ? TX_CW: TX_SSB); 
	if (on) 
		ctlBuf[11] |=  TX_PTT; 
	if (firmVersion>0) 
		ctlBuf[11] |= USE_EXT_IO; 
	hiqSend();
}

void gr_hiqsdr_source::setCWMode(bool cw) {
	ctlBuf[11] &= 0xf0;
	cwMode = cw;
	ctlBuf[11] |= (cwMode ? TX_CW: TX_SSB); 
	if (firmVersion>0) 
		ctlBuf[11] |= USE_EXT_IO; 
	hiqSend();
}


void gr_hiqsdr_source::setTXRate(int rate) {
	ctlBuf[11] &= 0x0f;
	switch (rate) {
		case 192000:
			ctlBuf[11] |= 0x10;
			break;
		case 480000:
			ctlBuf[11] |= 0x20;
			break;
		case 8000:
			ctlBuf[11] |= 0x30;
			break;
		default:
			;
	}
	hiqSend();
}

void gr_hiqsdr_source::setTXLevel(int l) {
	ctlBuf[10] = l;
	hiqSend();
}

void gr_hiqsdr_source::setAttenuator(int n) {
	ctlBuf[15]=0;
	if (n>0) 
		ctlBuf[15] = ((n/10)<<3) + (((n % 10)>>2)&7) ; 
	hiqSend();
}

void gr_hiqsdr_source::setPresel(int n) {

	ctlBuf[14] = n; 
	hiqSend();
}

void gr_hiqsdr_source::setAnt(bool on) {
	if (on)
		ctlBuf[16] |= HIQ_ANT; 
	else
		ctlBuf[16] &= ~HIQ_ANT; 
	hiqSend();
}

void gr_hiqsdr_source::setClockCorr(int diff) {
	clockCorr = diff;
}

#define COEF_LEN	5
#define COEF_CNT	4

void gr_hiqsdr_source::setFilter(std::vector<gr_complex> coef, int dest) {
	float re,im;
	unsigned int ire,iim;
	int reset=0x80;
	int cnt=0;
	int taps=0;
	int socket;
	unsigned long long tmp;
	float sum_re=0,sum_im=0;

	printf ("set filter tabs %u dest: %i\n",(unsigned int)coef.size(),dest);

	if (coef.size() == 0)
		return;

	if (dest==FILTER_RX)
		socket = rx_fir_socket;
	else
		socket = tx_fir_socket;

	memset(firBuf, 0, sizeof(firBuf));
	for (std::vector<gr_complex>::iterator it = coef.begin(); it != coef.end(); ++it) {
		re = (it->real()*((1<<17)-1));
		im = (it->imag()*((1<<17)-1));
		//printf ("%f + %f i\n",it->real(),it->imag());
		sum_re += it->real();
		sum_im += it->imag();

		if (re>0) re+= 0.5;
		if (re<0) re-= 0.5;
		if (im>0) im+= 0.5;
		if (im<0) im-= 0.5;

		ire = re;
		iim = im;
		tmp =  iim & 0x3ffff;
		tmp = (tmp << 18);
		tmp |= ire & 0x3ffff;
		firBuf[cnt*COEF_LEN+0] = tmp >>  0;
		firBuf[cnt*COEF_LEN+1] = tmp >>  8;
		firBuf[cnt*COEF_LEN+2] = tmp >> 16;
		firBuf[cnt*COEF_LEN+3] = tmp >> 24;
		firBuf[cnt*COEF_LEN+4] = tmp >> 32 | reset;
		//	printf ("%f %f %u %lx\n",it->real(),re, ire, tmp);
		reset = 0;
		cnt++;
		if (cnt==COEF_CNT) {
			send(socket, firBuf, COEF_CNT * COEF_LEN, 0 );
			memset(firBuf, 0, sizeof(firBuf));
			cnt = 0;
			usleep(1000);
		}
		taps++;
	}
	if (cnt<COEF_CNT) {
		send(socket, firBuf, COEF_CNT * COEF_LEN, 0 );
		taps+=(COEF_CNT-cnt);
	}
	memset(firBuf, 0, sizeof(firBuf));
	if (dest == FILTER_RX) {
		// allign
		if (taps % 8 > 0) {
			send(socket, firBuf, COEF_CNT * COEF_LEN, 0 );
			taps += COEF_CNT;
		}
		// last 4 coefficients have to zero
		for (int i=0;i<8*4;i++)
			send(socket, firBuf, COEF_CNT * COEF_LEN, 0 );
	}
	printf ("tabs %i, sum re %f sum im %f\n",taps, sum_re, sum_im);

}
