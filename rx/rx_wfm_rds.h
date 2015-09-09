#ifndef RX_WFM_RDS_H
#define RX_WFM_RDS_H

#include <gnuradio/hier_block2.h>
#include "rx.h"
#include "gnuradio.h"
#include <gnuradio/digital/diff_decoder_bb.h>
#include <gnuradio/analog/pll_freqdet_cf.h>
#include "gr_rds_bpsk_demod.h"  
#include "gr_rds_data_decoder.h"  
#include "gr_rds_freq_divider.h"

class Rx_wfmrds;

typedef boost::shared_ptr<Rx_wfmrds> Rx_wfmrds_sptr;
Rx_wfmrds_sptr make_rx_wfmrds(int sampleRate, gr::msg_queue::sptr msgq);

class Rx_wfmrds : public Rx 
{

public:
    Rx_wfmrds(int, gr::msg_queue::sptr);
    ~Rx_wfmrds();

	void setAGC(bool on, double att, double dec, double ref);
	void setFreq(int f);
	void setFilter(int, int, int);
	void setFilterNotch(int, int);
	void setNotch(bool);
	void setSampleRate(int f);
	void setSquelch(int n);
	float getSignal();

private:

	gr::filter::freq_xlating_fir_filter_ccf::sptr filter_wfm;
	gr::filter::firdes *firdes;
	gr::digital::diff_decoder_bb::sptr gr_diff_decoder_bb_0;
	gr::blocks::multiply_ff::sptr gr_multiply_xx_0;
	gr::blocks::multiply_ff::sptr gr_multiply_xx_1;
	gr::blocks::multiply_ff::sptr gr_multiply_xx_2;
	gr::analog::pll_freqdet_cf::sptr gr_pll_freqdet_cf_0;
	gr_rds_bpsk_demod_sptr gr_rds_bpsk_demod_0;
	gr_rds_data_decoder_sptr gr_rds_data_decoder_0;
	gr_rds_freq_divider_sptr gr_rds_freq_divider_0;
	gr::blocks::add_ff::sptr left;
	gr::filter::fir_filter_fff::sptr gr_rds_clock_filter;
	gr::filter::fir_filter_fff::sptr lmr_bb_filter;
	gr::filter::fir_filter_fff::sptr lmr_filter;
	gr::filter::fir_filter_fff::sptr lpr_filter;
	gr::filter::fir_filter_fff::sptr rds_bb_filter;
	gr::filter::fir_filter_fff::sptr rds_clk_filter;
	gr::filter::fir_filter_fff::sptr rds_filter;
	gr::blocks::sub_ff::sptr right;
	gr::analog::probe_avg_mag_sqrd_c::sptr smeter;
	
	gr::filter::fir_filter_fcc::sptr filterPilotTone; 
	gr::analog::pll_refout_cc::sptr pllPilotTone;
	gr::blocks::complex_to_imag::sptr c2imag;
	gr::blocks::complex_to_imag::sptr c2imag1;
	gr::blocks::multiply_cc::sptr multic; 

	int sampleRate;
};

#endif // RX_WFM_RDS_H
