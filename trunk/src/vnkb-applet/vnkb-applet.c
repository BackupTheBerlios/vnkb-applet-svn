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

#include <panel-applet.h>
#include <libgnomeui/libgnomeui.h>
#include "vnkb.h"
#include <gdk/gdkx.h>
#include "xvnkb.h"
#include "uksync.h"
#include "keycons.h"

#define VNKB_APPLET(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), \
			vnkb_applet_get_type(),          \
			VnkbApplet))
#define VNKB_IS_APPLET(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), \
			   VNKB_TYPE_APPLET))

typedef struct {
  PanelApplet   	 applet;

  PanelAppletOrient   	orientation;
  Vnkb 			vnkb;
} VnkbApplet;

typedef struct {
	PanelAppletClass klass;
} VnkbAppletClass;


static GType vnkb_applet_get_type (void);

static GObjectClass *parent_class;

static void vnkb_applet_show_preferences (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname);
static void vnkb_applet_show_help (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname);
static void vnkb_applet_show_about (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname);

static void vnkb_applet_dummy_handler (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname);
static void vnkb_applet_ui_component_event (BonoboUIComponent *comp,
				     const gchar                  *path,
				     Bonobo_UIComponent_EventType  type,
				     const gchar                  *state_string,
				     VnkbApplet                    *data);

static void vnkb_applet_update_charset(Vnkb *applet);
static void vnkb_applet_update_method(Vnkb *applet);
static void vnkb_applet_update_enabled(Vnkb *applet);

static gboolean button_press_hack (GtkWidget *w, GdkEventButton *event, gpointer data);

static const BonoboUIVerb vnkb_menu_verbs [] = {
  BONOBO_UI_UNSAFE_VERB ("Props", vnkb_applet_show_preferences),
  BONOBO_UI_UNSAFE_VERB ("Help",  vnkb_applet_show_help),
  BONOBO_UI_UNSAFE_VERB ("About", vnkb_applet_show_about),

  BONOBO_UI_UNSAFE_VERB ("IM_Off", vnkb_applet_dummy_handler),
  BONOBO_UI_UNSAFE_VERB ("IM_Vni", vnkb_applet_dummy_handler),
  BONOBO_UI_UNSAFE_VERB ("IM_Telex", vnkb_applet_dummy_handler),
  BONOBO_UI_UNSAFE_VERB ("IM_Viqr", vnkb_applet_dummy_handler),

  BONOBO_UI_UNSAFE_VERB ("CS_Unicode", vnkb_applet_dummy_handler),
  BONOBO_UI_UNSAFE_VERB ("CS_Tcvn3", vnkb_applet_dummy_handler),
  BONOBO_UI_UNSAFE_VERB ("CS_Vni", vnkb_applet_dummy_handler),
  BONOBO_UI_UNSAFE_VERB ("CS_Viqr", vnkb_applet_dummy_handler),
  BONOBO_UI_UNSAFE_VERB ("CS_Viscii", vnkb_applet_dummy_handler),
  BONOBO_UI_UNSAFE_VERB ("CS_Vps", vnkb_applet_dummy_handler),

  BONOBO_UI_VERB_END
};


static void
vnkb_applet_driver_changed(Vnkb *vnkb)
{
  GtkWidget *w;
  PanelApplet *applet = (PanelApplet*)vnkb->panel;
  BonoboUIComponent *component;
  gboolean is_xvnkb = vnkb->driver == DRIVER_XVNKB;

  component = panel_applet_get_popup_component (applet);


  bonobo_ui_component_set_prop (component,"/commands/IM_StarViqr",
				"hidden",(!is_xvnkb ? "0" : "1"),NULL);
  bonobo_ui_component_set_prop (component,"/commands/CS_Vps",
				"hidden",(is_xvnkb ? "0" : "1"),NULL);
  bonobo_ui_component_set_prop (component,"/commands/CS_Viscii",
				"hidden",(is_xvnkb ? "0" : "1"),NULL);
  bonobo_ui_component_set_prop (component,"/commands/Spell",
				"hidden",(is_xvnkb ? "0" : "1"),NULL);
}

static gboolean
vnkb_applet_fill (VnkbApplet *fish)
{
  PanelApplet *applet = (PanelApplet *) fish;
  BonoboUIComponent *component;
  //GError      *error = NULL;
  Vnkb *vnkb = &fish->vnkb;

  fish->orientation = panel_applet_get_orient (applet);

  panel_applet_setup_menu_from_file (applet,
				     NULL,
				     "VnkbApplet.xml",
				     NULL,
				     vnkb_menu_verbs,
				     fish);
	
	
  component = panel_applet_get_popup_component (applet);
  g_signal_connect (component,
		    "ui-event",
		    (GCallback) vnkb_applet_ui_component_event,
		    fish);

  vnkb_init (vnkb,GTK_WIDGET(fish));

  g_signal_connect(G_OBJECT(vnkb->button),
		   "button-press-event",
		   G_CALLBACK(button_press_hack),
		   vnkb->panel);
  vnkb_set_event_filter(vnkb,TRUE);

  fish->vnkb.initialized = TRUE;	/* done */

  return TRUE;
}


static void
vnkb_applet_update_enabled(Vnkb *vnkb)
{
  VnkbApplet *fish = (VnkbApplet*)vnkb->panel;
  PanelApplet *applet = (PanelApplet *) fish;
  BonoboUIComponent *component;
  component = panel_applet_get_popup_component (applet);
  bonobo_ui_component_set_prop (component,
				"/commands/Enable",
				"state",
				(fish->vnkb.enabled ? "1" : "0"),
				NULL);
}

static void
vnkb_applet_update_spelling(Vnkb *vnkb)
{
  VnkbApplet *fish = (VnkbApplet*)vnkb->panel;
  PanelApplet *applet = (PanelApplet *) fish;
  BonoboUIComponent *component;
  component = panel_applet_get_popup_component (applet);
  bonobo_ui_component_set_prop (component,
				"/commands/Spell",
				"state",
				(fish->vnkb.spelling ? "1" : "0"),
				NULL);
}

static void
vnkb_applet_update_charset(Vnkb *vnkb)
{
  VnkbApplet *fish = (VnkbApplet*)vnkb->panel;
  PanelApplet *applet = (PanelApplet *) fish;
  BonoboUIComponent *component;
  char *cmd;

  component = panel_applet_get_popup_component (applet);
  switch (fish->vnkb.charset) {
  case VKC_UTF8: cmd = "/commands/CS_Unicode"; break;
  case VKC_TCVN: cmd = "/commands/CS_Tcvn3"; break;
  case VKC_VNI: cmd = "/commands/CS_Vni"; break;
  case VKC_VIQR: cmd ="/commands/CS_Viqr"; break;
  case VKC_VISCII: cmd="/commands/CS_Viscii"; break;
  case VKC_VPS: cmd="/commands/CS_Vps"; break;
  default: cmd = NULL;
  }
  if (cmd) {
    //bonobo_ui_component_set_prop (component, "/commands/CS_Unicode", "state", "0", NULL);
    bonobo_ui_component_set_prop (component, cmd, "state", "1", NULL);
  }

}

static void
vnkb_applet_update_method(Vnkb *vnkb)
{
  VnkbApplet *fish = (VnkbApplet*)vnkb->panel;
  PanelApplet *applet = (PanelApplet *) fish;
  BonoboUIComponent *component;
  char *cmd;

  component = panel_applet_get_popup_component (applet);
  switch (fish->vnkb.method) {
  case VKM_OFF: cmd = "/commands/IM_Off"; break;
  case VKM_VIQR: cmd = "/commands/IM_Viqr"; break;
  case VKM_VIQR_STAR: cmd = "/commands/IM_StarViqr"; break;
  case VKM_VNI: cmd = "/commands/IM_Vni"; break;
  case VKM_TELEX: cmd = "/commands/IM_Telex"; break;
  default:cmd = NULL;
  }

  if (cmd) {
    FILE *fp = fopen("/tmp/log","a");
    fprintf(fp,"Update method %s\n",cmd);
    fclose(fp);
    //bonobo_ui_component_set_prop (component, "/commands/IM_Off", "state", "0", NULL);
    bonobo_ui_component_set_prop (component, cmd, "state", "1", NULL);
  }

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
  vnkb_cleanup(&fish->vnkb);
  GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
vnkb_applet_instance_init (VnkbApplet      *fish,
			   VnkbAppletClass *klass)
{
  fish->vnkb.panel = fish;

  fish->vnkb.update_charset = vnkb_applet_update_charset;
  fish->vnkb.update_method = vnkb_applet_update_method;
  fish->vnkb.update_enabled = vnkb_applet_update_enabled;
  fish->vnkb.update_spelling = vnkb_applet_update_spelling;
  fish->vnkb.driver_changed = vnkb_applet_driver_changed;

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

static void
vnkb_applet_show_preferences (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname)
{
  vnkb_show_preferences(&mc->vnkb);
}

static void
vnkb_applet_show_help (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname)
{
}

static void
vnkb_applet_show_about (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname)
{
}


static void vnkb_applet_dummy_handler (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname)
{
}

static void vnkb_applet_ui_component_event (BonoboUIComponent 			*comp,
					    const gchar                  	*path,
					    Bonobo_UIComponent_EventType  	 type,
					    const gchar                  	*state_string,
					    VnkbApplet                    	*data)
{

  if (!strcmp(path,"Enable")) {
    FILE *fp = fopen("/tmp/log","a");
    gboolean state;
    state = !strcmp(state_string,"1");
    fprintf(fp,"Enable %d\n",state);
    fclose(fp);
    vnkb_set_enabled(&data->vnkb,state);
    return;
  }

  if (!strcmp(path,"Spell")) {
    gboolean state;
    state = !strcmp(state_string,"1");
    vnkb_set_spelling(&data->vnkb,state);
    return;
  }

  if (!strncmp(path,"IM_",3)) {
    int im = VKM_OFF;
    if (!strcmp(path,"IM_Off")) im = VKM_OFF;
    else if (!strcmp(path,"IM_Vni")) im = VKM_VNI;
    else if (!strcmp(path,"IM_Telex")) im = VKM_TELEX;
    else if (!strcmp(path,"IM_Viqr")) im = VKM_VIQR;
    else if (!strcmp(path,"IM_StarViqr")) im = VKM_VIQR_STAR;

    if (!strcmp(state_string,"1")) {
      FILE *fp = fopen("/tmp/log","a");
      fprintf(fp,"Method %d\n",im);
      fclose(fp);
      vnkb_set_method(&data->vnkb,im);
    }

    return;
  }

  if (!strncmp(path,"CS_",3)) {
    int cs = VKC_UTF8;
    if (!strcmp(path,"CS_Unicode")) cs = VKC_UTF8;
    else if (!strcmp(path,"CS_Tcvn3")) cs = VKC_TCVN;
    else if (!strcmp(path,"CS_Vni")) cs = VKC_VNI;
    else if (!strcmp(path,"CS_Viqr")) cs = VKC_VIQR;
    else if (!strcmp(path,"CS_Viscii")) cs = VKC_VISCII;   
    else if (!strcmp(path,"CS_Vps")) cs = VKC_VPS;

    if (!strcmp(state_string,"1"))
      vnkb_set_charset(&data->vnkb,cs);

    return;
  }
}

// stolen from gdict-applet
static gboolean 
button_press_hack (GtkWidget *w, GdkEventButton *event, gpointer data)
{
    GtkWidget *applet = GTK_WIDGET (data);
    
    if (event->button == 3 || event->button == 2) {
	gtk_propagate_event (applet, (GdkEvent *) event);

	return TRUE;
    }
    
    return FALSE;
    
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:VnkbApplet_Factory",
			     vnkb_applet_get_type (),
			     "Vietnamese-keyboard-applet",
			     "0",
			     vnkb_factory,
			     NULL)
