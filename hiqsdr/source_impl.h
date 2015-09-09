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

#ifndef INCLUDED_HIQSDR_SOURCE_IMPL_H
#define INCLUDED_HIQSDR_SOURCE_IMPL_H

#include <hiqsdr/source.h>


namespace gr {
  namespace hiqsdr {

    class source_impl : public source
	  {
		  private:
			  size_t	d_itemsize;
			  int           d_payload_size;  // maximum transmission unit (packet length)
			  bool          d_eof;           // zero-length packet is EOF
			  int           d_socket;        // handle to socket
			  // gruel::mutex  d_mutex;           // protects d_socket and d_connected
			  unsigned char *d_temp_buff;    // hold buffer between  between call
			  int			byteMode;
			  unsigned char seqNr;
			  gr_complex outBuf[8192];
			  int outBufPos;
			  int rxRate,txRate;

			  unsigned char ctlBuf[22];
			  unsigned char firBuf[80];
			  int c_socket;
			  int rx_fir_socket;
			  int tx_fir_socket;
			  int firmVersion;
			  bool cwMode;
			  int clockCorr;

			  void hiqSend();
			  void closeSocket(int socket);
			  int openSocket (const char *host, unsigned short port);

			  //void connect( const char *host, unsigned short port );

			  void disconnect();
			  // Nothing to declare in this block.

		  public:
			  source_impl(size_t itemsize, const char *host,unsigned short port, unsigned short c_port, unsigned short rx_fir_port, 
					  unsigned short tx_fir_port, int payload_size,bool eof, bool wait, int rxfreq, int txfreq, int rate,bool ant, 
					  int presel, int att, int txLevel, bool ptt, int txRate, int clockCorr,
					  std::vector<gr_complex> rxFirTaps, std::vector<gr_complex> txFirTaps);
			  ~source_impl();

			  // Where all the action really happens
			  int work(int noutput_items,
					  gr_vector_const_void_star &input_items,
					  gr_vector_void_star &output_items);


			  int payload_size() { return d_payload_size; }
			  int get_port();
			  void setRXFreq(int f);
			  void setTXFreq(int f);
			  void setSampleRate(int rate);
			  void setAnt(bool);
			  void setPresel(int);
			  void setAttenuator(int);
			  void setTXLevel(int l);
			  void setPtt(bool on);
			  void setTXRate(int);
			  void setCWMode(bool);
			  void setFilter(std::vector<gr_complex> coef, int dest=0);
			  void setClockCorr(int diff);

	  };

  } // namespace hiqsdr
} // namespace gr

#endif /* INCLUDED_HIQSDR_SOURCE_IMPL_H */

