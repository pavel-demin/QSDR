#ifndef __CONFIG_H__
#define __CONFIG_H__

#define VERSION        		"1.1.0"

#define MAX_FFT 			(1024*8)	
#define MAX_MEM 			100
#define AUDIO_RATE			48000
#define REMOTE_AUDIO_RATE	8000
#define IF_RATE				(AUDIO_RATE*1)
#define IF_STEREO_RATE		(AUDIO_RATE*5)
#define DEFAULT_SAMPLE_RATE	(48000)
#define RX_CLOCK			122880000LL

#define HIQSDR_RX_PORT		0xbc77 
#define HIQSDR_CTL_PORT		0xbc78 
#define HIQSDR_TX_PORT		0xbc79 
#define HIQSDR_RAW_PORT		0xbc7a 
#define HIQSDR_RX_FIR_PORT	0xbc7b 
#define HIQSDR_TX_FIR_PORT	0xbc7c 

#define HIQSCOPE_RAW_PORT	22222

#define FFT_OFFSET			(256+60)

#define TRX_PORT			51400

#define HIQ_PREAMP			0x10
#define HIQ_ANT				0x01

#define TIMEZONE_UTC		0
#define TIMEZONE_LOCAL		1

#define MAX_VFO 			4
#define MAXBUTTON			20

#define NBUTTON			(sdrSettings->getIntVal("Band,BandButtons"))	
#define CW_TONE			(sdrSettings->getIntVal("Config,CWTone"))	

#define RAW_SRC_FULL	0
#define RAW_SRC_CIC_A	1
#define RAW_SRC_CIC_B	2

typedef enum {
    MODE_LSB,
    MODE_USB,
    MODE_CW,
    MODE_AM,
    MODE_NFM,
    MODE_WFM_RDS,
    MODE_RAW,
    MODE_DAB,
    MODE_TEST,
    MODE_LAST
} Mode;

typedef enum {
    SRC_HIQSDR,
	SRC_USRP,
    SRC_OSMOSDR,
    SRC_NULL,
    SRC_TESTSIGNAL,
    SRC_FILE,
    SRC_END
} RxSrc;

typedef enum {
	RX_SINK_LOCALAUDIO,
	RX_SINK_IP
} ERxSink;

typedef enum {
	TX_SRC_LOCALAUDIO,
	TX_SRC_IP,
	TX_SRC_TWOTONE
} ETxSource;


#endif
