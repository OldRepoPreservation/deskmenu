CFLAGS = @CFLAGS@ @X_CFLAGS@ -DRCFILE=\".deskmenurc\" $(DEBUG) `gtk-config --cflags`
LIBS = @LIBS@ @X_LIBS@ `gtk-config --libs`
bin_PROGRAMS=deskmenu

deskmenu_SOURCES=deskmenu.c deskmenu.h popup.c windows.c workspaces.c keyboard.c \
		 config.h xmalloc.c system.h aclocal.m4 getopt.c getopt1.c getopt.h
deskmenu_LDADD=-lX11
