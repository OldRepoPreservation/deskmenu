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
#include <X11/keysym.h>

Display *dpy;
int scr, quit_menu=False, button=Button2;
Atom wm_state, gnome[GNOME_HINT_COUNT];
Window root, proxy_win;
int NumLockMask, CapsLockMask, ScrollLockMask;
char *locale;

void signal_handler(int signal)
{
#ifdef DEBUG
  printf("signal_handler\n");
#endif

  if(signal==SIGCHLD) wait(NULL);
}

int handle_xerror(Display *dpy, XErrorEvent *err)
{
  return 0;
}

long get_wm_state(Window w)
{
  Atom real_type;
  int real_format;
  unsigned long items_read, items_left;
  long *data, state = WithdrawnState;

#ifdef DEBUG
  printf("get_wm_state\n");
#endif

  if(XGetWindowProperty(dpy, w, wm_state, 0L, 2L, False, wm_state, &real_type,&real_format, &items_read, &items_left,(unsigned char **)&data)==Success && items_read)
  {
    state=*data;
    XFree(data);
  }
  return state;
}

void set_gnome_hint(Window w, int a, long value)
{
#ifdef DEBUG
  printf("set_gnome_hint\n");
#endif

  if(a==WIN_WORKSPACE_COUNT && value<=0) return;

  XChangeProperty(dpy, w, gnome[a], XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&value, 1);
}

long get_gnome_hint(Window w, int a)
{
  Atom real_type;
  int real_format;
  unsigned long items_read, items_left;
  long *data, value=0;

#ifdef DEBUG
  printf("get_gnome_hint\n");
#endif

  if(XGetWindowProperty(dpy, w, gnome[a], 0L, 1L, False, XA_CARDINAL, &real_type, &real_format, &items_read, &items_left, (unsigned char **)&data)==Success && items_read)
  {
    value=*data;
    XFree(data);
  }
  return value;
}

void init_keyboard()
{
  XModifierKeymap *xmk=NULL;
  KeyCode *map;
  int m, k;

#ifdef DEBUG
  printf("init_keyboard\n");
#endif

  xmk=XGetModifierMapping(dpy);
  if(xmk)
  {
    map=xmk->modifiermap;
    for(m=0;m<8;m++)
      for(k=0;k<xmk->max_keypermod; k++, map++)
      {
        if(*map==XKeysymToKeycode(dpy, XK_Num_Lock))
          NumLockMask=(1<<m);
        if(*map==XKeysymToKeycode(dpy, XK_Caps_Lock))
          CapsLockMask=(1<<m);
        if(*map==XKeysymToKeycode(dpy, XK_Scroll_Lock))
          ScrollLockMask=(1<<m);
      }
    XFreeModifiermap(xmk);
  }
}

void grab_key(int keycode, unsigned int modifiers, Window w)
{
  if(keycode)
  {
    XGrabKey(dpy, keycode, modifiers, w, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, keycode, modifiers|NumLockMask, w, False, GrabModeAsync, GrabModeAsync); 
    XGrabKey(dpy, keycode, modifiers|CapsLockMask, w, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, keycode, modifiers|ScrollLockMask, w, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, keycode, modifiers|NumLockMask|CapsLockMask, w, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, keycode, modifiers|NumLockMask|ScrollLockMask, w, False, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, keycode, modifiers|NumLockMask|CapsLockMask|ScrollLockMask, w, False, GrabModeAsync, GrabModeAsync);
  }
}

void initialize(int argc, char *argv[])
{
  int i;
  struct sigaction act;

#ifdef DEBUG
  printf("initialize\n");
#endif

  locale = gtk_set_locale();
  gtk_init(&argc, &argv);
  for(i=1;i<argc;i++)
  {
    if(!strcmp(argv[i], "-quit")) quit_menu=True;
    if(!strcmp(argv[i], "-button=1")) button=Button1;
    if(!strcmp(argv[i], "-button=2")) button=Button2;
    if(!strcmp(argv[i], "-button=3")) button=Button3;
  }

  dpy=GDK_DISPLAY();
  root=GDK_ROOT_WINDOW();

  XSetErrorHandler(handle_xerror);

  init_keyboard();
  grab_key(XKeysymToKeycode(dpy, XK_Escape), ControlMask, root);

  act.sa_handler=signal_handler;
  act.sa_flags=0;
  sigaction(SIGCHLD, &act, NULL);

  /* Wait until a GNOME compliant WM is running */
  while(!XInternAtom(dpy, "_WIN_DESKTOP_BUTTON_PROXY", True));
  sleep(1);

  wm_state=XInternAtom(dpy, "WM_STATE", False);  
  gnome[WIN_HINTS]=XInternAtom(dpy, "_WIN_HINTS", False);
  gnome[WIN_WORKSPACE]=XInternAtom(dpy, "_WIN_WORKSPACE", False);  
  gnome[WIN_WORKSPACE_COUNT]=XInternAtom(dpy, "_WIN_WORKSPACE_COUNT", False);
  gnome[WIN_DESKTOP_BUTTON_PROXY]=XInternAtom(dpy, "_WIN_DESKTOP_BUTTON_PROXY", False);
  gnome[WIN_CLIENT_LIST]=XInternAtom(dpy, "_WIN_CLIENT_LIST", False);

  proxy_win=get_gnome_hint(root, WIN_DESKTOP_BUTTON_PROXY);
  XSelectInput(dpy, proxy_win, SubstructureNotifyMask);
  XSelectInput(dpy, root, PropertyChangeMask);
}

int main(int argc, char *argv[])
{
  initialize(argc, argv);
  while(1)
  {
    while(XPending(dpy))
    {
      XEvent ev;
      XNextEvent(dpy, &ev);

      if((ev.type==ButtonPress && ev.xbutton.button==button) || ev.type==KeyPress)
        popup_menu(&ev);
      if(ev.type==PropertyNotify && ev.xproperty.atom==gnome[WIN_DESKTOP_BUTTON_PROXY])
      {
        proxy_win=get_gnome_hint(root, WIN_DESKTOP_BUTTON_PROXY);
        XSelectInput(dpy, proxy_win, SubstructureNotifyMask);
      }
    }
    usleep(5);
  }
  return 0;
}
