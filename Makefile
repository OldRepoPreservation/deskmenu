CC        = gcc
CFLAGS    = -g -O2 -Wall `gtk-config --cflags`
XROOT     = /usr/X11
INCLUDES  = -I$(XROOT)/include
LIBS      = -lX11 `gtk-config --libs`
LDPATH    = -L$(XROOT)/lib

PROG      = deskmenu
HEADERS   = $(PROG).h
RCFILE    = -DRCFILE=\".deskmenurc\"
DEBUG     = #-DDEBUG
DEFINES   = $(RCFILE) $(DEBUG)
# Next two lines were edited/added for Debian packaging
DESTDIR =
PREFIX    = $(DESTDIR)/usr

OBJS      = $(PROG).o popup.o workspaces.o windows.o

all: $(PROG) 

$(PROG): $(OBJS)
	$(CC) $(OBJS) $(LDPATH) $(LIBS) -o $@

$(OBJS): %.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

install: all
	install $(PROG) $(PREFIX)/bin

#install-menu: deskmenu.menu-method
#	install -m 0755 deskmenu.menu-method /etc/menu-methods/deskmenu
#	echo "updating menus..."
#	update-menus

clean:
	rm -f $(PROG) $(OBJS)

uninstall:
	rm -f $(PREFIX)/bin/$(PROG)
