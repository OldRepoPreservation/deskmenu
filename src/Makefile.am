CFLAGS = -O2 -g @CFLAGS@ @X_CFLAGS@ `pkg-config gtk+-2.0 --cflags`
LIBS = @LIBS@ @X_LIBS@ `pkg-config gtk+-2.0 --libs`
bin_PROGRAMS=deskmenu

deskmenu_SOURCES=deskmenu.c deskmenu.h popup.c windows.c workspaces.c keyboard.c \
		 config.h xmalloc.c system.h getopt.c getopt1.c getopt.h
deskmenu_LDADD=-lX11
