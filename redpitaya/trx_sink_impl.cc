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
#include "trx_sink_impl.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


namespace gr {
	namespace redpittrx {

		sink::sptr sink::make(size_t itemsize, const char *host, unsigned short port, int rate) {
			return gnuradio::get_initial_sptr (new sink_impl(itemsize, host, port, rate));
		}

		/*
		 * The private constructor
		 */
		sink_impl::sink_impl(size_t itemsize, const char *host, unsigned short port, int rate)
			: gr::sync_block("red_pitaya sink",
					gr::io_signature::make(1, 1, itemsize),
					gr::io_signature::make(0, 0, 0)) {

				printf("redpittrx sink contructor %s %d\n",host,port);
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

		int sink_impl::connect (struct addrinfo *ip_dst, int rate) {
			uint32_t ctl=2;
			uint32_t data=3;
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
			setTXRate(rate);
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
		sink_impl::~sink_impl() {
			printf("redpittrx sink destructor\n");
			disconnect();
		}

		void sink_impl::disconnect()
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
			if(d_datasock >= 0) {
				ssize_t n=send(d_datasock,input_items[0],2*noutput_items*sizeof(float),MSG_NOSIGNAL);
				return n/(sizeof(float)*2);
			} else
				return 0;
		}

		void sink_impl::setTXFreq(int f) {
			if (d_ctlsock >= 0) {
				uint32_t msg= (f & 0x0fffffff);
				printf("redpittrx setTXFreq %d\n",f);
				send(d_ctlsock, &msg, sizeof(msg), 0);
			}
		}

		void sink_impl::setTXRate(int rate) {
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

		void sink_impl::setPtt(bool on) {
			if (d_ctlsock >= 0) {
				uint32_t msg = (on)?0x20000000:0x30000000;
				printf("PTT=%s\n",(on)?"on":"off");
				send(d_ctlsock, &msg, sizeof(msg), 0);
			}
		}

	} /* namespace redpittrx */
} /* namespace gr */

