CFLAGS=-Wall

MAINSRC=main.c

all: main
install: all
        install -v -m755 dcdc-usb-parser /usr/local/bin/

main: $(MAINSRC)
        $(CC) $(CFLAGS) $(MAINSRC) -o dcdc-usb-parser

clean:
        rm -rf dcdc-usb-parser

uninstall:
        rm /usr/local/bin/dcdc-usb-parser
