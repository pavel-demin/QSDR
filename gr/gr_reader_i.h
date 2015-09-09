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

#ifndef INCLUDED_GR_READER_I_H
#define INCLUDED_GR_READER_I_H

#include <gnuradio/sync_block.h>
#include <stddef.h>			// size_t

class Parent;
typedef void(Parent::*cbF)(int);


class gr_reader_i;
typedef boost::shared_ptr<gr_reader_i> gr_reader_i_sptr;

gr_reader_i_sptr
gr_make_reader_i (Parent*, cbF f);


class gr_reader_i : public gr::sync_block
{

    friend gr_reader_i_sptr gr_make_reader_i (Parent*, cbF f);

    public:
    gr_reader_i (Parent*, cbF f);

    virtual int work (int noutput_items,
	    gr_vector_const_void_star &input_items,
	    gr_vector_void_star &output_items);

    private:
    cbF cb;
    Parent *parent;

    int j,i,c;
    int n;
    unsigned int codes[4096];
    unsigned int ch;
    int bits;
    int sampleRate;
};

#endif 
