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

#ifndef INCLUDED_GR_PSK31_DECODER_H
#define INCLUDED_GR_PSK31_DECODER_H

#include <gnuradio/sync_block.h>
#include <stddef.h>			// size_t


class gr_psk31_decoder;
typedef boost::shared_ptr<gr_psk31_decoder> gr_psk31_decoder_sptr;

gr_psk31_decoder_sptr
gr_make_psk31_decoder (size_t sizeof_stream_item, int rate);


class gr_psk31_decoder : public gr::sync_block
{

    friend gr_psk31_decoder_sptr gr_make_psk31_decoder (size_t sizeof_stream_item, int rate);

    public:
    gr_psk31_decoder (size_t sizeof_stream_item, int rate);

    virtual int work (int noutput_items,
	    gr_vector_const_void_star &input_items,
	    gr_vector_void_star &output_items);

    private:
    unsigned int d_vlen;
    int itemSize;

    int j,i,c;
    int n;
    unsigned int codes[4096];
    unsigned int ch;
    int bits;
    int sampleRate;
};

#endif 
