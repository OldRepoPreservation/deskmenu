/* 
 * deskmenu - keyboard.c
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

int NumLockMask, CapsLockMask, ScrollLockMask;

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

/* vim600:set foldmarker={{{,}}} foldmethod=marker: */

