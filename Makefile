PREFIX = /usr/local
bindir = $(PREFIX)/bin
datadir = $(PREFIX)/share
pkgdatadir = $(datadir)/gamine
docdir = $(datadir)/doc/gamine
sysconfdir = $(PREFIX)/etc
desktopdir = $(datadir)/applications
icondir = $(datadir)/icons/hicolor/scalable/apps
localedir = $(datadir)/locale

GCONFTOOL = /usr/bin/gconftool-2
GCONF_SCHEMA_CONFIG_SOURCE = xml:merged:/etc/gconf/gconf.xml.defaults
GCONF_SCHEMA_FILE_DIR = $(sysconfdir)/gconf/schemas
GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL = 0
CFLAGS = -Wall
#CFLAGS = -Wall -g 
CPPFLAGS = $(shell pkg-config --cflags gtk+-2.0 cairo glib-2.0 gstreamer-0.10 gconf-2.0)  -DDATADIR=\""$(pkgdatadir)"\"  -DLOCALDIR=\""$(localedir)"\"
LDLIBS = $(shell pkg-config --libs gtk+-2.0 cairo glib-2.0 gstreamer-0.10 gconf-2.0)  -DDATADIR=\""$(pkgdatadir)"\"  -DLOCALDIR=\""$(localedir)"\"
LDFLAGS = -g 
CC = gcc
target = gamine
objs = gamine.o

$(target): $(objs)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	sed 's@BINDIR@$(bindir)@' gamine.desktop.in > gamine.desktop
	msgfmt -c -o locale/fr.mo locale/fr.po

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(CPPFLAGS)
clean:
	rm -f $(target) $(objs) gamine.desktop *~ locale/*.mo

install:
	mkdir -p $(bindir)
	mkdir -p $(pkgdatadir)/sounds
	mkdir -p $(docdir)
	mkdir -p $(icondir)
	mkdir -p $(desktopdir)
	mkdir -p $(GCONF_SCHEMA_FILE_DIR)
	mkdir -p $(localedir)/fr/LC_MESSAGES
	install -m 755 gamine $(bindir)/
	install -m 644 pencil.png $(pkgdatadir)/
	install -m 644 gamine.png $(pkgdatadir)/
	install -m 644 sounds/* $(pkgdatadir)/sounds/
	install -m 644 README.pencil README.sounds README ChangeLog COPYING $(docdir)/
	install -m 644 gamine.schemas $(GCONF_SCHEMA_FILE_DIR)/
	install -m 644 locale/fr.mo $(localedir)/fr/LC_MESSAGES/gamine.mo
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(GCONF_SCHEMA_FILE_DIR)/gamine.schemas
	install -m 644 gamine.desktop $(desktopdir)/
	install -m 644 gamine.svg $(icondir)/

uninstall:
	rm -rf $(bindir)/gamine
	rm -rf $(pkgdatadir)
	rm -rf $(docdir)
	rm -rf $(GCONF_SCHEMA_FILE_DIR)/gamine.schemas
	rm -rf $(desktopdir)/gamine.desktop
	rm -rf $(icondir)/gamine.svg
