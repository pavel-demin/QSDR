#!/bin/sh
GNURADIO=/tmp/test 
export PATH=$PATH:$GNURADIO/bin
export LD_LIBRARY_PATH=$GNURADIO/:$LD_LIBRARY_PATH:
$GNURADIO/qsdr.bin $*


