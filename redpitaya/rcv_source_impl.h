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

#ifndef INCLUDED_REDPITRCV_SOURCE_IMPL_H
#define INCLUDED_REDPITRCV_SOURCE_IMPL_H

#include <redpitaya/rcv_source.h>

namespace gr {
  namespace redpitrcv {

    class source_impl : public source
	  {
		  private:
				size_t  d_itemsize;
				int           d_socket;        // handle to socket

			  void closeSocket(int socket);
			  int openSocket (const char *host, unsigned short port);
			  void disconnect();
			  // Nothing to declare in this block.

		  public:
			  source_impl(size_t itemsize, const char *host,unsigned short port, int rate);
			  ~source_impl();

			  // Where all the action really happens
			  int work(int noutput_items,
					  gr_vector_const_void_star &input_items,
					  gr_vector_void_star &output_items);

			  //int payload_size() { return d_payload_size; }
			  int get_port();
			  void setRXFreq(int f);
			  //void setTXFreq(int f);
			  void setSampleRate(int rate);
			  //void setAnt(bool);
			  //void setPresel(int);
			  //void setAttenuator(int);
			  //void setTXLevel(int l);
			  //void setPtt(bool on);
			  //void setTXRate(int);
			  //void setCWMode(bool);
			  //void setFilter(std::vector<gr_complex> coef, int dest=0);
			  //void setClockCorr(int diff);

	  };

  } // namespace redpitrcv
} // namespace gr

#endif /* INCLUDED_REDPITRCV_SOURCE_IMPL_H */

