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

void raise_window(GtkWidget *widget, Window win)
{
#ifdef DEBUG
  printf("raise_window\n");
#endif

  XMapRaised(dpy, win);
  XSetInputFocus(dpy, win, RevertToPointerRoot, CurrentTime);
}

void add_windows_menu(GtkMenu *menu, char *title)
{
  GtkWidget *menu_item, *win_menu;
  Atom real_type;
  int real_format;
  unsigned long items_read, items_left, item=0;
  long *win=NULL;
  char *name=NULL, label[27];

#ifdef DEBUG
  printf("add_windows_menu\n");
#endif

  win_menu=gtk_menu_new();
  menu_item=add_menu_item(GTK_MENU(menu), title, NULL, NULL);

  while(XGetWindowProperty(dpy, root, gnome[WIN_CLIENT_LIST], item++, 1L, False, XA_CARDINAL, &real_type, &real_format, &items_read, &items_left, (unsigned char **)&win)==Success && items_read)
  {
    if(!(get_gnome_hint(*win, WIN_HINTS)&WIN_HINTS_SKIP_WINLIST) && (get_gnome_hint(*win, WIN_WORKSPACE)==get_gnome_hint(root, WIN_WORKSPACE)))
    {
      XFetchName(dpy, *win, &name);
      if(strlen(name)>=25)
        strcpy(name+21, "...");
      strcpy(label, name);
      if(get_wm_state(*win)==IconicState)
        sprintf(label, "[%s]", name);
       
      add_menu_item(GTK_MENU(win_menu), label, raise_window, (gpointer) *win);
      if(name) XFree(name);
    }
    if(win) XFree(win);
  }
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), win_menu);
}
