#include <gtk/gtk.h>
#include "eggtrayicon.h"
#include "vnkb.h"
#include "xvnkb.h"
#include "keycons.h"

typedef struct {
  EggTrayIcon *docklet;
  Vnkb *vnkb;
  GtkWidget *menu;
  GtkUIManager *uim;
  GtkActionGroup *actions;
} VnkbDocklet;

static void vnkb_docklet_update_charset(Vnkb *applet);
static void vnkb_docklet_update_method(Vnkb *applet);
static void vnkb_docklet_update_enabled(Vnkb *applet);
static void vnkb_docklet_update_spelling(Vnkb *applet);

static void
im_change_cb(GtkAction *action, GtkRadioAction *current,Vnkb *vnkb)
{
  int im = VKM_OFF;
  const char *path = gtk_action_get_name (GTK_ACTION (current));

  if (!strcmp(path,"IM_Off")) im = VKM_OFF;
  else if (!strcmp(path,"IM_Vni")) im = VKM_VNI;
  else if (!strcmp(path,"IM_Telex")) im = VKM_TELEX;
  else if (!strcmp(path,"IM_Viqr")) im = VKM_VIQR;
  else if (!strcmp(path,"IM_StarViqr")) im = VKM_VIQR_STAR;

  vnkb_set_method(vnkb,im);
}

static void
cs_change_cb(GtkAction *action, GtkRadioAction *current,Vnkb *vnkb)
{
  int cs = VKC_UTF8;
  const char *path = gtk_action_get_name (GTK_ACTION (current));

  if (!strcmp(path,"CS_Unicode")) cs = VKC_UTF8;
  else if (!strcmp(path,"CS_Tcvn3")) cs = VKC_TCVN;
  else if (!strcmp(path,"CS_Vni")) cs = VKC_VNI;
  else if (!strcmp(path,"CS_Viqr")) cs = VKC_VIQR;
  else if (!strcmp(path,"CS_Viscii")) cs = VKC_VISCII;   
  else if (!strcmp(path,"CS_Vps")) cs = VKC_VPS;

  vnkb_set_charset(vnkb,cs);
}

static void
vnkb_docklet_about_cb(GtkAction *action,Vnkb *vnkb)
{
  g_message ("Action \"%s\" activated", gtk_action_get_name (action));
}

static void
vnkb_docklet_exit_cb(GtkAction *action,Vnkb *vnkb)
{
  gtk_exit(0);
}

static void
vnkb_docklet_pref_cb(GtkAction *action,Vnkb *vnkb)
{
  vnkb_show_preferences(vnkb);
}

static void
vnkb_docklet_enabled_cb(GtkToggleAction *action,Vnkb *vnkb)
{
  vnkb_set_enabled(vnkb,gtk_toggle_action_get_active(action));
}

static void
vnkb_docklet_spelling_cb(GtkToggleAction *action,Vnkb *vnkb)
{
  vnkb_set_spelling(vnkb,gtk_toggle_action_get_active(action));
}

static void
button_popup_cb(GtkWidget *w,gpointer data,Vnkb *vnkb)
{
  VnkbDocklet *docklet = (VnkbDocklet*)vnkb->panel;
  gtk_menu_popup(GTK_MENU(docklet->menu),NULL,NULL,NULL,NULL,0,gtk_get_current_event_time());
}

static gboolean 
button_press_hack (GtkWidget *w, GdkEventButton *event, Vnkb *vnkb)
{
  VnkbDocklet *docklet = (VnkbDocklet*)vnkb->panel;
  
  if (event->button == 3) {
    gtk_menu_popup(GTK_MENU(docklet->menu),NULL,NULL,NULL,NULL,0,gtk_get_current_event_time());
    
    return TRUE;
  }
    
  return FALSE;
    
}

static void
gtk_widget_set_visible(GtkWidget *w,gboolean v)
{
  if (v)
    gtk_widget_show(w);
  else
    gtk_widget_hide(w);
}

static void
vnkb_docklet_driver_changed(Vnkb *vnkb)
{
  GtkWidget *w;
  VnkbDocklet *docklet = (VnkbDocklet*)vnkb->panel;

  gboolean is_xvnkb = vnkb->driver == DRIVER_XVNKB;

  w = gtk_ui_manager_get_widget(docklet->uim,"/MainMenu/IM/IM_StarViqr");
  gtk_widget_set_visible(w,!is_xvnkb);
  w = gtk_ui_manager_get_widget(docklet->uim,"/MainMenu/CS/CS_Vps");
  gtk_widget_set_visible(w,is_xvnkb);
  w = gtk_ui_manager_get_widget(docklet->uim,"/MainMenu/CS/CS_Viscii");
  gtk_widget_set_visible(w,is_xvnkb);
  w = gtk_ui_manager_get_widget(docklet->uim,"/MainMenu/Spell");
  gtk_widget_set_visible(w,is_xvnkb);
}

#define _(X) X

static GtkActionEntry entries[] = {
  {"IM",NULL,_("_Input Method")},
  {"CS",NULL,_("_Charset")},
  {"About",NULL,_("_About..."),NULL,NULL,G_CALLBACK(vnkb_docklet_about_cb)},
  {"Props",GTK_STOCK_PREFERENCES,NULL,NULL,NULL,G_CALLBACK(vnkb_docklet_pref_cb)},
  {"Exit",GTK_STOCK_QUIT,NULL,NULL,NULL,G_CALLBACK(vnkb_docklet_exit_cb)},
};

static GtkRadioActionEntry im_entries[] = {
  {"IM_Off",NULL,_("_Off"),NULL,NULL,0},
  {"IM_Vni",NULL,_("_Vni"),NULL,NULL,1},
  {"IM_Telex",NULL,_("_Telex"),NULL,NULL,2},
  {"IM_Viqr",NULL,_("V_iqr"),NULL,NULL,3},
  {"IM_StarViqr",NULL,_("Vi_qr*"),NULL,NULL,4},
};

static GtkRadioActionEntry cs_entries[] = {
  {"CS_Unicode",NULL,_("_Unicode"),NULL,NULL,0},
  {"CS_Vni",NULL,_("_Vni"),NULL,NULL,1},
  {"CS_Viscii",NULL,_("Vi_scii"),NULL,NULL,2},
  {"CS_Viqr",NULL,_("V_iqr"),NULL,NULL,3},
  {"CS_Vps",NULL,_("V_ps"),NULL,NULL,4},
  {"CS_Tcvn3",NULL,_("_Tcvn3"),NULL,NULL,5},
};

static GtkToggleActionEntry toggle_entries[] = {
  {"Enable",NULL,_("_Enable"),NULL,NULL,G_CALLBACK(vnkb_docklet_enabled_cb),0},
  {"Spell",GTK_STOCK_SPELL_CHECK,NULL,NULL,NULL,G_CALLBACK(vnkb_docklet_spelling_cb),0},
};

static char *xml = 
"<ui>"
"  <popup name=\"MainMenu\">"
"    <menu action=\"IM\">"
"      <menuitem action=\"IM_Off\" />"
"      <menuitem action=\"IM_Vni\" />"
"      <menuitem action=\"IM_Telex\" />"
"      <menuitem action=\"IM_Viqr\" />"
"      <menuitem action=\"IM_StarViqr\" />"
"    </menu>"
"    <menu action=\"CS\">"
"      <menuitem action=\"CS_Unicode\" />"
"      <menuitem action=\"CS_Tcvn3\" />"
"      <menuitem action=\"CS_Vni\" />"
"      <menuitem action=\"CS_Viqr\" />"
"      <menuitem action=\"CS_Viscii\" />"
"      <menuitem action=\"CS_Vps\" />"
"    </menu>"
""
"    <menuitem action=\"Enable\" />"
"    <menuitem action=\"Spell\" />"
"    <menuitem action=\"Props\" />"
/*"    <menuitem action=\"About\" />"*/
"    <menuitem action=\"Exit\" />"
"  </popup>"
"</ui>";

int main(int argc,char **argv)
{
  Vnkb *vnkb;

  VnkbDocklet *docklet = g_new0(VnkbDocklet,1);
  vnkb = g_new0(Vnkb,1);

  vnkb->panel = docklet;

  gtk_init(&argc,&argv);

  vnkb->update_charset = vnkb_docklet_update_charset;
  vnkb->update_method = vnkb_docklet_update_method;
  vnkb->update_enabled = vnkb_docklet_update_enabled;
  vnkb->update_spelling = vnkb_docklet_update_spelling;
  vnkb->driver_changed = vnkb_docklet_driver_changed;

  docklet->docklet = egg_tray_icon_new("Gaim");

  docklet->actions = gtk_action_group_new("MenuActions");
  gtk_action_group_set_translation_domain(docklet->actions,GETTEXT_PACKAGE);
  gtk_action_group_add_actions(docklet->actions,entries,G_N_ELEMENTS(entries),vnkb);
  gtk_action_group_add_radio_actions(docklet->actions,im_entries,G_N_ELEMENTS(im_entries),0,G_CALLBACK(im_change_cb),vnkb);
  gtk_action_group_add_radio_actions(docklet->actions,cs_entries,G_N_ELEMENTS(cs_entries),0,G_CALLBACK(cs_change_cb),vnkb);
  gtk_action_group_add_toggle_actions(docklet->actions,toggle_entries,G_N_ELEMENTS(toggle_entries),vnkb);

  docklet->uim = gtk_ui_manager_new();
  gtk_ui_manager_insert_action_group(docklet->uim,docklet->actions,0);
  gtk_ui_manager_add_ui_from_string(docklet->uim,xml,-1,NULL);
  docklet->menu = gtk_ui_manager_get_widget(docklet->uim,"/MainMenu");

  vnkb_setup_widget(vnkb,GTK_WIDGET(docklet->docklet));
  vnkb_get_sync_atoms(vnkb,TRUE);
  vnkb_init_charset(vnkb);
  vnkb_init_method(vnkb);
  vnkb_init_enabled(vnkb);
  vnkb_init_spelling(vnkb);
  vnkb_set_event_filter(vnkb,TRUE);

  vnkb->initialized = TRUE;	/* done */

  g_signal_connect(G_OBJECT(vnkb->button),
		   "button-press-event",
		   G_CALLBACK(button_press_hack),
		   vnkb);
  //signal_connect(G_OBJECT(vnkb->button),"popup-menu",G_CALLBACK(button_popup_cb),NULL);
  //g_signal_connect(G_OBJECT(docklet), "embedded", G_CALLBACK(docklet_x11_embedded_cb), NULL);
  //g_signal_connect(G_OBJECT(docklet), "destroy", G_CALLBACK(docklet_x11_destroyed_cb), NULL);
  //g_signal_connect(G_OBJECT(box), "button-press-event", G_CALLBACK(docklet_x11_clicked_cb), NULL);
  
  /* ref the docklet before we bandy it about the place */
  g_object_ref(G_OBJECT(docklet->docklet));

  gtk_main();
  return 0;
}

static void
vnkb_docklet_update_charset(Vnkb *applet)
{
  VnkbDocklet *docklet = (VnkbDocklet*)applet->panel;
  char *cmd;
  switch (applet->charset) {
  case VKC_UTF8: cmd = "/MainMenu/CS/CS_Unicode"; break;
  case VKC_TCVN: cmd = "/MainMenu/CS/CS_Tcvn3"; break;
  case VKC_VNI: cmd = "/MainMenu/CS/CS_Vni"; break;
  case VKC_VIQR: cmd ="/MainMenu/CS/CS_Viqr"; break;
  case VKC_VISCII: cmd="/MainMenu/CS/CS_Viscii"; break;
  case VKC_VPS: cmd="/MainMenu/CS/CS_Vps"; break;
  default: cmd = NULL;
  }

  if (cmd) {
    GtkWidget *w = gtk_ui_manager_get_widget(docklet->uim,cmd);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),TRUE);
  }
}

static void
vnkb_docklet_update_method(Vnkb *applet) 
{
  VnkbDocklet *docklet = (VnkbDocklet*)applet->panel;
  char *cmd;
  switch (applet->method) {
  case VKM_OFF: cmd = "/MainMenu/IM/IM_Off"; break;
  case VKM_VIQR: cmd = "/MainMenu/IM/IM_Viqr"; break;
  case VKM_VIQR_STAR: cmd = "/MainMenu/IM/IM_StarViqr"; break;
  case VKM_VNI: cmd = "/MainMenu/IM/IM_Vni"; break;
  case VKM_TELEX: cmd = "/MainMenu/IM/IM_Telex"; break;
  default:cmd = NULL;
  }

  if (cmd) {
    GtkWidget *w = gtk_ui_manager_get_widget(docklet->uim,cmd);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),TRUE);
  }
}

static void
vnkb_docklet_update_enabled(Vnkb *applet) 
{
  VnkbDocklet *docklet = (VnkbDocklet*)applet->panel;
  char *cmd = "/MainMenu/Enable";
  GtkWidget *w = gtk_ui_manager_get_widget(docklet->uim,cmd);
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),applet->enabled);
}

static void
vnkb_docklet_update_spelling(Vnkb *applet) 
{
  VnkbDocklet *docklet = (VnkbDocklet*)applet->panel;
  char *cmd = "/MainMenu/Spell";
  GtkWidget *w = gtk_ui_manager_get_widget(docklet->uim,cmd);
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),applet->spelling);
}
