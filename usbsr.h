#ifndef USBSR_H
#define USBSR_H

#include <usb.h>
extern "C" {
#include "operations.h"
}

class UsbSR {
    public:
	UsbSR (); 
	~UsbSR();
	int open();
	void setFreq(int f);
	void setPtt(bool on);
    private:
	usb_dev_handle *handle;
	int ready;
};

#endif
