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
change_workspace (GtkWidget * widget, long ws)
{
  XEvent ev;

#ifdef DEBUG
  printf ("change_workspace\n");
#endif

  ev.type = ClientMessage;
  ev.xclient.window = root;
  ev.xclient.message_type = gnome[WIN_WORKSPACE];
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = ws;
  XSendEvent (dpy, root, False,
	      SubstructureNotifyMask | SubstructureRedirectMask, &ev);
}

void
add_workspace (GtkWidget * widget)
{
#ifdef DEBUG
  printf ("add_workspace\n");
#endif

  set_gnome_hint (root, WIN_WORKSPACE_COUNT,
		  get_gnome_hint (root, WIN_WORKSPACE_COUNT) + 1);
}

void
del_workspace (GtkWidget * widget)
{
#ifdef DEBUG
  printf ("del_workspace\n");
#endif

  set_gnome_hint (root, WIN_WORKSPACE_COUNT,
		  get_gnome_hint (root, WIN_WORKSPACE_COUNT) - 1);
}

void
add_workspace_menu (GtkMenu * menu, char *title)
{
  GtkWidget *menu_item, *ws_menu;
  int ws, ws_count;
  char caption[255];

#ifdef DEBUG
  printf ("add_workspace_menu\n");
#endif

  ws_menu = gtk_menu_new ();
  ws_count = get_gnome_hint (root, WIN_WORKSPACE_COUNT);
  for (ws = 0; ws < ws_count; ws++)
    {
      sprintf (caption, "Workspace %d %s", ws + 1,
	       ws == get_gnome_hint (root, WIN_WORKSPACE) ? "*" : " ");
      add_menu_item (GTK_MENU (ws_menu), caption, change_workspace,
		     (gpointer) ws);
    }
  add_menu_item (GTK_MENU (ws_menu), NULL, NULL, NULL);
  add_menu_item (GTK_MENU (ws_menu), "Add Workspace", add_workspace, NULL);
  add_menu_item (GTK_MENU (ws_menu), "Delete Workspace", del_workspace, NULL);

  menu_item = add_menu_item (GTK_MENU (menu), title, NULL, NULL);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), ws_menu);
}
