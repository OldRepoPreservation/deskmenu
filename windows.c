/************************************/
/* DeskMenu (c) Ken Lynch, May 2001 */
/* Distributed under the GPL        */
/* See LISENCE for more details     */
/************************************/

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
