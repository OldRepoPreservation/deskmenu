/* 
 * deskmenu
 *
 * Copyright (C) 2001 Ken Lynch
 * Copyright (C) 2002 Stefan Pfetzing
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
 */

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

#define MODE_MOUSE				0
#define MODE_CENTERED				1

/* deskmenu.c */
extern Display *dpy;
extern int scr, quit_menu;
extern Window root, proxy_win;
extern Atom wm_state, gnome[GNOME_HINT_COUNT];
extern char *locale;
extern long get_wm_state(Window);
extern void set_gnome_hint(Window, int, long);
extern long get_gnome_hint(Window, int);

/* keyboard.c */
extern int NumLockMask, CapsLockMask, ScrollLockMask;
extern void grab_key(int, unsigned int, Window);
extern void init_keyboard(void);

/* popup.c */
extern int mode;
extern void popup_menu(XEvent *);
extern GtkWidget *add_menu_item(GtkMenu *, char *, GtkSignalFunc, gpointer);

/* window.c */
extern char *get_icon_name (Window win);
extern void add_windows_menu(GtkMenu *, char *);

/* workspace.c */
extern void add_workspace_menu(GtkMenu *, char *);

#endif /* DESKMENU_H */
