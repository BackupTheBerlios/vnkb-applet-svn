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

#include <libintl.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade-xml.h>
#include "xvnkb.h"
#include "uksync.h"
#include "keycons.h"
#include "vnkb.h"
#include "eggcellrendererkeys.h"

#ifndef _
#define _(x) dgettext (GETTEXT_PACKAGE, x)
#define N_(x) x
#endif

typedef struct
{
  guint keyval;
  guint keycode;
  EggVirtualModifierType mask;
} KeyEntry;


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
      vnkb_update_label(vnkb);
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

static void
accel_set_func (GtkTreeViewColumn *tree_column,
                GtkCellRenderer   *cell,
                GtkTreeModel      *model,
                GtkTreeIter       *iter,
                gpointer           data)
{
  KeyEntry *key_entry;
  
  gtk_tree_model_get (model, iter,
                      1, &key_entry,
                      -1);

  if (key_entry == NULL)
    g_object_set (G_OBJECT (cell),
		  "visible", FALSE,
		  NULL);
  else
    g_object_set (G_OBJECT (cell),
		  "visible", TRUE,
		  "accel_key", key_entry->keyval,
		  "accel_mask", key_entry->mask,
		  "keycode", key_entry->keycode,
		  NULL);
}


static void
shortcut_edited_cb(GtkCellRendererText   *cell,
		   const char            *path_string,
		   guint                  keyval,
		   EggVirtualModifierType mask,
		   guint		  keycode,
		   gpointer               data)
{
  GtkTreeModel *model = GTK_TREE_MODEL(data);
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter;
  KeyEntry *key_entry, tmp_key;
  GError *err = NULL;
  char *str;

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter,
		      1, &key_entry,
		      -1);

  /* sanity check */
  if (key_entry == NULL) {
    gtk_tree_path_free (path);
    return;
  }


  tmp_key.keyval = keyval;
  tmp_key.keycode = keycode;
  tmp_key.mask   = mask;
  *key_entry = tmp_key;
  gtk_tree_model_row_changed (model, path, &iter);
  gtk_tree_path_free (path);
}

static void
radio_use_default_clicked_cb(GtkButton *b,Vnkb *vnkb)
{
  vnkb_set_label_mode(vnkb,VNKB_LABEL_DEFAULT);
  gtk_widget_set_sensitive(vnkb->widget_text_enabled,FALSE);
  gtk_widget_set_sensitive(vnkb->widget_text_disabled,FALSE);
}

static void
radio_custom_text_clicked_cb(GtkButton *b,Vnkb *vnkb)
{
  vnkb_set_label_mode(vnkb,VNKB_LABEL_CUSTOM);
  gtk_widget_set_sensitive(vnkb->widget_text_enabled,TRUE);
  gtk_widget_set_sensitive(vnkb->widget_text_disabled,TRUE);
}

static void
radio_show_im_clicked_cb(GtkButton *b,Vnkb *vnkb)
{
  vnkb_set_label_mode(vnkb,VNKB_LABEL_IM);
  gtk_widget_set_sensitive(vnkb->widget_text_enabled,FALSE);
  gtk_widget_set_sensitive(vnkb->widget_text_disabled,FALSE);
}

static void
entry_enabled_changed_cb(GtkEntry *b,Vnkb *vnkb)
{
  if (vnkb->text_enabled)
    g_free(vnkb->text_enabled);
  vnkb->text_enabled = g_strdup(gtk_entry_get_text(GTK_ENTRY(vnkb->widget_text_enabled)));
}

static void
entry_disabled_changed_cb(GtkEntry *b,Vnkb *vnkb)
{
  if (vnkb->text_disabled)
    g_free(vnkb->text_disabled);
  vnkb->text_disabled = g_strdup(gtk_entry_get_text(GTK_ENTRY(vnkb->widget_text_disabled)));
}


static void
colorbutton_disabled_set_cb(GtkColorButton *b,Vnkb *vnkb)
{
  gtk_color_button_get_color(b,&vnkb->color_disabled);
  vnkb_update_label(vnkb);
}

static void
colorbutton_enabled_set_cb(GtkColorButton *b,Vnkb *vnkb)
{
  gtk_color_button_get_color(b,&vnkb->color_enabled);
  vnkb_update_label(vnkb);
}

void vnkb_show_preferences (Vnkb *vnkb)
{
  GladeXML *xml;
  GtkWidget *dlg,*button,*w;
  GtkTreeViewColumn *col;
  GtkWidget *treeview;
  GtkListStore *store;
  GtkTreeIter iter;
  GtkCellRenderer *cell;

  xml = glade_xml_new (VNKB_GLADEDIR "/vnkb-preferences.glade", NULL, GETTEXT_PACKAGE);
  dlg = glade_xml_get_widget(xml,"vnkb_preferences_dialog");

  vnkb->widget_text_enabled = glade_xml_get_widget(xml,"entry_enabled");
  g_signal_connect(G_OBJECT(vnkb->widget_text_enabled),"changed",
		   G_CALLBACK(entry_enabled_changed_cb),
		   vnkb);
  if (vnkb->text_enabled)
    gtk_entry_set_text(GTK_ENTRY(vnkb->text_enabled),vnkb->text_enabled);

  vnkb->widget_text_disabled = glade_xml_get_widget(xml,"entry_disabled");
  g_signal_connect(G_OBJECT(vnkb->widget_text_enabled),"changed",
		   G_CALLBACK(entry_disabled_changed_cb),
		   vnkb);
  if (vnkb->text_disabled)
    gtk_entry_set_text(GTK_ENTRY(vnkb->text_enabled),vnkb->text_disabled);

  //gtk_dialog_set_default_response (GTK_DIALOG (dlg), GTK_RESPONSE_OK);

  treeview = glade_xml_get_widget(xml,"treeview_shortcut");
  store = gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_POINTER);
  gtk_tree_view_set_model(GTK_TREE_VIEW(treeview),GTK_TREE_MODEL(store));
  col = gtk_tree_view_column_new_with_attributes(_("Function"),
						 gtk_cell_renderer_text_new(),
						 "text",
						 0,
						 NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW(treeview),col);

  cell = (GtkCellRenderer *) g_object_new (EGG_TYPE_CELL_RENDERER_KEYS,
					   "editable", TRUE,
					   NULL);
  g_signal_connect(G_OBJECT(cell),"accel_edited",
		   G_CALLBACK(shortcut_edited_cb),
		   store);
  col = gtk_tree_view_column_new_with_attributes(_("Shortcut"),cell,NULL);
  gtk_tree_view_column_set_cell_data_func (col, cell, accel_set_func, NULL, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW(treeview),col);

  gtk_list_store_append(store,&iter);
  gtk_list_store_set(store,&iter,
		     0,"Enable",
		     1,g_new0(KeyEntry,1),
		     -1);

  switch (vnkb->label_mode) {
  case VNKB_LABEL_DEFAULT:  w = glade_xml_get_widget (xml, "radio_use_default"); break;
  case VNKB_LABEL_CUSTOM: w = glade_xml_get_widget (xml, "radio_custom_text"); break;
  case VNKB_LABEL_IM: w = glade_xml_get_widget (xml, "radio_show_im"); break;
  }

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w),TRUE);

  w = glade_xml_get_widget (xml, "radio_use_default");
  g_signal_connect(G_OBJECT(w),"clicked",
		   G_CALLBACK(radio_use_default_clicked_cb),
		   vnkb);
  w = glade_xml_get_widget (xml, "radio_custom_text");
  g_signal_connect(G_OBJECT(w),"clicked",
		   G_CALLBACK(radio_custom_text_clicked_cb),
		   vnkb);
  w = glade_xml_get_widget (xml, "radio_show_im");
  g_signal_connect(G_OBJECT(w),"clicked",
		   G_CALLBACK(radio_show_im_clicked_cb),
		   vnkb);

  button = glade_xml_get_widget (xml, "colorbutton_enabled");
  gtk_color_button_set_color(GTK_COLOR_BUTTON(button),
			     &vnkb->color_enabled);
  g_signal_connect (button, "color-set",
		    G_CALLBACK(colorbutton_enabled_set_cb), 
		    vnkb);

  button = glade_xml_get_widget (xml, "colorbutton_disabled");
  gtk_color_button_set_color(GTK_COLOR_BUTTON(button),
			     &vnkb->color_disabled);
  g_signal_connect (button, "color-set",
		    G_CALLBACK(colorbutton_disabled_set_cb), 
		    vnkb);

  button = glade_xml_get_widget (xml, "button_close");
  g_signal_connect_swapped (button, "clicked",
			    (GCallback) gtk_widget_destroy, 
			    dlg);

  //gtk_window_set_screen (GTK_WINDOW (dlg),
  //gtk_widget_get_screen (GTK_WIDGET (fish)));
  
  //gtk_window_set_resizable (GTK_WINDOW (fish->preferences_dialog), FALSE);
  gtk_window_present (GTK_WINDOW (dlg));

  g_object_unref (xml);

}

void vnkb_set_label_mode(Vnkb *vnkb,int mode)
{
  vnkb->label_mode = mode;
  vnkb_update_label(vnkb);
}

void vnkb_update_label(Vnkb *vnkb)
{
  char *label;
  int type;


  gtk_widget_modify_fg(vnkb->label,GTK_STATE_NORMAL,vnkb->enabled ? &vnkb->color_enabled : &vnkb->color_disabled);

  switch (vnkb->label_mode) {
  case VNKB_LABEL_CUSTOM:
    if (vnkb->enabled)
      label = vnkb->text_enabled ? vnkb->text_enabled : "V";
    else
      label = vnkb->text_disabled ? vnkb->text_disabled : "N";
    break;

  case VNKB_LABEL_IM: 
    switch (vnkb->method) {
    case VKM_OFF: label = _("Off");break;
    case VKM_VNI: label = _("Vni"); break;
    case VKM_TELEX: label = _("Telex"); break;
    case VKM_VIQR: label = _("Viqr"); break;
    default: label = vnkb->enabled ? "V" : "N"; break;
    }
    break;
  default: label = vnkb->enabled ? "V" : "N"; break;
  }
  gtk_label_set_text(GTK_LABEL(vnkb->label),label);
}
