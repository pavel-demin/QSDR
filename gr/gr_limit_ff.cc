/* -*- c++ -*- */
/*
 * Copyright 2004,2010 Free Software Foundation, Inc.
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

#include <gr_limit_ff.h>
#include <gnuradio/io_signature.h>

gr_limit_ff_sptr gr_make_limit_ff (float k) {
  return gnuradio::get_initial_sptr (new gr_limit_ff (k));
}

gr_limit_ff::gr_limit_ff (float k)
  : gr::sync_block ("limit_ff",
		   gr::io_signature::make (1, 1, sizeof (float)),
		   gr::io_signature::make (1, 1, sizeof (float))),
    d_k (k)
{
}

int
gr_limit_ff::work (int noutput_items,
		   gr_vector_const_void_star &input_items,
		   gr_vector_void_star &output_items)
{
  float v;
  float *iptr = (float *) input_items[0];
  float *optr = (float *) output_items[0];

  int	size = noutput_items;

  while (size-- > 0) {
      v = *iptr++;
      if (v >  d_k) v= d_k;
      if (v < -d_k) v=-d_k;
      *optr++ = v;
  }
  
  return noutput_items;
}
