/* -*- c++ -*- */
/*
 * Copyright 2007,2008,2009,2010 Free Software Foundation, Inc.
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

#ifndef INCLUDED_GR_HIQSDR_SOURCE_H
#define INCLUDED_GR_HIQSDR_SOURCE_H

#include <hiqsdr_api.h>
#include <gr_sync_block.h>
#include <gruel/thread.h>

#define RX_CLOCK	122880000LL
#define RX_EXT			0x80

#define TX_OFF      0
#define TX_CW       1
#define TX_SSB      2
#define USE_EXT_IO  4
#define TX_PTT      8

#define HIQ_PREAMP      0x10
#define HIQ_ANT         0x01

#define FILTER_RX	0
#define FILTER_TX	1


class gr_hiqsdr_source;
typedef boost::shared_ptr<gr_hiqsdr_source> gr_hiqsdr_source_sptr;

HIQSDR_API gr_hiqsdr_source_sptr gr_make_hiqsdr_source(size_t itemsize, const char *host, 
		unsigned short port, unsigned short c_port, unsigned short rx_fir_port, unsigned short tx_fir_port,
		int payload_size=1472,
		bool eof=true, bool wait=true, int rxfreq=7000000, int txfreq=7000000, int rate=48000, 
		bool ant=0, int presel=0, int att=0, int txLevel=255, bool ptt=0, int txRate=48000, int clockCorr=0,
		std::vector<gr_complex> rxFirTaps=std::vector<gr_complex>() , std::vector<gr_complex> txFirTaps=std::vector<gr_complex>() );


class HIQSDR_API gr_hiqsdr_source : public gr_sync_block {
	friend gr_hiqsdr_source_sptr gr_make_hiqsdr_source(size_t itemsize,
			const char *host, 
			unsigned short port,
			unsigned short c_port,
			unsigned short rx_fir_port,
			unsigned short tx_fir_port,
			int payload_size,
			bool eof, bool wait,
			int rxfreq, int txfreq, int rate, 
			bool ant, int presel, int att, 
			int txLevel, bool ptt, int txRate, int clockCorr,
			std::vector<gr_complex> rxFirTaps, std::vector<gr_complex> txFirTaps);
	private:
	size_t	d_itemsize;
	int           d_payload_size;  // maximum transmission unit (packet length)
	bool          d_eof;           // zero-length packet is EOF
	int           d_socket;        // handle to socket
	gruel::mutex  d_mutex;           // protects d_socket and d_connected
	unsigned char *d_temp_buff;    // hold buffer between  between call
	int			byteMode;
	unsigned char seqNr;
	gr_complex outBuf[8192];
	int outBufPos;

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
	protected:
	gr_hiqsdr_source(size_t itemsize, const char *host, 
			unsigned short port, unsigned short c_port, unsigned short rx_fir_port,
			unsigned short tx_fir_port,
			int payload_size, bool eof, bool wait, int rxfreq, int txfreq, int rate,
			bool ant, int presel, int att, int txLevel, bool ptt, int txRate, int clockCorr,
			std::vector<gr_complex> rxFirTaps, std::vector<gr_complex> txFirTaps);
	public:
	~gr_hiqsdr_source();

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

	int work(int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items);
};


#endif
