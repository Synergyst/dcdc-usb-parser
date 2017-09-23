PACKAGES=libusb
LIBS=`pkg-config --libs ${PACKAGES}` -lm
INCS=`pkg-config --cflags ${PACKAGES}`
CFLAGS=-Wall

MAINSRC=main.c
DCDCUSBSRC=dcdc-usb-portable.c

all: main dcdc-usb-portable
install: all
        install -v -m755 dcdc-usb-parser /usr/local/bin/
        install -v -m755 dcdc-usb-portable /usr/local/bin/

main: $(MAINSRC)
        $(CC) $(CFLAGS) $(MAINSRC) -o dcdc-usb-parser

dcdc-usb-portable: $(DCDCUSBSRC)
        $(CC) $(CFLAGS) $(DCDCUSBSRC) -o $@ $(LIBS)

clean:
        rm -rf dcdc-usb-parser
        rm -rf dcdc-usb-portable

uninstall:
        rm /usr/local/bin/dcdc-usb-parser
        rm /usr/local/bin/dcdc-usb-portable
