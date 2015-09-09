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
#include "trx_sink_impl.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


namespace gr {
	namespace redpittrx {

		sink::sptr sink::make(size_t itemsize, const char *host, unsigned short port, int payload_size) {
			return gnuradio::get_initial_sptr (new sink_impl(itemsize, host, port, payload_size));
		}

		/*
		 * The private constructor
		 */
		sink_impl::sink_impl(size_t itemsize, const char *host, unsigned short port, int payload_size)
			: gr::sync_block("sink", gr::io_signature::make(1, 1, itemsize), gr::io_signature::make(0, 0, 0)) {

				d_itemsize = itemsize;
				d_payload_size = payload_size;
				d_socket= -1;
				d_connected = false;
				printf("redpittrx sink contructor %s %d\n",host,port);

				// create socket
				d_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if(d_socket == -1)
					perror("redpittrx: socket open");

				// Don't wait when shutting down
				linger lngr;
				lngr.l_onoff  = 1;
				lngr.l_linger = 0;
				if(setsockopt(d_socket, SOL_SOCKET, SO_LINGER, &lngr, sizeof(linger)) == -1) {
					if(errno != ENOPROTOOPT)  {  // no SO_LINGER for SOCK_DGRAM on Windows
						perror("redpittrx: setsockopt");
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

			printf("redpittrx sink destructor\n");


			if (d_socket != -1){
				shutdown(d_socket, SHUT_RDWR);
				::close(d_socket);
				d_socket = -1;
			}
		}

		int sink_impl::work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items) {
#if 0
			float min=0.0;
			float max=0.0;
			float *v=(float *)input_items[0];
			for (int i=0; i<2*noutput_items; i++) {
				if (v[i] < min) min=v[i];
				if (v[i] > max) max=v[i];
			}
			printf ("%f %f\n",min,max);
#endif
			if(d_connected) {
				ssize_t n=send(d_socket,input_items[0],2*noutput_items*sizeof(float),MSG_NOSIGNAL);
				return n/(sizeof(float)*2);
			} else
				return 0;
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
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				char port_str[12];
				sprintf( port_str, "%d", port );

				int ret = getaddrinfo( host, port_str, &hints, &ip_dst );
				if( ret != 0 )
					perror("gr_redpittrx_source/getaddrinfo");

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

			d_connected = false;
			return;
		}

	} /* namespace redpittrx */
} /* namespace gr */

