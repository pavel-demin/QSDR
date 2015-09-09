#!/bin/sh
cp qsdr /usr/local/bin/qsdr
cp qsdr.png /usr/share/icons/qsdr.png
cp qsdr.png /usr/share/pixmaps/qsdr.png
cp qsdr.desktop /usr/share/applications/qsdr.desktop

if which update-icon-caches >/dev/null 2>&1 ; then
		update-icon-caches /usr/share/icons/qsdr.png
fi
ldconfig

