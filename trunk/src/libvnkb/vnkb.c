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
 *      Nguyen Thai Ngoc Duy <pclouds@vnlinux.org>
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libintl.h>
#include "xvnkb.h"
#include "xunikey.h"
#include "uksync.h"
#include "keycons.h"
#include "vnkb.h"
#include <libintl.h>
#include <gdk/gdkx.h>

#define UK_PROG_NAME 	"Unikey XIM"
#define XVNKB_PROG_NAME "VISCKEY"

void vnkb_update_label(Vnkb *vnkb);
//------------------------------------------
void vnkb_xvnkb_update_switchkey(Vnkb *);

//Display *display = NULL; /* HACK: if libxvnkb.so is loaded, then the content of this might change */


/**
  Load atoms. xvnkbSync is useless.
  Also set driver for vnkb.
 */
void
vnkb_get_sync_atoms(Vnkb *vnkb,int xvnkbSync)
{
  GdkWindow *gdkroot = gdk_get_default_root_window();
  Display *_display = GDK_WINDOW_XDISPLAY(gdkroot);
  Window root = GDK_WINDOW_XID(gdkroot);
  Atom ProgAtom;		/* for detecting Unikey */

  ProgAtom = XInternAtom(_display, UK_PROG_NAME, False);
  if (XGetSelectionOwner(_display, ProgAtom) != None)
    vnkb_set_driver(vnkb,DRIVER_UNIKEY);
  else {
    vnkb_set_driver(vnkb,DRIVER_XVNKB);
    /** Enable this code if we find any way to recognize which one is running.
    //ProgAtom = XInternAtom(_display, XVNKB_PROG_NAME, False);
    //if (XGetSelectionOwner(_display,ProgAtom) != None)
    if (!display)
      vnkb_set_driver(vnkb,DRIVER_XVNKB);
    else {
      GtkWidget *dialog;
      dialog = gtk_message_dialog_new (NULL,
				       GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_MESSAGE_WARNING,
				       GTK_BUTTONS_CLOSE,
				       _("You have probably not run either Unikey or Xvnkb.\n"
				       "This program is useless without one of them"));
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
    }
    */
  }

  UkInitSync(_display,root);
  vnkb->xunikey->BSuspend = XInternAtom(_display, UKP_SUSPEND, False);

  vnkb->xvnkb->AIMCharset = XInternAtom(_display, VKP_CHARSET, False);
  vnkb->xvnkb->AIMMethod = XInternAtom(_display, VKP_METHOD, False);
  vnkb->xvnkb->AIMUsing = XInternAtom(_display, VKP_USING, False);
  vnkb->xvnkb->AIMSwitchKey = XInternAtom(_display,VKP_HOTKEY,0);
  vnkb->xvnkb->AIMSpelling = XInternAtom(_display,VKP_SPELLING,0);

  vnkb->xunikey->BIMCharset = XInternAtom(_display, UKP_CHARSET, False);
  vnkb->xunikey->BIMMethod = XInternAtom(_display, UKP_METHOD, False);
  vnkb->xunikey->BIMUsing = XInternAtom(_display, UKP_USING, False);

  vnkb->xunikey->BIMViqrStarCore = XInternAtom(_display, UKP_VIQR_STAR_CORE, False);
  vnkb->xunikey->BIMViqrStarGui = XInternAtom(_display, UKP_VIQR_STAR_GUI, False);

}

/**
   Set event driver. The callback is vnkb_event_filter_cb.
 */

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

/**
   Create the vnkb widget, then add it to @widget. 
   @widget acts as a container.
 */

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

  fish->tooltip = gtk_tooltips_new();

  gtk_widget_show_all (widget);
}

/**
   Setup IM's charset using atoms.
   If there is no atom, set VKC_UTF8.
 */

void vnkb_init_charset(Vnkb *vnkb)
{
  long v;

  v = UkGetPropValue(vnkb->driver == DRIVER_UNIKEY ? vnkb->xunikey->BIMCharset : vnkb->xvnkb->AIMCharset, VKC_UTF8);
  vnkb->charset = v;
  vnkb_update_charset(vnkb);
}

/**
   Setup IM's input method.
   The default is TELEX.
 */

void vnkb_init_method(Vnkb *vnkb)
{
  long v;
  v = UkGetPropValue(vnkb->driver == DRIVER_UNIKEY ? vnkb->xunikey->BIMMethod : vnkb->xvnkb->AIMMethod, VKM_TELEX);
  vnkb->method = v;

  vnkb_update_method(vnkb);
}

/**
   Setup IM's enabling.
   The default is TELEX.
 */

void vnkb_init_enabled(Vnkb *vnkb)
{
  long v;
  v = UkGetPropValue(vnkb->driver == DRIVER_UNIKEY ? vnkb->xunikey->BIMMethod : vnkb->xvnkb->AIMMethod, VKM_TELEX);
  vnkb->enabled = v != VKM_OFF;
  vnkb->backup_method = UkGetPropValue(vnkb->xvnkb->AIMUsing, VKM_TELEX);
  vnkb_update_enabled(vnkb);
}

/**
   Set vnkb's spelling state from vnkb->xvnkb->AIMSpelling.
 */

void vnkb_init_spelling(Vnkb *vnkb)
{
  long v;
  v = UkGetPropValue(vnkb->xvnkb->AIMSpelling, 0);
  vnkb->spelling = v;
  vnkb_update_spelling(vnkb);
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
  vnkb_update_label(applet);
}

void vnkb_update_method(Vnkb *applet)
{
  if (applet->update_method)
    applet->update_method(applet);
  vnkb_update_label(applet);
}

void vnkb_update_enabled(Vnkb *applet)
{
  if (applet->update_enabled)
    applet->update_enabled(applet);
  vnkb_update_label(applet);
}

void vnkb_update_spelling(Vnkb *applet)
{
  if (applet->update_spelling)
    applet->update_spelling(applet);
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
  
  
  if (ev->atom == vnkb->xvnkb->AIMCharset || ev->atom == vnkb->xunikey->BIMCharset) {
    v = UkGetPropValue(ev->atom, VKC_UTF8);
    vnkb_set_charset(vnkb,v);
    return GDK_FILTER_REMOVE;
  } else if (ev->atom == vnkb->xvnkb->AIMMethod || ev->atom == vnkb->xunikey->BIMMethod) {
    v = UkGetPropValue(ev->atom,VKM_TELEX);
    vnkb_set_method(vnkb,v);
    return GDK_FILTER_REMOVE;
  } else if (ev->atom == vnkb->xvnkb->AIMUsing || ev->atom == vnkb->xunikey->BIMUsing) {
    v = UkGetPropValue(ev->atom,VKM_TELEX);
    if (v != vnkb->backup_method && v != VKM_OFF)
      vnkb->backup_method = v;
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
      vnkb_update_enabled(vnkb);
      if (!vnkb->enabled) {
      	vnkb_set_method(vnkb,VKM_OFF);
      } else {
	if (vnkb->backup_method == VKM_OFF)
	  vnkb->backup_method = VKM_TELEX;
      	vnkb_set_method(vnkb,vnkb->backup_method);
      }
    }
  }
}

void
vnkb_set_spelling(Vnkb *vnkb,gboolean state)
{
  if (vnkb->set_spelling)
    vnkb->set_spelling(vnkb,state);
  else {
    if (state != vnkb->spelling) {
      vnkb->spelling = state;
      UkSetPropValue(vnkb->xvnkb->AIMSpelling,state);
    }
  }
}

void vnkb_set_charset(Vnkb *vnkb,int cs)
{
  if (vnkb->charset != cs) {
    vnkb->charset = cs;
    if (vnkb->driver == DRIVER_UNIKEY &&
	(cs == VKC_VPS || cs == VKC_VISCII))
      vnkb_set_charset(vnkb,
		       vnkb->charset != VKC_VPS && vnkb->charset != VKC_VISCII ? 
		       vnkb->charset : VKC_UTF8);
    else {
      UkSetPropValue(vnkb->xvnkb->AIMCharset,vnkb->charset);
      UkSetPropValue(vnkb->xunikey->BIMCharset,vnkb->charset);
    }
    vnkb_update_charset(vnkb);
  }
}

void vnkb_set_method(Vnkb *vnkb,int im)
{
  long v;

  if (im == VKM_VIQR) {
    v = UkGetPropValue(vnkb->xunikey->BIMViqrStarGui,0);
    if (v) {
      im = VKM_VIQR_STAR;
      UkSetPropValue(vnkb->xunikey->BIMViqrStarGui,0);
    }
  }

  if (vnkb->method != im) {
    if (im != VKM_OFF)
      vnkb->backup_method = im;
    vnkb->method = im;
    switch (im) {
    case VKM_VIQR_STAR:
      if (vnkb->driver == DRIVER_UNIKEY) {
	UkSetPropValue(vnkb->xunikey->BIMViqrStarCore, 1);
	UkSetPropValue(vnkb->xunikey->BIMViqrStarGui, 1);
        UkSetPropValue(vnkb->xunikey->BIMMethod,VKM_VIQR);
      } else
	UkSetPropValue(vnkb->xvnkb->AIMMethod,VKM_VIQR);
      break;
/*      
    case VKM_VIQR:
      UkSetPropValue(vnkb->xvnkb->AIMMethod,VKM_VIQR);
      UkSetPropValue(vnkb->xunikey->BIMMethod,VKM_VIQR);
      break;
*/
    default:
      if (im == VKM_OFF) {
      	if (vnkb->backup_method == VKM_OFF)
	  vnkb->backup_method = VKM_TELEX;
	UkSetPropValue(vnkb->xvnkb->AIMUsing,vnkb->backup_method);
	UkSetPropValue(vnkb->xunikey->BIMUsing,vnkb->backup_method);
      }
      UkSetPropValue(vnkb->xvnkb->AIMMethod,vnkb->method);
      UkSetPropValue(vnkb->xunikey->BIMMethod,vnkb->method);
    }
    vnkb_set_enabled(vnkb,im != VKM_OFF);
    vnkb_update_method(vnkb);
  }
}

const char* charset_to_text(int cs)
{
  switch (cs) {
  case VKC_UTF8: return _("Unicode");
  case VKC_VNI: return _("Vni");
  case VKC_VISCII: return _("Viscii");
  case VKC_VPS: return _("Vps");
  case VKC_TCVN: return _("Tcvn3");
  default: return _("Unknown");
  }
}

const char* method_to_text(int im)
{
  switch (im) {
  case VKM_OFF: return _("Off");
  case VKM_VNI: return _("Vni");
  case VKM_TELEX: return _("Telex");
  case VKM_VIQR: return _("Viqr");
  case VKM_VIQR_STAR: return _("Viqr*");
  default: return _("Unknown");
  }
}

void vnkb_set_label_mode(Vnkb *vnkb,int mode)
{
  vnkb->label_mode = mode;
  vnkb_update_label(vnkb);
}

void vnkb_update_label(Vnkb *vnkb)
{
  const char *label;
  int type;
  char *new_label = NULL;

  if (vnkb->method == VKM_OFF)
    new_label = g_strdup_printf(_("Charset: %s\nInput method: %s (Off)"),
			    charset_to_text(vnkb->charset),
			    method_to_text(vnkb->backup_method));
  else
    new_label = g_strdup_printf(_("Charset: %s\nInput method: %s"),
			    charset_to_text(vnkb->charset),
			    method_to_text(vnkb->method));
  if (new_label) {
    gtk_tooltips_set_tip(vnkb->tooltip,
			 vnkb->button,
			 new_label,
			 NULL);
    g_free(new_label);
    new_label = NULL;
  }

  gtk_widget_modify_fg(vnkb->label,
		       GTK_STATE_NORMAL,
		       vnkb->enabled ? &vnkb->color_enabled : &vnkb->color_disabled);
  /*gtk_widget_modify_bg(vnkb->button,GTK_STATE_NORMAL,!vnkb->enabled ? &vnkb->color_enabled : &vnkb->color_disabled);
  gtk_widget_modify_fg(vnkb->button,GTK_STATE_NORMAL,vnkb->enabled ? &vnkb->color_enabled : &vnkb->color_disabled);*/

  switch (vnkb->label_mode) {
  case VNKB_LABEL_CUSTOM:
    if (vnkb->enabled)
      label = vnkb->text_enabled ? vnkb->text_enabled : "V";
    else
      label = vnkb->text_disabled ? vnkb->text_disabled : "N";
    break;

  case VNKB_LABEL_IM: 
    label = method_to_text(vnkb->method);
    break;

  default: 
    label = vnkb->enabled ? "V" : "N";
    break;
  }

  if (vnkb->enabled && vnkb->font_enabled) {
    label = new_label = g_strdup_printf("<span font_desc=\"%s\">%s</span>",vnkb->font_enabled,label);
  }
  if (!vnkb->enabled && vnkb->font_disabled) {
    label = new_label = g_strdup_printf("<span font_desc=\"%s\">%s</span>",vnkb->font_disabled,label);
  }
  if (new_label) {
    gtk_label_set_markup(GTK_LABEL(vnkb->label),new_label);
    g_free(new_label);
  } else
    gtk_label_set(GTK_LABEL(vnkb->label),label);
}

void vnkb_xvnkb_update_switchkey(Vnkb *vnkb)
{
  UkSetPropValues(vnkb->xvnkb->AIMSwitchKey,&vnkb->xvnkb->hotkey,2);
}

void vnkb_set_driver(Vnkb *vnkb,int driver)
{
  if (driver != DRIVER_XVNKB && driver != DRIVER_UNIKEY)
    return;
  vnkb->driver = driver;
  if (vnkb->driver_changed)
    vnkb->driver_changed(vnkb);
};

void vnkb_init(Vnkb *vnkb,GtkWidget *container)
{
  vnkb->xvnkb = g_new0(Xvnkb,1);
  vnkb->xunikey = g_new0(Xunikey,1);
  vnkb_setup_widget(vnkb,container);
  vnkb_get_sync_atoms(vnkb,TRUE);
  vnkb_init_charset(vnkb);
  vnkb_init_method(vnkb);
  vnkb_init_enabled(vnkb);
  vnkb_init_spelling(vnkb);
  vnkb_set_event_filter(vnkb,TRUE);
  vnkb_load_config(vnkb);

  vnkb->initialized = TRUE;	/* done */
}

void vnkb_cleanup(Vnkb *vnkb)
{
  vnkb_save_config(vnkb);
  if (vnkb->disable_on_exit) {
    vnkb_set_enabled(vnkb,FALSE);
    /*
      because this function is call right before the program exits, 
      we should flush the display to make sure xvnkb/unikey get it
    */
    gdk_display_flush(gdk_display_get_default());
  }
  vnkb_set_event_filter(vnkb,FALSE);
  g_free(vnkb->xvnkb);
  g_free(vnkb->xunikey);
}
