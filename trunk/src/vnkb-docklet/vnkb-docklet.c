#include <gtk/gtk.h>
#include "eggtrayicon.h"
#include "vnkb.h"
#include "xvnkb.h"
#include "keycons.h"


static void vnkb_docklet_update_charset(Vnkb *applet);
static void vnkb_docklet_update_method(Vnkb *applet);
static void vnkb_docklet_update_enabled(Vnkb *applet);

static EggTrayIcon *docklet;
static Vnkb *vnkb;
static GtkWidget *menu;
static GtkUIManager *uim;
static GtkActionGroup *actions;

static void
im_change_cb(GtkAction *action, GtkRadioAction *current)
{
  int im = VKM_OFF;
  const char *path = gtk_action_get_name (GTK_ACTION (current));

  if (!strcmp(path,"IM_Off")) im = VKM_OFF;
  else if (!strcmp(path,"IM_Vni")) im = VKM_VNI;
  else if (!strcmp(path,"IM_Telex")) im = VKM_TELEX;
  else if (!strcmp(path,"IM_Viqr")) im = VKM_VIQR;

  vnkb_set_method(vnkb,im);
}

static void
cs_change_cb(GtkAction *action, GtkRadioAction *current)
{
  int cs = UNICODE_CHARSET;
  const char *path = gtk_action_get_name (GTK_ACTION (current));

  if (!strcmp(path,"CS_Unicode")) cs = UNICODE_CHARSET;
  else if (!strcmp(path,"CS_Tcvn3")) cs = TCVN3_CHARSET;
  else if (!strcmp(path,"CS_Vni")) cs = VNI_CHARSET;
  else if (!strcmp(path,"CS_Viqr")) cs = VIQR_CHARSET;
  //else if (!strcmp(path,"CS_Viscii")) cs = CS_VISCII;   
  //else if (!strcmp(path,"CS_Vps")) cs = CS_VPS;

  vnkb_set_charset(vnkb,cs);
}

static void
vnkb_docklet_about_cb(GtkAction *action)
{
  g_message ("Action \"%s\" activated", gtk_action_get_name (action));
}

static void
vnkb_docklet_exit_cb(GtkAction *action)
{
  gtk_exit(0);
}

static void
vnkb_docklet_pref_cb(GtkAction *action)
{
  vnkb_show_preferences(vnkb);
}

static void
vnkb_docklet_enabled_cb(GtkToggleAction *action)
{
  vnkb_set_enabled(vnkb,gtk_toggle_action_get_active(action));
}

static void
button_popup_cb(GtkWidget *w,gpointer data)
{
  gtk_menu_popup(GTK_MENU(menu),NULL,NULL,NULL,NULL,0,gtk_get_current_event_time());
}

static gboolean 
button_press_hack (GtkWidget *w, GdkEventButton *event, gpointer data)
{
    GtkWidget *applet = GTK_WIDGET (data);
    
    if (event->button == 3) {
  gtk_menu_popup(GTK_MENU(menu),NULL,NULL,NULL,NULL,0,gtk_get_current_event_time());
      
	return TRUE;
    }
    
    return FALSE;
    
}

#define _(X) X

static GtkActionEntry entries[] = {
  {"IM",NULL,_("_Input Method")},
  {"CS",NULL,_("_Charset")},
  {"About",NULL,_("_About..."),NULL,NULL,G_CALLBACK(vnkb_docklet_about_cb)},
  {"Props",NULL,_("_Preferences..."),NULL,NULL,G_CALLBACK(vnkb_docklet_pref_cb)},
  {"Exit",NULL,_("E_xit"),NULL,NULL,G_CALLBACK(vnkb_docklet_exit_cb)},
};

static GtkRadioActionEntry im_entries[] = {
  {"IM_Off",NULL,_("_Off"),NULL,NULL,0},
  {"IM_Vni",NULL,_("_Vni"),NULL,NULL,1},
  {"IM_Telex",NULL,_("_Telex"),NULL,NULL,2},
  {"IM_Viqr",NULL,_("V_iqr"),NULL,NULL,3},
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
};

static char *xml = 
"<ui>"
"  <popup name=\"MainMenu\">"
"    <menu action=\"IM\">"
"      <menuitem action=\"IM_Off\" />"
"      <menuitem action=\"IM_Vni\" />"
"      <menuitem action=\"IM_Telex\" />"
"      <menuitem action=\"IM_Viqr\" />"
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
"    <menuitem action=\"Props\" />"
"    <menuitem action=\"About\" />"
"    <menuitem action=\"Exit\" />"
"  </popup>"
"</ui>";

int main(int argc,char **argv)
{

  vnkb = g_new0(Vnkb,1);

  gtk_init(&argc,&argv);

  vnkb->update_charset = vnkb_docklet_update_charset;
  vnkb->update_method = vnkb_docklet_update_method;
  vnkb->update_enabled = vnkb_docklet_update_enabled;

  docklet = egg_tray_icon_new("Gaim");

  actions = gtk_action_group_new("MenuActions");
  gtk_action_group_set_translation_domain(actions,GETTEXT_PACKAGE);
  gtk_action_group_add_actions(actions,entries,G_N_ELEMENTS(entries),docklet);
  gtk_action_group_add_radio_actions(actions,im_entries,G_N_ELEMENTS(im_entries),0,G_CALLBACK(im_change_cb),docklet);
  gtk_action_group_add_radio_actions(actions,cs_entries,G_N_ELEMENTS(cs_entries),0,G_CALLBACK(cs_change_cb),docklet);
  gtk_action_group_add_toggle_actions(actions,toggle_entries,G_N_ELEMENTS(toggle_entries),docklet);

  uim = gtk_ui_manager_new();
  gtk_ui_manager_insert_action_group(uim,actions,0);
  gtk_ui_manager_add_ui_from_string(uim,xml,-1,NULL);
  menu = gtk_ui_manager_get_widget(uim,"/MainMenu");

  vnkb_setup_widget(vnkb,GTK_WIDGET(docklet));
  vnkb_get_sync_atoms(TRUE);
  vnkb_init_charset(vnkb);
  vnkb_init_method(vnkb);
  vnkb_init_enabled(vnkb);
  vnkb_set_event_filter(vnkb,TRUE);

  vnkb->initialized = TRUE;	/* done */

  g_signal_connect(G_OBJECT(vnkb->button),
		   "button-press-event",
		   G_CALLBACK(button_press_hack),
		   vnkb->panel);
  //signal_connect(G_OBJECT(vnkb->button),"popup-menu",G_CALLBACK(button_popup_cb),NULL);
  //g_signal_connect(G_OBJECT(docklet), "embedded", G_CALLBACK(docklet_x11_embedded_cb), NULL);
  //g_signal_connect(G_OBJECT(docklet), "destroy", G_CALLBACK(docklet_x11_destroyed_cb), NULL);
  //g_signal_connect(G_OBJECT(box), "button-press-event", G_CALLBACK(docklet_x11_clicked_cb), NULL);
  
  /* ref the docklet before we bandy it about the place */
  g_object_ref(G_OBJECT(docklet));

  gtk_main();
  return 0;
}

static void
vnkb_docklet_update_charset(Vnkb *applet)
{
  char *cmd;
  switch (vnkb->charset) {
  case UNICODE_CHARSET: cmd = "/MainMenu/CS/CS_Unicode"; break;
  case TCVN3_CHARSET: cmd = "/MainMenu/CS/CS_Tcvn3"; break;
  case VNI_CHARSET: cmd = "/MainMenu/CS/CS_Vni"; break;
  case VIQR_CHARSET: cmd ="/MainMenu/CS/CS_Viqr"; break;
  //case VISCII_CHARSET: cmd="/MainMenu/CS/CS_Viscii"; break;
  //case VPS_CHARSET: cmd="/MainMenu/CS/CS_Vps"; break;
  default: cmd = NULL;
  }

  if (cmd) {
    GtkWidget *w = gtk_ui_manager_get_widget(uim,cmd);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),TRUE);
  }
}

static void
vnkb_docklet_update_method(Vnkb *applet) 
{
  char *cmd;
  switch (vnkb->method) {
  case VKM_OFF: cmd = "/MainMenu/IM/IM_Off"; break;
  case VKM_VIQR: cmd = "/MainMenu/IM/IM_Viqr"; break;
  case VKM_VNI: cmd = "/MainMenu/IM/IM_Vni"; break;
  case VKM_TELEX: cmd = "/MainMenu/IM/IM_Telex"; break;
  default:cmd = NULL;
  }

  if (cmd) {
    GtkWidget *w = gtk_ui_manager_get_widget(uim,cmd);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),TRUE);
  }
}

static void
vnkb_docklet_update_enabled(Vnkb *applet) 
{
  char *cmd = "/MainMenu/Enable";
  GtkWidget *w = gtk_ui_manager_get_widget(uim,cmd);
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),applet->enabled);
  vnkb_update_label(applet);
}
