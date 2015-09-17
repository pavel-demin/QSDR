/* -*- c++ -*- */
/*
 * Copyright 2015 Renzo Davoli (modified for Red Pitaya)
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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "rcv_source_impl.h"

namespace gr {
	namespace redpitrcv {

		source::sptr
			source::make(size_t itemsize, const char *host, unsigned short port, int rate)
			{
				return gnuradio::get_initial_sptr
					(new source_impl(itemsize, host, port, rate));
			}

		/*
		 * The private constructor
		 */
		source_impl::source_impl(size_t itemsize, const char *host,unsigned short port, int rate)
			: gr::sync_block("tcp source",
					gr::io_signature::make(0, 0, 0),
					gr::io_signature::make(1, 1, itemsize))
		{
			printf("redpitrcv contructor %s %d %d\n",host,port,rate);
			d_itemsize = itemsize;
			d_socket = openSocket(host, port);
			setSampleRate(rate);
		}

		/*
		 * Our virtual destructor.
		 */
		source_impl::~source_impl() {
			printf("redpitrcv destructor \n");
			if (d_socket != 0){
				shutdown(d_socket, SHUT_RDWR);
				::close(d_socket);
			}
		}

#define SCALE (float)(2<<24)
		int source_impl::work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items) {
			gr_complex *out = (gr_complex *) output_items[0];

			int32_t data[2*noutput_items];
			ssize_t n=recv(d_socket, data, 2*noutput_items*sizeof(int32_t), MSG_WAITALL);
			int nelem=n/(sizeof(int32_t)*2);
			if (n<=0)
				return -1;
			for (int i=0; i<nelem; i++)
				out[i]=gr_complex(float(data[2*i])/SCALE,float(data[2*i+1])/SCALE);
			return nelem;
		}

		int source_impl::openSocket (const char *host, unsigned short port) {
			int sock;

			sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if(sock == -1) {
				perror("redpitrcv: socket open");
				return 0;
			}

			// Don't wait when shutting down
			linger lngr;
			lngr.l_onoff  = 1;
			lngr.l_linger = 0;
			if(setsockopt(sock, SOL_SOCKET, SO_LINGER, &lngr, sizeof(linger)) == -1) {
				if( errno != ENOPROTOOPT) {  // no SO_LINGER for SOCK_DGRAM on Windows
					perror("redpitrcv: setsockopt");
					return 0;
				}
			}

			if(host != NULL ) {
				// Get the destination address
				struct addrinfo *ip_dst;
				struct addrinfo hints;
				memset( (void*)&hints, 0, sizeof(hints) );
				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				char port_str[12];
				sprintf( port_str, "%d", port );

				if (getaddrinfo( host, port_str, &hints, &ip_dst )!=0) {
					perror("redpitrcv: socket getaddrinfo");
					return 0;
				}

				if(::connect(sock, ip_dst->ai_addr, ip_dst->ai_addrlen) == -1) {
					perror("redpitrcv: socket connect");
					return 0;
				}

				freeaddrinfo(ip_dst);

			}
			return sock;
		}

		// Return port number of d_socket
		int source_impl::get_port(void)
		{
			sockaddr_in name;
			socklen_t len = sizeof(name);
			int ret = getsockname( d_socket, (sockaddr*)&name, &len );
			if( ret ) {
				perror("source_impl getsockname");
				return -1;
			}
			return ntohs(name.sin_port);
		}


		void source_impl::closeSocket(int socket) {
			// gruel::scoped_lock guard(d_mutex);  // protect d_socket from work()

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
				perror("redpitrcv select");
#endif
			}
			else if(r > 0) {  // call recv() to get error return
				r = recv(socket, (char*)&readfds, sizeof(readfds), 0);
				if(r < 0) {
#if SNK_VERBOSE
					perror("redpitrcv recv");
#endif
				}
			}
		}

		void source_impl::disconnect()
		{
			printf("redpitrcv disconnect\n");
			closeSocket(d_socket);
			return;
		}

		void source_impl::setRXFreq(int f) {
			uint32_t msg= (f & 0x3fffffff);
			printf("redpitrcv setRXFreq %d\n",f);
			send(d_socket, &msg, sizeof(msg), 0);
		}

		void source_impl::setSampleRate(int rate) {
			uint32_t msg=0x80000000;
			printf("redpitrcv setSampleRate %d\n",rate);
			switch (rate) {
				case 50000: msg |= 0; break;
				case 100000: msg |= 1; break;
				case 250000: msg |= 2; break;
				case 500000: msg |= 3; break;
			}
			send(d_socket, &msg, sizeof(msg), 0);
		}

	} /* namespace redpitrcv */
} /* namespace gr */

