/* -*- c++ -*- */
/* 
 * Copyright 2013 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "sink_impl.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


// #include <gruel/thread.h>

extern int gr_hiqsdr_tx_samples;

namespace gr {
	namespace hiqsdr {

		sink::sptr sink::make(size_t itemsize, const char *host, unsigned short port, int payload_size, bool eof) {
			return gnuradio::get_initial_sptr (new sink_impl(itemsize, host, port, payload_size, eof));
		}

		/*
		 * The private constructor
		 */
		sink_impl::sink_impl(size_t itemsize, const char *host, unsigned short port, int payload_size, bool eof) 
			: gr::sync_block("sink", gr::io_signature::make(1, 1, itemsize), gr::io_signature::make(0, 0, 0)) {

				d_itemsize = itemsize;
				d_payload_size = payload_size; 
				d_eof= eof;
				d_socket= -1;
				d_connected = false;

				// create socket
				d_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
				if(d_socket == -1) 
					perror("hiqsdr: socket open");

				// Don't wait when shutting down
				linger lngr;
				lngr.l_onoff  = 1;
				lngr.l_linger = 0;
				if(setsockopt(d_socket, SOL_SOCKET, SO_LINGER, &lngr, sizeof(linger)) == -1) {
					if(errno != ENOPROTOOPT)  {  // no SO_LINGER for SOCK_DGRAM on Windows
						perror("hiqsdr: setsockopt");
					}
				}

				// Get the destination address
				connect(host, port);
			}

		/*
		 * Our virtual destructor.
		 */
		sink_impl::~sink_impl() {
			if (d_connected)
				disconnect();

			if (d_socket != -1){
				shutdown(d_socket, SHUT_RDWR);
				::close(d_socket);
				d_socket = -1;
			}
		}

		int sink_impl::work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items) {
			const char *in = (const char *) input_items[0];
			static unsigned char udpBuf[600*2+2];
			static int pos = 0;
			int i,items = 0;
			// printf ("tx pos %i samples %i outitems %i\n",pos,gr_hiqsdr_tx_samples, noutput_items);
			if (pos==0) {
				udpBuf[pos++]=0;
				udpBuf[pos++]=0;
			}

			if (gr_hiqsdr_tx_samples <= 0) {
				usleep(1000);
				return 0;
			}

			// gruel::scoped_lock guard(d_mutex);  // protect d_socket

			if(d_connected) {
				for (i=0;i<noutput_items;i++) {
					if (pos >= sizeof(udpBuf)) {
						if (gr_hiqsdr_tx_samples >= (pos-2)/4) {
							gr_hiqsdr_tx_samples -= (pos-2)/4;
							send(d_socket, udpBuf, pos, 0);
							// printf ("tx send items %i\n",pos);
							pos = 2;
						} else {
							// printf ("tx items abort  %i %i\n",gr_hiqsdr_tx_samples, items);
							usleep(1000);
							return items;
						}

					}
					float re = *((float*)(in+i*sizeof(gr_complex)));
					float im = *((float*)(in+i*sizeof(gr_complex)+4));
					if (re> 0.9999) re= 0.9999;
					if (re<-0.9999) re=-0.9999;
					if (im> 0.9999) im= 0.9999;
					if (im<-0.9999) im=-0.9999;
					int i_re = re * 0x8000 + 0.5;
					int i_im = im * 0x8000 + 0.5;
					udpBuf[pos++] = (i_re>>0) & 0xff;
					udpBuf[pos++] = (i_re>>8) & 0xff;
					udpBuf[pos++] = (i_im>>0) & 0xff;
					udpBuf[pos++] = (i_im>>8) & 0xff;
					items++;
					// printf("%f %f %02x%02x %02x%02x\n",re,im,udpBuf[pos-3],udpBuf[pos-4],udpBuf[pos-1],udpBuf[pos-2]);

				}
			}
			// printf ("tx items %i %i\n",gr_hiqsdr_tx_samples, items);
			return items;
		}

		void sink_impl::connect( const char *host, unsigned short port ) {
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

				int ret = getaddrinfo( host, port_str, &hints, &ip_dst );
				if( ret != 0 )
					perror("gr_hiqsdr_source/getaddrinfo");

				if(::connect(d_socket, ip_dst->ai_addr, ip_dst->ai_addrlen) == -1) {
					perror("socket connect");
				}
				d_connected = true;

				freeaddrinfo(ip_dst);
			}
			return;
		}

		void sink_impl::disconnect() {
			if(!d_connected)
				return;

#if SNK_VERBOSE
			printf("gr_hiqsdr_sink disconnecting\n");
#endif

			// gruel::scoped_lock guard(d_mutex);  // protect d_socket from work()

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
				perror("udp_sink/select");
#endif
			}
			else if(r > 0) {  // call recv() to get error return
				r = recv(d_socket, (char*)&readfds, sizeof(readfds), 0);
				if(r < 0) {
#if SNK_VERBOSE
					perror("udp_sink/recv");
#endif
				}
			}
			d_connected = false;

			return;
		}

	} /* namespace hiqsdr */
} /* namespace gr */

