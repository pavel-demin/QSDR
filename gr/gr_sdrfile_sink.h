/* -*- c++ -*- */
/*
 * Copyright 2004,2007 Free Software Foundation, Inc.
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

#ifndef INCLUDED_GR_SDRFILE_SINK_H
#define INCLUDED_GR_SDRFILE_SINK_H

// #include <gr_core_api.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/blocks/file_sink_base.h>

class gr_sdrfile_sink;
typedef boost::shared_ptr<gr_sdrfile_sink> gr_sdrfile_sink_sptr;

 gr_sdrfile_sink_sptr gr_make_file_sink(size_t itemsize, const char *filename);

/*!
 * \brief Write stream to file.
 * \ingroup sink_blk
 */

class  gr_sdrfile_sink : public gr::sync_block, public gr::blocks::file_sink_base
{
  friend  gr_sdrfile_sink_sptr gr_make_file_sink(size_t itemsize, const char *filename);

 private:
  size_t	d_itemsize;

 protected:
  gr_sdrfile_sink(size_t itemsize, const char *filename);

 public:
  ~gr_sdrfile_sink();

  int work(int noutput_items,
	   gr_vector_const_void_star &input_items,
	   gr_vector_void_star &output_items);
};

#endif /* INCLUDED_GR_SDRFILE_SINK_H */
