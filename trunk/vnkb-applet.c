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
#include <gdk/gdkx.h>
#include <panel-applet.h>
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

  PanelAppletOrient   	 orientation;
  GtkWidget         	*label;

  gboolean      	 enabled;
  int           	 method;
  int           	 charset;
  gboolean 		 initialized;
  gboolean		 filter_set;
} VnkbApplet;

typedef struct {
	PanelAppletClass klass;
} VnkbAppletClass;


static GType vnkb_applet_get_type (void);

static GObjectClass *parent_class;

static void show_preferences (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname);
static void show_help (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname);
static void show_about (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname);

static void Dummy_Handler (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname);

static void vnkb_ui_component_event (BonoboUIComponent *comp,
				     const gchar                  *path,
				     Bonobo_UIComponent_EventType  type,
				     const gchar                  *state_string,
				     VnkbApplet                    *data);

static void vnkb_get_sync_atoms(int xvnkbSync);
static void vnkb_set_event_filter(VnkbApplet *applet,int enable);
static void vnkb_update_charset(VnkbApplet *applet);
static void vnkb_update_method(VnkbApplet *applet);
static void vnkb_update_enabled(VnkbApplet *applet);


//------------------------------------------
static Atom AIMCharset, AIMUsing, AIMMethod, AIMViqrStarGui, AIMViqrStarCore;
static Atom ASuspend;

static const BonoboUIVerb vnkb_menu_verbs [] = {
  BONOBO_UI_UNSAFE_VERB ("Props", show_preferences),
  BONOBO_UI_UNSAFE_VERB ("Help",  show_help),
  BONOBO_UI_UNSAFE_VERB ("About", show_about),

  BONOBO_UI_UNSAFE_VERB ("IM_Off", Dummy_Handler),
  BONOBO_UI_UNSAFE_VERB ("IM_Vni", Dummy_Handler),
  BONOBO_UI_UNSAFE_VERB ("IM_Telex", Dummy_Handler),
  BONOBO_UI_UNSAFE_VERB ("IM_Viqr", Dummy_Handler),

  BONOBO_UI_UNSAFE_VERB ("CS_Unicode", Dummy_Handler),
  BONOBO_UI_UNSAFE_VERB ("CS_Tcvn3", Dummy_Handler),
  BONOBO_UI_UNSAFE_VERB ("CS_Vni", Dummy_Handler),
  BONOBO_UI_UNSAFE_VERB ("CS_Viqr", Dummy_Handler),
  BONOBO_UI_UNSAFE_VERB ("CS_Viscii", Dummy_Handler),
  BONOBO_UI_UNSAFE_VERB ("CS_Vps", Dummy_Handler),

  BONOBO_UI_VERB_END
};


static void
setup_vnkb_widget (VnkbApplet *fish)
{
  GtkWidget *widget = (GtkWidget *) fish;
  fish->label = gtk_label_new(fish->enabled ? _("V") : _("N"));

  gtk_container_add (GTK_CONTAINER (widget), fish->label);

  //	g_signal_connect (fish, "key_press_event",
  //			  G_CALLBACK (handle_keypress), fish);

  gtk_widget_show_all (widget);
}

static gboolean
vnkb_applet_fill (VnkbApplet *fish)
{
  PanelApplet *applet = (PanelApplet *) fish;
  BonoboUIComponent *component;
  GError      *error = NULL;
  long v;

  GdkWindow *gdkroot = gdk_get_default_root_window();
  Window root = GDK_WINDOW_XID(gdkroot);
  Display *display = GDK_WINDOW_XDISPLAY(gdkroot);

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
		    (GCallback) vnkb_ui_component_event,
		    fish);

  UkInitSync(display,root);
  vnkb_get_sync_atoms(TRUE);
  v = UkGetPropValue(AIMCharset, VKC_UTF8);
  fish->charset = SyncToUnikeyCharset((int)v);
  vnkb_update_charset(fish);
	
  v = UkGetPropValue(AIMMethod, VKM_TELEX);
  fish->method = v;
  vnkb_update_method(fish);

  fish->enabled = v != VKM_OFF;
  if (!fish->enabled)
    fish->method = UkGetPropValue(AIMUsing, VKM_TELEX);
  vnkb_update_enabled(fish);

  setup_vnkb_widget (fish);
  vnkb_set_event_filter(fish,TRUE);

  fish->initialized = TRUE;	/* done */

  return TRUE;
}

static void
vnkb_update_enabled(VnkbApplet *fish)
{
  PanelApplet *applet = (PanelApplet *) fish;
  BonoboUIComponent *component;
  component = panel_applet_get_popup_component (applet);
  bonobo_ui_component_set_prop (component,
				"/commands/Enable",
				"state",
				(fish->enabled ? "1" : "0"),
				NULL);
  gtk_label_set_text(GTK_LABEL(fish->label),fish->enabled ? _("V") : _("N"));
}

static void
vnkb_update_charset(VnkbApplet *fish)
{
  PanelApplet *applet = (PanelApplet *) fish;
  BonoboUIComponent *component;
  char *cmd;

  component = panel_applet_get_popup_component (applet);
  switch (fish->charset) {
  case UNICODE_CHARSET: cmd = "/commands/CS_Unicode"; break;
  case TCVN3_CHARSET: cmd = "/commands/CS_Tcvn3"; break;
  case VNI_CHARSET: cmd = "/commands/CS_Vni"; break;
  case VIQR_CHARSET: cmd ="/commands/CS_Viqr"; break;
  //case VISCII_CHARSET: cmd="/commands/CS_Viscii"; break;
  //case VPS_CHARSET: cmd="/commands/CS_Vps"; break;
  default: cmd = NULL;
  }
  if (cmd) {
    //bonobo_ui_component_set_prop (component, "/commands/CS_Unicode", "state", "0", NULL);
    bonobo_ui_component_set_prop (component, cmd, "state", "1", NULL);
  }

}

static void
vnkb_update_method(VnkbApplet *fish)
{
  PanelApplet *applet = (PanelApplet *) fish;
  BonoboUIComponent *component;
  char *cmd;

  component = panel_applet_get_popup_component (applet);
  switch (fish->method) {
  case VKM_OFF: cmd = "/commands/IM_Off"; break;
  case VKM_VIQR: cmd = "/commands/IM_Viqr"; break;
  case VKM_VNI: cmd = "/commands/IM_Vni"; break;
  case VKM_TELEX: cmd = "/commands/IM_Telex"; break;
  default:cmd = NULL;
  }

  if (cmd) {
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
  vnkb_set_event_filter(fish,FALSE);
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

static void
show_preferences (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname)
{
  GtkWidget *dlg = gtk_message_dialog_new(NULL,
					  GTK_DIALOG_DESTROY_WITH_PARENT,
					  GTK_MESSAGE_ERROR,
					  GTK_BUTTONS_CLOSE,
					  "haha");
  gtk_dialog_run(GTK_DIALOG(dlg));
  gtk_widget_destroy(dlg);
}

static void
show_help (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname)
{
}

static void
show_about (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname)
{
}


static GdkFilterReturn
vnkb_event_filter_cb(GdkXEvent 	*xevent,
		     GdkEvent 	*event,
		     VnkbApplet *applet)
{
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
    applet->charset = SyncToUnikeyCharset(v);
    vnkb_update_charset(applet);
    return GDK_FILTER_REMOVE;
  } else if (ev->atom == AIMMethod) {
    //fixSyncToUnikeyMethod();
    //if (applet->enabled)
    //UnikeySetInputMethod(GlobalOpt.inputMethod);
    applet->method = UkGetPropValue(ev->atom,VKM_TELEX);
    applet->enabled = applet->method != VKM_OFF;
    if (!applet->enabled)
      applet->method = UkGetPropValue(AIMUsing, VKM_TELEX);
    vnkb_update_enabled(applet);
    vnkb_update_method(applet);
    return GDK_FILTER_REMOVE;
  } else if (ev->atom == AIMUsing) {
    //dont' need this
    return GDK_FILTER_REMOVE;
  }

  return GDK_FILTER_CONTINUE;
}

static void
vnkb_set_event_filter(VnkbApplet *applet,int enable)
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

static void vnkb_get_sync_atoms(int xvnkbSync)
{
  GdkWindow *gdkroot = gdk_get_default_root_window();
  Display *display = GDK_WINDOW_XDISPLAY(gdkroot);

  long v;
  
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

static FILE *fp = NULL;

static void Dummy_Handler (BonoboUIComponent *uic, VnkbApplet *mc, const char *verbname)
{
}

static void vnkb_ui_component_event (BonoboUIComponent 			*comp,
				     const gchar                  	*path,
				     Bonobo_UIComponent_EventType  	 type,
				     const gchar                  	*state_string,
				     VnkbApplet                    	*data)
{

  if (!strcmp(path,"Enable")) {
    gboolean state;
    state = !strcmp(state_string,"1");
    if (state != data->enabled) {
      data->enabled = state;
      gtk_label_set_text(GTK_LABEL(data->label),data->enabled ? _("V") : _("N"));
      if (!data->enabled) {
	UkSetPropValue(AIMUsing,data->method);
	UkSetPropValue(AIMMethod,VKM_OFF);
      } else
	UkSetPropValue(AIMMethod,data->method);

      if (!fp) fp = fopen("/tmp/log","wt");
      fprintf(fp,"Enable %d\n",state);
      fflush(fp);
    }
    return;
  }

  if (!strcmp(path,"IM")) {
    if (!fp) fp = fopen("/tmp/log","wt");
    fprintf(fp,"ZZ %s %d %s\n",path,type,state_string);
    fflush(fp);
  }

  if (!strncmp(path,"IM_",3)) {
    int im = VKM_OFF;
    if (!strcmp(path,"IM_Off")) im = VKM_OFF;
    else if (!strcmp(path,"IM_Vni")) im = VKM_VNI;
    else if (!strcmp(path,"IM_Telex")) im = VKM_TELEX;
    else if (!strcmp(path,"IM_Viqr")) im = VKM_VIQR;
		
    if (!strcmp(state_string,"1") && data->method != im) {
      data->method = im;
      UkSetPropValue(AIMMethod,data->method);

      if (!fp) fp = fopen("/tmp/log","wt");
      fprintf(fp,"ZZ %s %d %s\n",path,type,state_string);
      fflush(fp);
    }
    return;
  }

  if (!strncmp(path,"CS_",3)) {
    int cs = UNICODE_CHARSET;
    if (!strcmp(path,"CS_Unicode")) cs = UNICODE_CHARSET;
    else if (!strcmp(path,"CS_Tcvn3")) cs = TCVN3_CHARSET;
    else if (!strcmp(path,"CS_Vni")) cs = VNI_CHARSET;
    else if (!strcmp(path,"CS_Viqr")) cs = VIQR_CHARSET;
    //else if (!strcmp(path,"CS_Viscii")) cs = CS_VISCII;   
    //else if (!strcmp(path,"CS_Vps")) cs = CS_VPS;

    if (!strcmp(state_string,"1") && data->charset != cs) {
      data->charset = cs;
      UkSetPropValue(AIMCharset,data->charset);

      if (!fp) fp = fopen("/tmp/log","wt");
      fprintf(fp,"ZZ %s %d %s\n",path,type,state_string);
      fflush(fp);
    }
    return;
  }
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:VnkbApplet_Factory",
			     vnkb_applet_get_type (),
			     "Vietnamese-keyboard-applet",
			     "0",
			     vnkb_factory,
			     NULL)
