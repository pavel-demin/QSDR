/* -*- c++ -*- */
/*
 * Copyright 2004 Free Software Foundation, Inc.
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

#ifndef INCLUDED_GR_LIMIT_FF_H
#define INCLUDED_GR_LIMIT_FF_H

// #include <gnuradio/core_api.h>
#include <gnuradio/sync_block.h>

class gr_limit_ff;
typedef boost::shared_ptr<gr_limit_ff> gr_limit_ff_sptr;

 gr_limit_ff_sptr gr_make_limit_ff (float k);

class  gr_limit_ff : public gr::sync_block
{
  friend  gr_limit_ff_sptr gr_make_limit_ff (float k);

  float	d_k;		// the constant
  gr_limit_ff (float k);

 public:
  float k () const { return d_k; }
  void set_k (float k) { d_k = k; }

  int work (int noutput_items,
	    gr_vector_const_void_star &input_items,
	    gr_vector_void_star &output_items);
};

#endif
