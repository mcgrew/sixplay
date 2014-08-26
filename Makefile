# You know, there are pre-compile DEBs of this...

CXX ?= g++
CXXFLAGS ?= -O2 -Wall
LDFLAGS += -Wl,-Bsymbolic-functions
CC := gcc

all: sixplay_bins

bluez_cflags := $(shell pkg-config --cflags bluez)
bluez_libs := $(shell pkg-config --libs bluez)

targets := sixplay-bin sixplay-sixaxis sixplay-remote sixplay-raw sixplay-3in1 sixpair
sixplay_bins: $(targets)

sixplay-bin: bluetooth.o shared.o textfile.o
sixplay-bin: CFLAGS += $(bluez_cflags)
sixplay-bin: LDLIBS += $(bluez_libs)

sixplay-sixaxis: sixaxis.o shared.o uinput.o textfile.o
sixplay-sixaxis: LDLIBS += -lpthread -lrt

sixplay-remote: remote.o shared.o uinput.o textfile.o
sixplay-remote: LDLIBS += -lrt

sixplay-raw: sixaxis.o shared.o uinput.o textfile.o

sixplay-3in1: sixaxis.o shared.o uinput.o textfile.o

sixpair: LDLIBS += -lusb

clean:
	$(RM) *~ $(targets) *.o

install: $(targets)
	install -d $(DESTDIR)/etc/default/
	install -d $(DESTDIR)/etc/init.d/
	install -d $(DESTDIR)/etc/logrotate.d/
	install -d $(DESTDIR)/usr/bin/
	install -d $(DESTDIR)/usr/sbin/
	install -d $(DESTDIR)/var/lib/sixplay/
	install -d $(DESTDIR)/var/lib/sixplay/profiles/
	install -m 644 sixplay.default $(DESTDIR)/etc/default/sixplay
	install -m 755 sixplay.init $(DESTDIR)/etc/init.d/sixplay
	install -m 644 sixplay.log $(DESTDIR)/etc/logrotate.d/sixplay
	install -m 755 sixplay $(DESTDIR)/usr/bin/
	install -m 755 sixplay-bin $(DESTDIR)/usr/sbin/
	install -m 755 sixplay-sixaxis $(DESTDIR)/usr/sbin/
	install -m 755 sixplay-remote $(DESTDIR)/usr/sbin/
	install -m 755 sixplay-3in1 $(DESTDIR)/usr/sbin/
	install -m 755 sixplay-raw $(DESTDIR)/usr/sbin/
	install -m 755 sixplay-dbus-blocker $(DESTDIR)/usr/sbin/
	install -m 755 sixpair $(DESTDIR)/usr/sbin/
	@chmod 777 -R $(DESTDIR)/var/lib/sixplay/
	@echo "Installation is Complete!"

uninstall:
	$(RM) $(DESTDIR)/etc/default/sixplay
	$(RM) $(DESTDIR)/etc/init.d/sixplay
	$(RM) $(DESTDIR)/etc/logrotate.d/sixplay
	$(RM) $(DESTDIR)/usr/bin/sixplay
	$(RM) $(DESTDIR)/usr/sbin/sixplay-bin
	$(RM) $(DESTDIR)/usr/sbin/sixplay-sixaxis
	$(RM) $(DESTDIR)/usr/sbin/sixplay-remote
	$(RM) $(DESTDIR)/usr/sbin/sixplay-raw
	$(RM) $(DESTDIR)/usr/sbin/sixplay-dbus-blocker
	$(RM) $(DESTDIR)/usr/sbin/sixpair
	$(RM) -r $(DESTDIR)/var/lib/sixplay/
