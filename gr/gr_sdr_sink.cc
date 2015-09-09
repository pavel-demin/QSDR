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

#include <gr_sdr_sink.h>
#include <gnuradio/io_signature.h>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"


gr_sdr_sink::gr_sdr_sink (size_t vlen, Parent *p, cbF f, float *fp)
  : gr::sync_block ("sdr_sink", gr::io_signature::make (1, 1, vlen * sizeof(gr_complex)), gr::io_signature::make (0, 0, 0))
{
    parent = p;
    d_vlen = vlen;
    outData = fp;
    cb = f;
}

gr_sdr_sink_sptr gr_make_sdr_sink (size_t vlen, Parent *p, cbF f, float *fp)
{
  return gnuradio::get_initial_sptr (new gr_sdr_sink (vlen, p, f, fp));
}

int gr_sdr_sink::work (int noutput_items,
		    gr_vector_const_void_star &input_items,
		    gr_vector_void_star &output_items)
{

  const gr_complex *in = (const gr_complex *) input_items[0];

  for (int i = 0; i < (int)d_vlen; i++){
      const float __x = in[i].real();
      const float __y = in[i].imag();
      outData[i] = sqrt(__x * __x + __y * __y);
  }
  if (cb)
      (parent->*cb)(noutput_items);
  return noutput_items;


}
