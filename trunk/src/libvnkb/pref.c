/* pref.c:
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
#include "vnkb.h"
#include <libintl.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <glade/glade-xml.h>
#include "eggcellrendererkeys.h"

typedef struct
{
  guint keyval;
  guint keycode;
  EggVirtualModifierType mask;
} KeyEntry;


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

