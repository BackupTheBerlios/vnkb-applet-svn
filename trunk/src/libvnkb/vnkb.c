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
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade-xml.h>
#include "xvnkb.h"
#include "uksync.h"
#include "keycons.h"
#include "vnkb.h"
#include "eggcellrendererkeys.h"

#define UK_PROG_NAME "Unikey XIM"
#define XVNKB_PROG_NAME             "VISCKEY"

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


void vnkb_update_label(Vnkb *vnkb);
//------------------------------------------
Atom AIMCharset, AIMUsing, AIMMethod;
Atom BIMCharset, BIMUsing, BIMMethod;
Atom BIMViqrStarGui, BIMViqrStarCore;
Atom BSuspend,AIMSwitchKey,AIMSpelling;

void vnkb_xvnkb_update_switchkey(Vnkb *,int,int);

//Display *display = NULL; /* HACK: if libxvnkb.so is loaded, then the content of this might change */

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
    /*
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
  BSuspend = XInternAtom(_display, UKP_SUSPEND, False);

  AIMCharset = XInternAtom(_display, VKP_CHARSET, False);
  AIMMethod = XInternAtom(_display, VKP_METHOD, False);
  AIMUsing = XInternAtom(_display, VKP_USING, False);
  AIMSwitchKey = XInternAtom(_display,VKP_HOTKEY,0);
  AIMSpelling = XInternAtom(_display,VKP_SPELLING,0);

  BIMCharset = XInternAtom(_display, UKP_CHARSET, False);
  BIMMethod = XInternAtom(_display, UKP_METHOD, False);
  BIMUsing = XInternAtom(_display, UKP_USING, False);

  BIMViqrStarCore = XInternAtom(_display, UKP_VIQR_STAR_CORE, False);
  BIMViqrStarGui = XInternAtom(_display, UKP_VIQR_STAR_GUI, False);

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

  fish->tooltip = gtk_tooltips_new();

  gtk_widget_show_all (widget);
}

void vnkb_init_charset(Vnkb *vnkb)
{
  long v;

  v = UkGetPropValue(vnkb->driver == DRIVER_UNIKEY ? BIMCharset : AIMCharset, VKC_UTF8);
  vnkb->charset = v;
  vnkb_update_charset(vnkb);
}

void vnkb_init_method(Vnkb *vnkb)
{
  long v;
  v = UkGetPropValue(vnkb->driver == DRIVER_UNIKEY ? BIMMethod : AIMMethod, VKM_TELEX);
  vnkb->method = v;

  vnkb_update_method(vnkb);
}

void vnkb_init_enabled(Vnkb *vnkb)
{
  long v;
  v = UkGetPropValue(vnkb->driver == DRIVER_UNIKEY ? BIMMethod : AIMMethod, VKM_TELEX);
  vnkb->enabled = v != VKM_OFF;
  vnkb->backup_method = UkGetPropValue(AIMUsing, VKM_TELEX);
  vnkb_update_enabled(vnkb);
}

void vnkb_init_spelling(Vnkb *vnkb)
{
  long v;
  v = UkGetPropValue(AIMSpelling, 0);
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
  
  
  if (ev->atom == AIMCharset || ev->atom == BIMCharset) {
    v = UkGetPropValue(ev->atom, VKC_UTF8);
    vnkb_set_charset(vnkb,v);
    return GDK_FILTER_REMOVE;
  } else if (ev->atom == AIMMethod || ev->atom == BIMMethod) {
    v = UkGetPropValue(ev->atom,VKM_TELEX);
    vnkb_set_method(vnkb,v);
    return GDK_FILTER_REMOVE;
  } else if (ev->atom == AIMUsing || ev->atom == BIMUsing) {
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
      vnkb->enabled = state;
      UkSetPropValue(AIMSpelling,state);
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
      UkSetPropValue(AIMCharset,vnkb->charset);
      UkSetPropValue(BIMCharset,vnkb->charset);
    }
    vnkb_update_charset(vnkb);
  }
}

void vnkb_set_method(Vnkb *vnkb,int im)
{
  long v;

  if (im == VKM_VIQR) {
    v = UkGetPropValue(BIMViqrStarGui,0);
    if (v) {
      im = VKM_VIQR_STAR;
      UkSetPropValue(BIMViqrStarGui,0);
    }
  }

  if (vnkb->method != im) {
    if (im != VKM_OFF)
      vnkb->backup_method = im;
    vnkb->method = im;
    switch (im) {
    case VKM_VIQR_STAR:
      if (vnkb->driver == DRIVER_UNIKEY) {
	UkSetPropValue(BIMViqrStarCore, 1);
	UkSetPropValue(BIMViqrStarGui, 1);
        UkSetPropValue(BIMMethod,VKM_VIQR);
      } else
	UkSetPropValue(AIMMethod,VKM_VIQR);
      break;
/*      
    case VKM_VIQR:
      UkSetPropValue(AIMMethod,VKM_VIQR);
      UkSetPropValue(BIMMethod,VKM_VIQR);
      break;
*/
    default:
      if (im == VKM_OFF) {
      	if (vnkb->backup_method == VKM_OFF)
	  vnkb->backup_method = VKM_TELEX;
	UkSetPropValue(AIMUsing,vnkb->backup_method);
	UkSetPropValue(BIMUsing,vnkb->backup_method);
      }
      UkSetPropValue(AIMMethod,vnkb->method);
      UkSetPropValue(BIMMethod,vnkb->method);
    }
    vnkb_set_enabled(vnkb,im != VKM_OFF);
    vnkb_update_method(vnkb);
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
  Vnkb *vnkb = (Vnkb*)data;
  GtkTreeModel *model = GTK_TREE_MODEL(vnkb->store);
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
  vnkb_xvnkb_update_switchkey(vnkb,mask,keycode);
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
  vnkb->text_enabled = g_strdup(gtk_entry_get_text(b));
  vnkb_update_label(vnkb);
}

static void
entry_disabled_changed_cb(GtkEntry *b,Vnkb *vnkb)
{
  if (vnkb->text_disabled)
    g_free(vnkb->text_disabled);
  vnkb->text_disabled = g_strdup(gtk_entry_get_text(b));
  vnkb_update_label(vnkb);
}

static void
button_disable_exit_toggled_cb(GtkToggleButton *b,Vnkb *vnkb)
{
  vnkb->disable_on_exit = gtk_toggle_button_get_active(b);
}

static void
fontbutton_enabled_set_cb(GtkFontButton *b,Vnkb *vnkb)
{
  if (vnkb->font_enabled)
    g_free(vnkb->font_enabled);
  vnkb->font_enabled = g_strdup(gtk_font_button_get_font_name(b));
  vnkb_update_label(vnkb);
}

static void
fontbutton_disabled_set_cb(GtkFontButton *b,Vnkb *vnkb)
{
  if (vnkb->font_disabled)
    g_free(vnkb->font_disabled);
  vnkb->font_disabled = g_strdup(gtk_font_button_get_font_name(b));
  vnkb_update_label(vnkb);
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

static void
radio_xvnkb_clicked_cb(GtkRadioButton *b,Vnkb *vnkb)
{
  vnkb_set_driver(vnkb,DRIVER_XVNKB);
}

static void
radio_unikey_clicked_cb(GtkRadioButton *b,Vnkb *vnkb)
{
  vnkb_set_driver(vnkb,DRIVER_UNIKEY);
}

static void
button_unikey_macro_browse_clicked_cb(GtkButton *button, Vnkb *vnkb)
{
  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new (_("Choose Macro File"),
					NULL,
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					NULL);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename;

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    g_free (filename);
  }

  gtk_widget_destroy (dialog);
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
  GdkPixbuf *icon;

  xml = glade_xml_new (VNKB_GLADEDIR "/vnkb-preferences.glade", NULL, GETTEXT_PACKAGE);
  dlg = glade_xml_get_widget(xml,"vnkb_preferences_dialog");


  vnkb->widget_text_enabled = w = glade_xml_get_widget(xml,"entry_enabled");
  g_signal_connect(G_OBJECT(w),"changed",
		   G_CALLBACK(entry_enabled_changed_cb),
		   vnkb);
  if (vnkb->text_enabled)
    gtk_entry_set_text(GTK_ENTRY(w),vnkb->text_enabled);

  vnkb->widget_text_disabled = w = glade_xml_get_widget(xml,"entry_disabled");
  g_signal_connect(G_OBJECT(w),"changed",
		   G_CALLBACK(entry_disabled_changed_cb),
		   vnkb);
  if (vnkb->text_disabled)
    gtk_entry_set_text(GTK_ENTRY(w),vnkb->text_disabled);

  //gtk_dialog_set_default_response (GTK_DIALOG (dlg), GTK_RESPONSE_OK);

  treeview = glade_xml_get_widget(xml,"treeview_shortcut");
  vnkb->store = store = gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_POINTER);
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
		   vnkb);
  col = gtk_tree_view_column_new_with_attributes(_("Shortcut"),cell,NULL);
  gtk_tree_view_column_set_cell_data_func (col, cell, accel_set_func, NULL, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW(treeview),col);

  gtk_list_store_append(store,&iter);
  gtk_list_store_set(store,&iter,
		     0,_("Enable Xvnkb"),
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

  button = glade_xml_get_widget (xml, "fontbutton_enabled");
  if (vnkb->font_enabled)
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(button),
				  vnkb->font_enabled);
  g_signal_connect (button, "font-set",
		    G_CALLBACK(fontbutton_enabled_set_cb), 
		    vnkb);

  button = glade_xml_get_widget (xml, "button_disable_exit");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),vnkb->disable_on_exit);
  g_signal_connect (button, "toggled",
		    G_CALLBACK(button_disable_exit_toggled_cb),
		    vnkb);

  button = glade_xml_get_widget (xml, "fontbutton_disabled");
  if (vnkb->font_disabled)
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(button),
				  vnkb->font_disabled);
  g_signal_connect (button, "font-set",
		    G_CALLBACK(fontbutton_disabled_set_cb), 
		    vnkb);

  button = glade_xml_get_widget (xml, "button_close");
  g_signal_connect_swapped (button, "clicked",
			    (GCallback) gtk_widget_destroy, 
			    dlg);

  button = glade_xml_get_widget (xml, "radio_xvnkb");
  g_signal_connect (button, "clicked",
		    (GCallback) radio_xvnkb_clicked_cb, 
		    vnkb);

  button = glade_xml_get_widget (xml, "radio_unikey");
  if (vnkb->driver == DRIVER_UNIKEY)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),TRUE);
  g_signal_connect (button, "clicked",
		    (GCallback) radio_unikey_clicked_cb, 
		    vnkb);

  //gtk_window_set_screen (GTK_WINDOW (dlg),
  //gtk_widget_get_screen (GTK_WIDGET (fish)));
  
  button = glade_xml_get_widget(xml,"buton_unikey_macro_browse");
  g_signal_connect(G_OBJECT(button),"clicked",
		   (GCallback) button_unikey_macro_browse_clicked_cb,
		   vnkb);

  icon = gtk_widget_render_icon (dlg,
				 GTK_STOCK_PREFERENCES,
				 GTK_ICON_SIZE_MENU,
				 "vnkb_preferences_dialog");
  gtk_window_set_icon (GTK_WINDOW (dlg), icon);


  //gtk_window_set_resizable (GTK_WINDOW (fish->preferences_dialog), FALSE);
  gtk_window_present (GTK_WINDOW (dlg));

  g_object_unref (xml);

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

void vnkb_xvnkb_update_switchkey(Vnkb *vnkb,int state,int code)
{
  vk_hotkey_info hk;
  GdkWindow *gdkroot = gdk_get_default_root_window();
  Display *display = GDK_WINDOW_XDISPLAY(gdkroot);

  hk.state = state;
  hk.sym = XKeycodeToKeysym(display,code,0);
  if (state & VK_SHIFT)
    hk.sym = toupper(hk.sym);
  UkSetPropValues(AIMSwitchKey,&hk,2);
}

void vnkb_set_driver(Vnkb *vnkb,int driver)
{
  if (driver != DRIVER_XVNKB && driver != DRIVER_UNIKEY)
    return;
  vnkb->driver = driver;
  if (vnkb->driver_changed)
    vnkb->driver_changed(vnkb);
};

void vnkb_import_param(Vnkb *vnkb,const char *name,const char *param)
{
  gboolean enable;
  char *sep;

  if (!strcmp(name,"charset")) {
    if (!strcasecmp(param,"VNI"))
      vnkb_set_charset(vnkb,VKC_VNI);
    else if (!strcasecmp(param,"UTF8"))
      vnkb_set_charset(vnkb,VKC_UTF8);
    else if (!strcasecmp(param,"TCVN3"))
      vnkb_set_charset(vnkb,VKC_TCVN);
    else if (!strcasecmp(param,"VISCII"))
      vnkb_set_charset(vnkb,VKC_VISCII);
    else if (!strcasecmp(param,"VPS"))
      vnkb_set_charset(vnkb,VKC_VPS);
    else if (!strcasecmp(param,"VIQR"))
      vnkb_set_charset(vnkb,VKC_VIQR);
    else
      fprintf(stderr,"Unknown charset %s\n",param);

    return;
  }

  if (!strcmp(name,"method")) {
    if (!strcasecmp(param,"VNI"))
      vnkb_set_method(vnkb,VKM_VNI);
    else if (!strcasecmp(param,"VIQR"))
      vnkb_set_method(vnkb,VKM_VIQR);
    else if (!strcasecmp(param,"VIQR*"))
      vnkb_set_method(vnkb,VKM_VIQR_STAR);
    else if (!strcasecmp(param,"TELEX"))
      vnkb_set_method(vnkb,VKM_TELEX);
    else if (!strcasecmp(param,"OFF"))
      vnkb_set_method(vnkb,VKM_OFF);
    else
      fprintf(stderr,"Unknown input method %s\n",param);

    return;
  }

  if (!strcmp(name,"label_mode")) {
    if (!strcasecmp(param,"default"))
      vnkb_set_label_mode(vnkb,VNKB_LABEL_DEFAULT);
    else if (!strcasecmp(param,"custom"))
      vnkb_set_label_mode(vnkb,VNKB_LABEL_CUSTOM);
    else if (!strcasecmp(param,"im"))
      vnkb_set_label_mode(vnkb,VNKB_LABEL_IM);
    else
      fprintf(stderr,"Unknown label mode %s\n",param);
    vnkb_update_label(vnkb);
    return;
  }


  if (!strcmp(name,"xvnkb_spell")) {
    if (!strcasecmp(param,"on"))
      vnkb_set_spelling(vnkb,TRUE);
    else if (!strcasecmp(param,"off"))
      vnkb_set_spelling(vnkb,FALSE);
    else
      fprintf(stderr,"Unknown value %s of setting %s\n",param,name);
    return;
  }

  if (!strcmp(name,"disable_on_exit")) {
    if (!strcasecmp(param,"on"))
      vnkb->disable_on_exit = TRUE;
    else if (!strcasecmp(param,"off"))
      vnkb->disable_on_exit = FALSE;
    else
      fprintf(stderr,"Unknown value %s of setting %s\n",param,name);
    return;
  }

  if (!strncmp(name,"enabled_",strlen("enabled_")))
    enable = TRUE;
  else if (!strncmp(name,"disabled_",strlen("disabled_")))
    enable = FALSE;
  else {
    fprintf(stderr,"Unknown parameter %s\n",name);
    return;
  }
  sep = strchr(name,'_');
  sep ++;
  if (!strcmp(sep,"text")) {
    if (enable)
      vnkb->text_enabled = g_strdup(param);
    else
      vnkb->text_disabled = g_strdup(param);
    vnkb_update_label(vnkb);
    return;
  }
  if (!strcmp(sep,"color")) {
    unsigned int r,g,b;
    GdkColor *color;
    sscanf(param,"%x:%x:%x",&r,&g,&b);
    if (enable)
      color = &vnkb->color_enabled;
    else
      color = &vnkb->color_disabled;

    color->red = r;
    color->green = g;
    color->blue = b;
    vnkb_update_label(vnkb);
    return;
  }
  if (!strcmp(sep,"font")) {
    if (enable)
      vnkb->font_enabled = g_strdup(param);
    else
      vnkb->font_disabled = g_strdup(param);
    vnkb_update_label(vnkb);
    return;
  }
}

void vnkb_load_config(Vnkb *vnkb)
{
  char *home = getenv("HOME");
  char *filename;

  if (home) {
    filename = g_strdup_printf("%s/.vnkb-applet.conf",home);
    if (filename) {
      FILE *fp = fopen(filename,"r");
      if (fp) {
	char buffer[256];
	char *sep;
	char *param;
	int n = 0;

	while (fgets(buffer,sizeof(buffer),fp)) {
	  int l = strlen(buffer);
	  n ++;
	  if (l && buffer[l-1] != '\n') {
	    fprintf(stderr,"Error: line %d in file %s too long\n",n,filename);
	    continue;
	  }

	  if (!l)
	    continue;

	  buffer[l-1] = 0;

	  if (buffer[0] == '#')
	    continue;		/* comment */

	  sep = strchr(buffer,'=');
	  if (!sep) {
	    fprintf(stderr,"Error: line %d in file %s not valid\n",n,filename);
	    continue;
	  }

	  param = sep+1;
	  *sep = 0;

	  vnkb_import_param(vnkb,buffer,param);
	}

	fclose(fp);
      }
      g_free(filename);
    }
  }
}

void vnkb_save_config(Vnkb *vnkb)
{
  char *home = getenv("HOME");
  char *filename;

  if (home) {
    filename = g_strdup_printf("%s/.vnkb-applet.conf",home);
    if (filename) {
      FILE *fp = fopen(filename,"w");
      if (fp) {
	char *param;

	switch (vnkb->charset) {
	case VKC_UTF8: param = "UTF8"; break;
	case VKC_TCVN: param = "TCVN3"; break;
	case VKC_VNI: param = "VNI"; break;
	case VKC_VIQR: param ="VIQR"; break;
	case VKC_VISCII: param ="VISCII"; break;
	case VKC_VPS: param ="VPS"; break;
	default: param = NULL;
	}
	if (param)
	  fprintf(fp,"charset=%s\n",param);

	switch (vnkb->method) {
	case VKM_OFF: param = "OFF"; break;
	case VKM_VIQR: param = "VIQR"; break;
	case VKM_VIQR_STAR: param = "VIQR*"; break;
	case VKM_VNI: param = "VNI"; break;
	case VKM_TELEX: param = "TELEX"; break;
	default:param = NULL;
	}
	if (param)
	  fprintf(fp,"method=%s\n",param);

	switch (vnkb->label_mode) {
	case VNKB_LABEL_IM: param = "im"; break;
	case VNKB_LABEL_DEFAULT: param = "default"; break;
	case VNKB_LABEL_CUSTOM: param = "custom"; break;
	default: param = NULL;
	}
	if (param)
	  fprintf(fp,"label_mode=%s\n",param);

	if (vnkb->text_enabled)
	  fprintf(fp,"enabled_text=%s\n",vnkb->text_enabled);
	if (vnkb->text_disabled)
	  fprintf(fp,"disabled_text=%s\n",vnkb->text_disabled);

	if (vnkb->font_enabled)
	  fprintf(fp,"enabled_font=%s\n",vnkb->font_enabled);
	if (vnkb->font_disabled)
	  fprintf(fp,"disabled_font=%s\n",vnkb->font_disabled);

	fprintf(fp,"enabled_color=%x:%x:%x\n",
		(unsigned)vnkb->color_enabled.red,
		(unsigned)vnkb->color_enabled.green,
		(unsigned)vnkb->color_enabled.blue);

	fprintf(fp,"disabled_color=%x:%x:%x\n",
		(unsigned)vnkb->color_disabled.red,
		(unsigned)vnkb->color_disabled.green,
		(unsigned)vnkb->color_disabled.blue);


	fprintf(fp,"disable_on_exit=%s\n",vnkb->disable_on_exit ? "On" : "Off");
	fprintf(fp,"xvnkb_spell=%s\n",vnkb->spelling ? "On" : "Off");

	fclose(fp);
      }
    }
    g_free(filename);
  }
}

void vnkb_init(Vnkb *vnkb,GtkWidget *container)
{
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
}
