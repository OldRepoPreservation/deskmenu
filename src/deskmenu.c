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

/* some includes {{{1 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <X11/keysym.h>
#include "deskmenu.h"
#include "system.h"
/* }}}1 */

/* getopt_long return codes {{{1 */
enum
{ DUMMY_CODE = 129
};
/* }}}1 */

/* The name the program was run with, stripped of any leading path. */
char *program_name;

struct stat stat_buf;

/* Option flags and variables {{{1 */
int quit_menu;			/* --quit-menu */
int button;			/* --button */
int want_verbose;		/* --verbose */
char *rcfile;			/* --file */
Display *dpy;			/* the pointer to the display */
int scr;			/* the number of the used screen */
Atom wm_state;
Atom gnome[GNOME_HINT_COUNT];
Window root;			/* The root window */
Window proxy_win;		/* a proxy window */
char *locale;			/* the used locale */
KeyCode keycode;
int modifier;

/* }}}1 */

/* the structure of all long options for getopt {{{1 */
static struct option const long_options[] = {
  {"quit-menu", no_argument, 0, 'q'},
  {"verbose", no_argument, 0, 'v'},
  {"help", no_argument, 0, 'h'},
  {"version", no_argument, 0, 'V'},
  {"file", required_argument, 0, 'f'},
  {"button", required_argument, 0, 'b'},
  {NULL, 0, NULL, 0}
};
/* }}}1 */

/* a signal handling function,
 * SIGCHLD is currently totally ignored */
void
signal_handler (int signal)
{
#ifdef DEBUG
  printf ("signal_handler\n");
#endif

  if (signal == SIGCHLD)
    wait (NULL);
}

int
handle_xerror (Display * dpy, XErrorEvent * err)
{
  return 0;
}

long
get_wm_state (Window w)
{
  Atom real_type;
  int real_format;
  unsigned long items_read, items_left;
  long *data, state = WithdrawnState;

#ifdef DEBUG
  printf ("get_wm_state\n");
#endif

  if (XGetWindowProperty
      (dpy, w, wm_state, 0L, 2L, False, wm_state, &real_type, &real_format,
       &items_read, &items_left, (unsigned char **) &data) == Success
      && items_read)
    {
      state = *data;
      XFree (data);
    }
  return state;
}

void
set_gnome_hint (Window w, int a, long value)
{
#ifdef DEBUG
  printf ("set_gnome_hint\n");
#endif

  if (a == WIN_WORKSPACE_COUNT && value <= 0)
    return;

  XChangeProperty (dpy, w, gnome[a], XA_CARDINAL, 32, PropModeReplace,
		   (unsigned char *) &value, 1);
}

long
get_gnome_hint (Window w, int a)
{
  Atom real_type;
  int real_format;
  unsigned long items_read, items_left;
  long *data, value = 0;

#ifdef DEBUG
  printf ("get_gnome_hint\n");
#endif

  if (XGetWindowProperty
      (dpy, w, gnome[a], 0L, 1L, False, XA_CARDINAL, &real_type, &real_format,
       &items_read, &items_left, (unsigned char **) &data) == Success
      && items_read)
    {
      value = *data;
      XFree (data);
    }
  return value;
}

void
initialize (int argc, char *argv[])
{
  int i;
  struct sigaction act;
  FILE *rc;

#ifdef DEBUG
  printf ("initialize\n");
#endif

  program_name = argv[0];

  locale = gtk_set_locale ();
  gtk_init (&argc, &argv);

  i = decode_switches (argc, argv);

  if (!rcfile)
    {
      size_t size =
	sizeof (char) * (strlen (getenv ("HOME")) + 1) +
	sizeof (char) * (strlen (".deskmenurc") + 1);
      rcfile = (char *) xmalloc (size);
      snprintf (rcfile, size, "%s/%s", getenv ("HOME"), ".deskmenurc");
    }

  /* check if the given file is correct */
  if (stat (rcfile, &stat_buf))
    {
      /* an error while stating occured */
      fprintf (stderr, "%s: ", PACKAGE);
      perror (rcfile);
      free(rcfile);
      /* check if /etc/deskmenurc is existant */
      rcfile = "/etc/deskmenurc";
      if (stat (rcfile, &stat_buf))
	{
	  /* an error while stating occured */
	  fprintf (stderr, "%s: ", PACKAGE);
	  perror (rcfile);
	  exit (EXIT_FAILURE);
	}
    }

  keycode = 0;
  modifier = 0;

  if (!button)
    button = Button2;

  dpy = GDK_DISPLAY ();
  root = GDK_ROOT_WINDOW ();

  /* now since we have a usable rcfile, open it and read the key which to use
   * for the popup. */
  if ((rc = fopen (rcfile, "r")))
    {
      char buf[1024], *lvalue, *rvalue, *k;
      while (fgets (buf, sizeof buf, rc))
	{
	  char *comment;
	  /* to make sure comments are ignored */
	  comment = strchr(buf, '#');
	  if (comment)
	    {
	      comment[0] = '\n';
	      comment[1] = '\0';
	    }

	  /* remove all spaces and tabs */
	  while (comment = strpbrk(buf, " \t"))
	    strcpy (comment, comment+1);
	  
	  lvalue = strtok (buf, "=");
	  if (lvalue)
	    {
	      if (!strcmp (lvalue, "keycode"))
		{
		  rvalue = strtok(NULL, "\n");
		  if (rvalue)
		    {
		      if (!strcmp (rvalue, "none"))
			{
			  fclose (rc);
			  break;
			}
		      k = strrchr (rvalue, '+');
		      if (k)
			{
			  keycode = XKeysymToKeycode (dpy,
			      XStringToKeysym (k + 1));
			  if (strstr (rvalue, "Shift"))
			    modifier = modifier | ShiftMask;
			  if (strstr (rvalue, "Control"))
			    modifier = modifier | ControlMask;
			  if (strstr (rvalue, "Mod1"))
			    modifier = modifier | Mod1Mask;
			  if (strstr (rvalue, "Mod2"))
			    modifier = modifier | Mod2Mask;
			  if (strstr (rvalue, "Mod3"))
			    modifier = modifier | Mod3Mask;
			  if (strstr (rvalue, "Mod4"))
			    modifier = modifier | Mod4Mask;
			  if (strstr (rvalue, "Mod5"))
			    modifier = modifier | Mod5Mask;
			}
		    }
		}
	    }
	}
      fclose (rc);
    }

  XSetErrorHandler (handle_xerror);

  init_keyboard ();
  grab_key (keycode, modifier, root);

  act.sa_handler = signal_handler;
  act.sa_flags = 0;
  sigaction (SIGCHLD, &act, NULL);
  /* currently do *nothing* when getting a HUP, because reloading of
   * the configfile happens everytime the menu is build */
  sigaction (SIGHUP, &act, NULL);

  /* Wait until a GNOME compliant WM is running */
  while (!XInternAtom (dpy, "_WIN_DESKTOP_BUTTON_PROXY", True));
  sleep (1);

  wm_state = XInternAtom (dpy, "WM_STATE", False);
  gnome[WIN_HINTS] = XInternAtom (dpy, "_WIN_HINTS", False);
  gnome[WIN_WORKSPACE] = XInternAtom (dpy, "_WIN_WORKSPACE", False);
  gnome[WIN_WORKSPACE_COUNT] =
    XInternAtom (dpy, "_WIN_WORKSPACE_COUNT", False);
  gnome[WIN_DESKTOP_BUTTON_PROXY] =
    XInternAtom (dpy, "_WIN_DESKTOP_BUTTON_PROXY", False);
  gnome[WIN_CLIENT_LIST] = XInternAtom (dpy, "_WIN_CLIENT_LIST", False);

  proxy_win = get_gnome_hint (root, WIN_DESKTOP_BUTTON_PROXY);
  XSelectInput (dpy, proxy_win, SubstructureNotifyMask);
  XSelectInput (dpy, root, PropertyChangeMask);
}

int
decode_switches (int argc, char **argv)
{
  int c;

  while ((c = getopt_long (argc, argv, "v"	/* verbose */
			   "h"	/* help */
			   "V"	/* version */
			   "f:"	/* file */
			   "b:"	/* button */
			   "q",	/* quit-menu */
			   long_options, (int *) 0)) != EOF)
    {
      switch (c)
	{
	case 'q':		/* --quit-menu */
	  quit_menu = 1;
	  break;
	case 'v':		/* --verbose */
	  want_verbose = 1;
	  break;
	case 'f':		/* --file */
	  rcfile = optarg;
	  break;
	case 'b':		/* --button */
	  button = atoi (optarg);
	  switch (button)
	    {
	    case 1:
	      button = Button1;
	      break;
	    case 2:
	      button = Button2;
	      break;
	    case 3:
	      button = Button3;
	      break;
	    case 4:
	      button = Button4;
	      break;
	    case 5:
	      button = Button5;
	      break;
	    default:
	      usage (EXIT_FAILURE);
	    }
	  break;
	case 'V':
	  printf ("%s %s\n", PACKAGE, VERSION);
	  exit (0);

	case 'h':
	  usage (0);

	default:
	  usage (EXIT_FAILURE);
	}
    }

  return optind;
}

void
usage (int status)
{
  printf (_("%s - \
A root menu for X.\n"), program_name);
  printf (_("Usage: %s [OPTION]... \n"), program_name);
  printf (_("\
Options:\n\
  -r, --file=FILE            use an alternative rcfile\n\
                             instead of $HOME/.deskmenurc\n\
  -q, --quit-menu            display a quit menu\n\
  --verbose                  print more information\n\
  -h, --help                 display this help and exit\n\
  -V, --version              output version information and exit\n\
  --display                  use an alternative display instead of,
                             $DISPLAY\n\
"));
  exit (status);
}

int
main (int argc, char *argv[])
{
  initialize (argc, argv);
  while (1)
    {
      while (XPending (dpy))
	{
	  XEvent ev;
	  XNextEvent (dpy, &ev);

	  if ((ev.type == ButtonPress && ev.xbutton.button == button))
	    {
	      mode = MODE_MOUSE;
	      popup_menu (&ev);
	    }
	  if (ev.type == KeyPress)
	    {
	      mode = MODE_CENTERED;
	      popup_menu (&ev);
	    }
	  if (ev.type == PropertyNotify
	      && ev.xproperty.atom == gnome[WIN_DESKTOP_BUTTON_PROXY])
	    {
	      proxy_win = get_gnome_hint (root, WIN_DESKTOP_BUTTON_PROXY);
	      XSelectInput (dpy, proxy_win, SubstructureNotifyMask);
	    }
	}
      usleep (50);
    }
  return 0;
}

/* vim600:set foldmarker={{{,}}} foldmethod=marker: */
