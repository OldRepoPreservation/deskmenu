/* 
 * deskmenu - popup.c
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

int confirm = False;
/* display the menu centered or under the mouse */
int mode = 0;

void
run_program (GtkWidget * widget, char *cmd)
{
  pid_t pid;

#ifdef DEBUG
  printf ("run_program\n");
#endif

  pid = fork ();

  switch (pid)
    {
    case 0:
      execlp ("/bin/sh", "sh", "-c", cmd, NULL);
      fprintf (stderr, "%s: Failed to run command %s\n", program_name, cmd);
      exit (1);
      break;
    case -1:
      fprintf (stderr, "%s: Fork failed.\n", program_name);
      break;
    }
}

void
ok_click (GtkWidget * w)
{
  confirm = True;
  gtk_main_quit ();
}

void
quit (GtkWidget * widget)
{
  GtkWidget *dialog, *label, *ok, *cancel;

#ifdef DEBUG
  printf ("quit\n");
#endif

  dialog = gtk_dialog_new ();
  label = gtk_label_new ("Do you really want to quit?");
  ok = gtk_button_new_with_label ("Ok");
  cancel = gtk_button_new_with_label ("Cancel");

  gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
		      GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
  gtk_signal_connect (GTK_OBJECT (ok), "clicked", GTK_SIGNAL_FUNC (ok_click),
		      NULL);
  gtk_signal_connect_object (GTK_OBJECT (cancel), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     (gpointer) dialog);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label, TRUE, TRUE,
		      0);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area), ok, TRUE,
		      TRUE, 0);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area), cancel,
		      TRUE, TRUE, 0);

  gtk_container_border_width (GTK_CONTAINER (dialog), 5);
  gtk_window_set_title (GTK_WINDOW (dialog), "Confirm");
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_policy (GTK_WINDOW (dialog), FALSE, FALSE, TRUE);

  gtk_widget_show_all (dialog);
  gtk_main ();
  gtk_widget_destroy (dialog);

  if (confirm)
    exit (0);
}

GtkWidget *
add_menu_item (GtkMenu * menu, char *label, GtkSignalFunc fn, gpointer data)
{
  GtkWidget *menu_item;

#ifdef DEBUG
  printf ("add_menu_item\n");
#endif

  if (!label)
    menu_item = gtk_menu_item_new ();
  else
    menu_item = gtk_menu_item_new_with_label (label);
  if (fn)
    gtk_signal_connect (GTK_OBJECT (menu_item), "activate", fn, data);
  gtk_widget_show (menu_item);
  gtk_menu_append (menu, menu_item);
  return menu_item;
}


void
build_submenu (FILE * rc, GtkWidget * menu)
{
  char buf[1024], *lvalue, *rvalue;

  while (fgets (buf, sizeof buf, rc))
    {
      char *comment;
      /* to make sure comments are ignored */
      comment = strchr (buf, '#');
      if (comment)
	{
	  comment[0] = '\n';
	  comment[1] = '\0';
	}

      lvalue = strtok (buf, "=");
      if (lvalue)
	{
	  if (!strcmp (lvalue, "menuitem"))
	    {
	      lvalue = strtok (NULL, ":");
	      if (lvalue)
		{
		  rvalue = strtok (NULL, "\n");
		  if (rvalue)
		    {
		      add_menu_item (GTK_MENU (menu), lvalue, run_program,
				     strdup (rvalue));
		    }
		}
	    }
	  if (!strcmp (lvalue, "include"))
	    {
	      rvalue = strtok (NULL, "\n");
	      if (rvalue)
		{
		  FILE *irc = fopen (rvalue, "r");
		  if (irc)
		    {
		      build_submenu (irc, menu);
		      fclose (irc);
		    }
		}
	    }
	  if (!strcmp (lvalue, "windowlist"))
	    {
	      rvalue = strtok (NULL, "\n");
	      if (rvalue)
		{
		  add_windows_menu (GTK_MENU (menu), rvalue);
		}
	    }
	  if (!strcmp (lvalue, "workspaces"))
	    {
	      rvalue = strtok (NULL, "\n");
	      if (rvalue)
		{
		  add_workspace_menu (GTK_MENU (menu), rvalue);
		}
	    }
	  if (!strcmp (lvalue, "divider"))
	    {
	      add_menu_item (GTK_MENU (menu), NULL, NULL, NULL);
	    }
	  if (!strcmp (lvalue, "submenu"))
	    {
	      rvalue = strtok (NULL, "\n");
	      if (rvalue)
		{
		  GtkWidget *menu_item;
		  GtkWidget *submenu = gtk_menu_new ();
		  build_submenu (rc, submenu);
		  menu_item = add_menu_item (GTK_MENU (menu),
					     rvalue, NULL, NULL);
		  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item),
					     submenu);
		}
	    }
	  if (!strcmp (lvalue, "endmenu"))
	    {
	      return;
	    }
	}
    }
}


GtkWidget *
build_menu (char *file)
{
  GtkWidget *menu = 0;
  FILE *rc;

#ifdef DEBUG
  printf ("add_programs_menu\n");
#endif

  if ((rc = fopen (file, "r")))
    {
      menu = gtk_menu_new ();
      build_submenu (rc, menu);
      fclose (rc);
    }

  if (quit_menu)
    {
      add_menu_item (GTK_MENU (menu), NULL, NULL, NULL);
      add_menu_item (GTK_MENU (menu), "Quit", GTK_SIGNAL_FUNC (quit), NULL);
    }

  gtk_signal_connect (GTK_OBJECT (menu), "deactivate",
		      GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

  return menu;
}

void
menu_pos (GtkMenu * m, int *x, int *y, XEvent * ev)
{
  int w, h, dw, dh;

  w = GTK_WIDGET (m)->requisition.width;
  h = GTK_WIDGET (m)->requisition.height;
  gdk_window_get_size (GDK_ROOT_PARENT (), &dw, &dh);

  if (mode == MODE_MOUSE)
    {
      *x = ev->xbutton.x;
      *y = ev->xbutton.y;
    }
  else
    {
      *x = dw / 2;
      *y = dh / 2;
    }

  if (*x > dw - w)
    *x = dw - w;
  if (*y > dh - h)
    *y = dh - h;
}

void
popup_menu (XEvent * ev)
{
  GtkWidget *root_menu;

  root_menu = build_menu (rcfile);
  gtk_menu_popup (GTK_MENU (root_menu), NULL, NULL,
		  (GtkMenuPositionFunc) menu_pos, ev, ev->xbutton.button,
		  ev->xbutton.time);
  gtk_main ();
  gtk_widget_destroy (root_menu);
}
