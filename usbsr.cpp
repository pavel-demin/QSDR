#include "QDebug"
#include <usb.h>
#include "usbsr.h"

#define USBDEV_SHARED_VENDOR    0x16C0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   0x05DC  /* Obdev's free shared PID */
#define VENDOR_NAME		"www.obdev.at"
#define PRODUCT_NAME		"DG8SAQ-I2C"
#define USB_MAX_RETRIES		1


double multiplier = 4;
int i2cAddress = SI570_I2C_ADDR;
double fXtall = 114.224772;
double startupFreq = SI570_DEFAULT_STARTUP_FREQ;
int verbose = 0;
char * usbSerialID = NULL;

UsbSR::UsbSR () {
    handle = NULL;
    ready = false;
}

UsbSR::~UsbSR() {
    if (ready)
	usb_close(handle);
}

int UsbSR::open() {
    usb_init();
    char attempt=0, error=0;
    do {
	attempt++;
	error = usbOpenDevice(&handle, 
		USBDEV_SHARED_VENDOR, (const char*)VENDOR_NAME, USBDEV_SHARED_PRODUCT,  (const char*)PRODUCT_NAME, usbSerialID);
	if(error != 0){
	    qDebug() << "Could not open USB device ";
	    //sleep(2*attempt);
	    sleep(1);
	}
    } while (error && attempt < USB_MAX_RETRIES);
    if (error) {
	qDebug() << "Permanent problem opening usb device. Giving up.";
	return -1;
    }
    ready = true;
    return 0;
}

void UsbSR::setFreq(int f) {
    if (ready)
	setFrequency(handle, (float(f)/1e6));
}

void UsbSR::setPtt(bool on) {
    if (ready)
	setPTT(handle, on);
}


