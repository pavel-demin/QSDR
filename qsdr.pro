TARGET = qsdr
TEMPLATE = app
QT += uitools gui widgets network multimedia
OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build

FORMS += qsdr.ui play.ui
RESOURCES = qsdr.qrc

SOURCES += main.cpp trx.cpp qsdrimpl.cpp fftGraph.cpp filterGraph.cpp audioGraph.cpp
SOURCES += smeter.cpp settings.cpp delegate.cpp treemodel.cpp treeitem.cpp play.cpp

SOURCES += rx/rx.cpp rx/rx_usb.cpp rx/rx_lsb.cpp rx/rx_cw.cpp rx/rx_am.cpp rx/rx_fm.cpp rx/rx_null.cpp
SOURCES += rx/rx_wfm_rds.cpp rx/rx_raw.cpp
SOURCES += rx/rx_psk31.cpp

SOURCES += tx/tx.cpp tx/tx_usb.cpp tx/tx_lsb.cpp tx/tx_am.cpp tx/tx_fm.cpp tx/tx_null.cpp

SOURCES += trxdev/trxDev.cpp trxdev/trxDev_file.cpp
SOURCES += trxdev/trxDev_null.cpp trxdev/trxDev_testsignal.cpp

SOURCES += txsrc/txSrc.cpp txsrc/txSrc_mic.cpp txsrc/txSrc_miceq.cpp txsrc/txSrc_tone.cpp
SOURCES += txsrc/txSrc_null.cpp txsrc/txSrc_message.cpp

SOURCES += rxsink/rxSink.cpp rxsink/rxSink_speaker.cpp rxsink/rxSink_message.cpp

SOURCES += gr/gr_sdr_sink.cc gr/gr_psk31_decoder.cc
SOURCES += gr/gr_reader_i.cc gr/gr_limit_ff.cc
SOURCES += gr_rds/gr_rds_bpsk_demod.cc gr_rds/gr_rds_data_decoder.cc gr_rds/gr_rds_data_encoder.cc gr_rds/gr_rds_freq_divider.cc gr_rds/gr_rds_rate_enforcer.cc
SOURCES += gr/gr_sdrfile_source.cc gr/gr_sdrfile_sink.cc

SOURCES += lib.cpp trx.pb.cc
SOURCES += g7xx/g711.c g7xx/g721.c g7xx/g723_24.c g7xx/g723_40.c g7xx/g72x.c

SOURCES += crypt/sha256.c crypt/crypt.cpp

HEADERS += qsdrimpl.h fftGraph.h filterGraph.h audioGraph.h smeter.h settings.h delegate.h treemodel.h treeitem.h play.h trx.h

SOURCES += trxdev/trxDev_osmosdr.cpp
LIBS += -lgnuradio-osmosdr
DEFINES += OSMOSDR

SOURCES += trxdev/trxDev_usrp.cpp
LIBS += -lgnuradio-uhd -luhd
DEFINES += USRP

SOURCES += trxdev/trxDev_hiqsdr.cpp
SOURCES += hiqsdr/sink_impl.cc hiqsdr/source_impl.cc

SOURCES += trxdev/trxDev_redpitrcv.cpp
SOURCES += redpitaya/rcv_source_impl.cc redpitaya/trx_source_impl.cc redpitaya/trx_sink_impl.cc

INCLUDEPATH += rx rxsink tx trxdev txsrc gr gr_rds g7xx crypt /usr/include/libxml2

LIBS += -lprotobuf

LIBS += -lfftw3
LIBS += -lxml2
LIBS += -lboost_system
LIBS += -lasound

LIBS += -lgnuradio-runtime
LIBS += -lgnuradio-audio
LIBS += -lgnuradio-digital
LIBS += -lgnuradio-vocoder
LIBS += -lgnuradio-pmt
LIBS += -lgnuradio-filter
LIBS += -lgnuradio-blocks
LIBS += -lgnuradio-analog
LIBS += -lgnuradio-fft
