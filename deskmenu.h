#ifndef DESKMENU_H
#define DESKMENU_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <gtk/gtkinvisible.h>
#include <gdk/gdkx.h>

#define PROGNAME	"DESKMENU"

#define WIN_HINTS				0
#define WIN_WORKSPACE				1
#define WIN_WORKSPACE_COUNT			2
#define WIN_DESKTOP_BUTTON_PROXY		3
#define WIN_CLIENT_LIST				4
#define GNOME_HINT_COUNT			5

#define WIN_HINTS_SKIP_WINLIST		(1L<<1)

/* deskmenu.c */
extern Display *dpy;
extern int scr, quit_menu;
extern Window root, proxy_win;
extern Atom wm_state, gnome[GNOME_HINT_COUNT];
extern long get_wm_state(Window);
extern void set_gnome_hint(Window, int, long);
extern long get_gnome_hint(Window, int);

/* popup.c */
extern void popup_menu(XEvent *);
extern GtkWidget *add_menu_item(GtkMenu *, char *, GtkSignalFunc, gpointer);

/* window.c */
extern void add_windows_menu(GtkMenu *, char *);

/* workspace.c */
extern void add_workspace_menu(GtkMenu *, char *);

#endif /* DESKMENU_H */
