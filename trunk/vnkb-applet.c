/* vnkb-applet.c:
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
 * Modified from fish.c (gnome-applet)
 */

#include <string.h>

#include <gtk/gtk.h>
#include <panel-applet.h>

#define VNKB_APPLET(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), \
			vnkb_applet_get_type(),          \
			VnkbApplet))
#define VNKB_IS_APPLET(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
			   VNKB_TYPE_APPLET))

typedef struct {
	PanelApplet        applet;

	PanelAppletOrient  orientation;

} VnkbApplet;

typedef struct {
	PanelAppletClass klass;
} VnkbAppletClass;


static GType vnkb_applet_get_type (void);

static GObjectClass *parent_class;

static void
setup_vnkb_widget (VnkbApplet *fish)
{
	GtkWidget *widget = (GtkWidget *) fish;
	GtkWidget *label = gtk_label_new(_("V"));

	gtk_container_add (GTK_CONTAINER (widget), label);

//	g_signal_connect (fish, "key_press_event",
//			  G_CALLBACK (handle_keypress), fish);

	gtk_widget_show_all (widget);
}

static gboolean
vnkb_applet_fill (VnkbApplet *fish)
{
	PanelApplet *applet = (PanelApplet *) fish;
	GError      *error = NULL;

	fish->orientation = panel_applet_get_orient (applet);

	setup_vnkb_widget (fish);

	return TRUE;
}

static gboolean
vnkb_factory (PanelApplet *applet,
	       const char  *iid,
	       gpointer     data)
{
	gboolean retval = FALSE;

	if (!strcmp (iid, "OAFIID:VnkbApplet"))
		retval = vnkb_applet_fill (VNKB_APPLET (applet));

	return retval;
}

static void
vnkb_applet_destroy (GtkObject *object)
{
	VnkbApplet *fish = (VnkbApplet *) object;
	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
vnkb_applet_instance_init (VnkbApplet      *fish,
			   VnkbAppletClass *klass)
{
	int i;

	fish->orientation = PANEL_APPLET_ORIENT_UP;
	panel_applet_set_flags (PANEL_APPLET (fish),
				PANEL_APPLET_EXPAND_MINOR);
}

static void
vnkb_applet_change_orient (PanelApplet       *applet,
			   PanelAppletOrient  orientation)
{
	VnkbApplet *fish = (VnkbApplet *) applet;

	if (fish->orientation == orientation)
		return;

	fish->orientation = orientation;

}

static void
vnkb_applet_class_init (VnkbAppletClass *klass)
{
	PanelAppletClass *applet_class    = (PanelAppletClass *) klass;
	GtkObjectClass   *gtkobject_class = (GtkObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	applet_class->change_orient = vnkb_applet_change_orient;

	gtkobject_class->destroy = vnkb_applet_destroy;

}

static GType
vnkb_applet_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (PanelAppletClass),
			NULL, NULL,
			(GClassInitFunc) vnkb_applet_class_init,
			NULL, NULL,
			sizeof (VnkbApplet),
			0,
			(GInstanceInitFunc) vnkb_applet_instance_init,
			NULL
		};

		type = g_type_register_static (
				PANEL_TYPE_APPLET, "VnkbApplet", &info, 0);
	}

	return type;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:VnkbApplet_Factory",
			     vnkb_applet_get_type (),
			     "Vietnamese-keyboard-applet",
			     "0",
			     vnkb_factory,
			     NULL)
