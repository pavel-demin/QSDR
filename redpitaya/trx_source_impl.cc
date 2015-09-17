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

#include "trx_source_impl.h"

namespace gr {
	namespace redpittrx {

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
			: gr::sync_block("red_pitaya source",
					gr::io_signature::make(0, 0, 0),
					gr::io_signature::make(1, 1, itemsize))
		{
			printf("redpittrx source constructor %s %d %d\n",host,port,rate);
			d_itemsize = itemsize;
			d_datasock = d_ctlsock = -1;
			if (host != NULL) {
				struct addrinfo *ip_dst;
				struct addrinfo hints;
				memset( (void*)&hints, 0, sizeof(hints) );
				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				char port_str[12];
				sprintf( port_str, "%d", port );
				if (getaddrinfo( host, port_str, &hints, &ip_dst )!=0) {
					disconnect();
					perror("redpittrx: socket getaddrinfo");
					return;
				}
				if (connect(ip_dst, rate) < 0)
					disconnect();
				freeaddrinfo(ip_dst);
			}
		}

		int source_impl::connect (struct addrinfo *ip_dst, int rate) {
			uint32_t ctl=0;
			uint32_t data=1;
			d_ctlsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if(d_ctlsock == -1) {
				perror("redpittrx: socket open");
				return -1;
			}
			d_datasock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if(d_datasock == -1) {
				perror("redpittrx: socket open");
				return -1;
			}
			if(::connect(d_ctlsock, ip_dst->ai_addr, ip_dst->ai_addrlen) == -1) {
				perror("redpittrx: socket connect");
				return -1;
			}
			if (send(d_ctlsock, &ctl, sizeof(ctl), 0) !=  sizeof(ctl)) {
				perror("redpittrx: rx ctl send\n");
				return -1;
			}
			setRXRate(rate);
			if(::connect(d_datasock, ip_dst->ai_addr, ip_dst->ai_addrlen) == -1) {
				perror("redpittrx: socket connect");
				return -1;
			}
			if (send(d_datasock, &data, sizeof(data), 0) !=  sizeof(data)) {
				perror("redpittrx: rx ctl send\n");
				return -1;
			}
			return 0;
		}


		/*
		 * Our virtual destructor.
		 */
					source_impl::~source_impl() {
						printf("redpittrx source destructor \n");
						disconnect();
					}

				void source_impl::disconnect()
				{
					printf("redpittrx disconnect\n");
					if (d_datasock >= 0) {
						shutdown(d_datasock, SHUT_RDWR);
						close(d_datasock);
					}
					if (d_ctlsock >= 0) {
						shutdown(d_ctlsock, SHUT_RDWR);
						close(d_ctlsock);
					}
					d_datasock = d_ctlsock = -1;
					return;
				}

				int source_impl::work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items) {
					if (d_datasock >= 0) {
						ssize_t n=recv(d_datasock, output_items[0], 2*noutput_items*sizeof(int32_t), MSG_WAITALL);
						return n/(sizeof(float)*2);
					} else
						return 0;
				}

				// Return port number of d_socket
				int source_impl::get_port(void)
				{
					sockaddr_in name;
					socklen_t len = sizeof(name);
					int ret = getsockname( d_datasock, (sockaddr*)&name, &len );
					if( ret ) {
						perror("source_impl getsockname");
						return -1;
					}
					return ntohs(name.sin_port);
				}

				void source_impl::setRXFreq(int f) {
					if (d_ctlsock >= 0) {
						uint32_t msg= (f & 0x0fffffff);
						printf("redpittrx setRXFreq %d\n",f);
						send(d_ctlsock, &msg, sizeof(msg), 0);
					}
				}

				void source_impl::setRXRate(int rate) {
					if (d_ctlsock >= 0) {
						uint32_t msg=0x10000000;
						printf("redpittrx setSampleRate %d\n",rate);
						switch (rate) {
							case 20000: msg |= 0; break;
							case 50000: msg |= 1; break;
							case 100000: msg |= 2; break;
							case 250000: msg |= 3; break;
							case 500000: msg |= 4; break;
						}
						send(d_ctlsock, &msg, sizeof(msg), 0);
					}
				}

	} /* namespace redpittrx */
} /* namespace gr */

