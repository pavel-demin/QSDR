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

#ifndef INCLUDED_REDPITTRX_SINK_IMPL_H
#define INCLUDED_REDPITTRX_SINK_IMPL_H

#include <redpitaya/trx_sink.h>

namespace gr {
  namespace redpittrx {

    class sink_impl : public sink
    {
     private:
		 size_t	d_itemsize;

		 int           d_payload_size;    // maximum transmission unit (packet length)
		 int           d_socket;          // handle to socket
		 bool          d_connected;       // are we connected?
		 //gruel::mutex  d_mutex;           // protects d_socket and d_connected

     public:
      sink_impl(size_t itemsize, const char *host, unsigned short port, int payload_size);
      ~sink_impl();

      // Where all the action really happens
      int work(int noutput_items,
	       gr_vector_const_void_star &input_items,
	       gr_vector_void_star &output_items);
	  int payload_size() { return d_payload_size; }
	  void connect( const char *host, unsigned short port );
	  void disconnect();
    };

  } // namespace redpittrx
} // namespace gr

#endif /* INCLUDED_HIQSDR_SINK_IMPL_H */
