/* vnkb.c:
 *
 * Copyright (C) 1998-2002 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors:
 *      pclouds <pclouds@vnlinux.org>
 *
 */

#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "xvnkb.h"
#include "uksync.h"
#include "keycons.h"
#include "vnkb.h"

//------------------------------------------
Atom AIMCharset, AIMUsing, AIMMethod, AIMViqrStarGui, AIMViqrStarCore;
Atom ASuspend;

void
vnkb_get_sync_atoms(int xvnkbSync)
{
  GdkWindow *gdkroot = gdk_get_default_root_window();
  Display *display = GDK_WINDOW_XDISPLAY(gdkroot);
  Window root = GDK_WINDOW_XID(gdkroot);

  UkInitSync(display,root);
  ASuspend = XInternAtom(display, UKP_SUSPEND, False);

  if (xvnkbSync) {
    AIMCharset = XInternAtom(display, VKP_CHARSET, False);
    AIMMethod = XInternAtom(display, VKP_METHOD, False);
    AIMUsing = XInternAtom(display, VKP_USING, False);
  }
  else {
    AIMCharset = XInternAtom(display, UKP_CHARSET, False);
    AIMMethod = XInternAtom(display, UKP_METHOD, False);
    AIMUsing = XInternAtom(display, UKP_USING, False);
  }

  AIMViqrStarCore = XInternAtom(display, UKP_VIQR_STAR_CORE, False);
  AIMViqrStarGui = XInternAtom(display, UKP_VIQR_STAR_GUI, False);

  /*
  v = UkGetPropValue(AIMCharset, VKC_UTF8);
  GlobalOpt.charset = SyncToUnikeyCharset((int)v);

  v = UkGetPropValue(AIMMethod, VKM_TELEX);
  GlobalOpt.enabled = (v != VKM_OFF);

  if (!GlobalOpt.enabled)
    v = UkGetPropValue(AIMUsing, VKM_TELEX);

  GlobalOpt.inputMethod = SyncToUnikeyMethod((int)v);

  if (GlobalOpt.inputMethod == VIQR_INPUT) {
    v = UkGetPropValue(AIMViqrStarCore, 0);
    if (v != 0)
      GlobalOpt.inputMethod = VIQR_STAR_INPUT;
    UkSetPropValue(AIMViqrStarCore, 0);
  }
  */
}

void
vnkb_set_event_filter(Vnkb *applet,int enable)
{
  static GdkEventMask oldMask;
  //static int filterAdded = 0; 
  GdkEventMask mask;
  GdkWindow *root = gdk_get_default_root_window();

  if (enable && !applet->filter_set) {
    mask = gdk_window_get_events(root);
    oldMask = mask;
    mask |= GDK_PROPERTY_CHANGE_MASK;
      
    gdk_window_set_events(root,mask);
      
    //fprintf(stderr, "Registering...\n");
    /* register an event filter */
    gdk_window_add_filter(root,
			  (GdkFilterFunc)vnkb_event_filter_cb,
			  applet);
    applet->filter_set = TRUE;
  } else if (!enable && applet->filter_set) {
    gdk_window_set_events(root,oldMask);
    //fprintf(stderr, "Unregistering...\n");
    ///* unregister event filter */
    gdk_window_remove_filter(root,
			     (GdkFilterFunc)vnkb_event_filter_cb,
			     applet);
    applet->filter_set = FALSE;
  }
}

void
vnkb_setup_widget (Vnkb *fish,GtkWidget *widget)
{
  fish->button = gtk_button_new();
  fish->label = gtk_label_new(fish->enabled ? "V" : "N");

  gtk_container_add (GTK_CONTAINER (fish->button), fish->label);
  gtk_container_add (GTK_CONTAINER (widget), fish->button);


  g_signal_connect(G_OBJECT(fish->button),
		   "clicked",
		   G_CALLBACK(vnkb_clicked_cb),
		   fish);
  //	g_signal_connect (fish, "key_press_event",
  //			  G_CALLBACK (handle_keypress), fish);

  if (fish->widget_setup)
    fish->widget_setup(fish,widget);
  gtk_widget_show_all (widget);
}

void vnkb_init_charset(Vnkb *vnkb)
{
  long v;

  v = UkGetPropValue(AIMCharset, VKC_UTF8);
  vnkb->charset = SyncToUnikeyCharset((int)v);
  vnkb_update_charset(vnkb);
}

void vnkb_init_method(Vnkb *vnkb)
{
  long v;
  v = UkGetPropValue(AIMMethod, VKM_TELEX);
  vnkb->method = v;

  if (!(v != VKM_OFF))
    vnkb->backup_method = UkGetPropValue(AIMUsing, VKM_TELEX);
  vnkb_update_method(vnkb);
}

void vnkb_init_enabled(Vnkb *vnkb)
{
  long v;
  v = UkGetPropValue(AIMMethod, VKM_TELEX);
  vnkb->enabled = v != VKM_OFF;
  vnkb_update_enabled(vnkb);
}

void
vnkb_toggle_enabled(Vnkb *applet)
{
  vnkb_set_enabled(applet,!applet->enabled);
}

void vnkb_clicked_cb(GtkButton *button,Vnkb *vnkb)
{
  if (vnkb->clicked_cb)
    vnkb->clicked_cb(button,vnkb);
  else
    vnkb_toggle_enabled(vnkb);
}

void vnkb_update_charset(Vnkb *applet)
{
  if (applet->update_charset)
    applet->update_charset(applet);
}

void vnkb_update_method(Vnkb *applet)
{
  if (applet->update_method)
    applet->update_method(applet);
}

void vnkb_update_enabled(Vnkb *applet)
{
  if (applet->update_enabled)
    applet->update_enabled(applet);
}

GdkFilterReturn
vnkb_event_filter_cb(GdkXEvent 	*xevent,
		     GdkEvent 	*event,
		     Vnkb *vnkb)
{
  if (vnkb->event_filter_cb) {
    vnkb->event_filter_cb(xevent,event,vnkb);
    return GDK_FILTER_CONTINUE;
  }

  XPropertyEvent *ev = (XPropertyEvent*)xevent;
  long v;
  GdkWindow *gdkroot = gdk_get_default_root_window();
  Window root = GDK_WINDOW_XID(gdkroot);

  if (!ev ||
      ev->type != PropertyNotify ||
      ev->window != root)
    return GDK_FILTER_CONTINUE;
  
  
  if (ev->atom == AIMCharset) {
    v = UkGetPropValue(ev->atom, VKC_UTF8);
    vnkb->charset = SyncToUnikeyCharset(v);
    vnkb_update_charset(vnkb);
    return GDK_FILTER_REMOVE;
  } else if (ev->atom == AIMMethod) {
    //fixSyncToUnikeyMethod();
    //if (applet->vnkb.enabled)
    //UnikeySetInputMethod(GlobalOpt.inputMethod);
    vnkb->method = UkGetPropValue(ev->atom,VKM_TELEX);
    vnkb->enabled = vnkb->method != VKM_OFF;
    if (!vnkb->enabled)
      vnkb->backup_method = UkGetPropValue(AIMUsing, VKM_TELEX);
    vnkb_update_enabled(vnkb);
    vnkb_update_method(vnkb);
    return GDK_FILTER_REMOVE;
  } else if (ev->atom == AIMUsing) {
    //dont' need this
    return GDK_FILTER_REMOVE;
  }

  return GDK_FILTER_CONTINUE;
}

void
vnkb_set_enabled(Vnkb *vnkb,gboolean state)
{
  if (vnkb->set_enabled)
    vnkb->set_enabled(vnkb,state);
  else {
    if (state != vnkb->enabled) {
      vnkb->enabled = state;
      gtk_label_set_text(GTK_LABEL(vnkb->label),vnkb->enabled ? "V" : "N");
      if (!vnkb->enabled) {
	if (vnkb->method != VKM_OFF)
	  vnkb->backup_method = vnkb->method;
	UkSetPropValue(AIMUsing,vnkb->method);
	UkSetPropValue(AIMMethod,VKM_OFF);
      } else {
	vnkb->method = vnkb->backup_method;
	UkSetPropValue(AIMMethod,vnkb->method);
      }
    }
  }
}

void vnkb_set_charset(Vnkb *vnkb,int cs)
{
  if (vnkb->charset != cs) {
    vnkb->charset = cs;
    UkSetPropValue(AIMCharset,vnkb->charset);
  }
}

void vnkb_set_method(Vnkb *vnkb,int im)
{
  if (vnkb->method != im) {
    vnkb->method = im;
    UkSetPropValue(AIMMethod,vnkb->method);
  }
}

