/* 
 * deskmenu - deskmenu.c
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

#include "deskmenu.h"

void
raise_window (GtkWidget * widget, Window win)
{
#ifdef DEBUG
  printf ("raise_window\n");
#endif

  XMapRaised (dpy, win);
  XSetInputFocus (dpy, win, RevertToPointerRoot, CurrentTime);
}

char *
get_icon_name (Window win)
{
  XTextProperty text_prop;
  int status;
  char *iconname;

  status = XGetWMIconName (dpy, win, &text_prop);
  if (!status || !text_prop.value || !text_prop.nitems)
    return NULL;
  if (text_prop.encoding == XA_STRING)
    {
      if (!text_prop.value)
	return NULL;
      iconname = (char *) strdup (text_prop.value);
      XFree (text_prop.value);
      return iconname;
    }
  else
    {
      char **list;
      int num;
      XmbTextPropertyToTextList (dpy, &text_prop, &list, &num);
      if (!num || !*list)
	return NULL;
      XFree (text_prop.value);
      iconname = (char *) strdup (*list);
      XFreeStringList (list);
      return iconname;
    }
}

void
add_windows_menu (GtkMenu * menu, char *title)
{
  GtkWidget *menu_item, *win_menu;
  Atom real_type;
  int real_format;
  unsigned long items_read, items_left, item = 0;
  long *win = NULL;
  char *name = NULL, label[27];

#ifdef DEBUG
  printf ("add_windows_menu\n");
#endif

  win_menu = gtk_menu_new ();
  menu_item = add_menu_item (GTK_MENU (menu), title, NULL, NULL);

  while (XGetWindowProperty
	 (dpy, root, gnome[WIN_CLIENT_LIST], item++, 1L, False, XA_CARDINAL,
	  &real_type, &real_format, &items_read, &items_left,
	  (unsigned char **) &win) == Success && items_read)
    {
      if (!(get_gnome_hint (*win, WIN_HINTS) & WIN_HINTS_SKIP_WINLIST)
	  && (get_gnome_hint (*win, WIN_WORKSPACE) ==
	      get_gnome_hint (root, WIN_WORKSPACE)))
	{
	  name = get_icon_name (*win);
	  if (name)
	    {
	      if (strlen (name) >= 25)
		strcpy (name + 21, "...");
	      strcpy (label, name);
	      if (get_wm_state (*win) == IconicState)
		sprintf (label, "[%s]", name);

	      add_menu_item (GTK_MENU (win_menu), label, G_CALLBACK(raise_window),
			     (gpointer) * win);
	      XFree (name);
	    }
	}
      if (win)
	XFree (win);
    }
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), win_menu);
}
