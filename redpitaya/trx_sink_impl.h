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

#ifndef INCLUDED_REDPITTRX_SINK_IMPL_H
#define INCLUDED_REDPITTRX_SINK_IMPL_H

#include <redpitaya/trx_sink.h>
#include <netdb.h>

namespace gr {
	namespace redpittrx {
		class sink_impl : public sink
		{
			private:
				size_t	d_itemsize;
				int           d_datasock;          // handle to data socket
				int           d_ctlsock;          // handle to ctl socket
				int connect (struct addrinfo *ip_dst, int rate);
				void disconnect();

			public:
				sink_impl(size_t itemsize, const char *host, unsigned short port, int rate);
				~sink_impl();

				void setPtt(bool on);
				void setTXFreq(int f);
				void setTXRate(int rate);

				// Where all the action really happens
				int work(int noutput_items,
						gr_vector_const_void_star &input_items,
						gr_vector_void_star &output_items);
		};

	} // namespace redpittrx
} // namespace gr

#endif /* INCLUDED_HIQSDR_SINK_IMPL_H */
