#define VNKB_LABEL_DEFAULT 0
#define VNKB_LABEL_CUSTOM  1
#define VNKB_LABEL_IM      2

#define DRIVER_XVNKB  0
#define DRIVER_UNIKEY 1

#include <gtk/gtk.h>
#include <libintl.h>

#ifndef _
#define _(x) dgettext (GETTEXT_PACKAGE, x)
#define N_(x) x
#endif

typedef struct _Vnkb Vnkb;
struct _Vnkb {
  gpointer panel;
  GtkWidget         	*label;
  GtkWidget         	*button;
  GtkWidget *widget_text_enabled,*widget_text_disabled;
  GtkListStore *store;		/* for preferences dialog (shortcut) */
  GtkTooltips *tooltip;

  gboolean      enabled;
  gboolean      spelling;
  gboolean 	disable_on_exit;
  int           method;
  int           backup_method;
  int           charset;
  gboolean 	initialized;
  gboolean	filter_set;
  int 		label_mode;
  int 		driver;

  char *text_enabled;
  char *text_disabled;
  char *font_enabled;
  char *font_disabled;
  GdkColor color_enabled;
  GdkColor color_disabled;

  void (*widget_setup)(Vnkb *vnkb,GtkWidget *container);
  void (*clicked_cb)(GtkButton *button,Vnkb *vnkb);
  GdkFilterReturn (*event_filter_cb)(GdkXEvent 	*xevent,
					      GdkEvent 	*event,
					      Vnkb *applet);
  
  void (*update_charset)(Vnkb *applet);
  void (*update_method)(Vnkb *applet);
  void (*update_enabled)(Vnkb *applet);
  void (*update_spelling)(Vnkb *applet);
  void (*set_enabled)(Vnkb *applet,gboolean enable);
  void (*set_spelling)(Vnkb *applet,gboolean enable);

  void (*driver_changed)(Vnkb *applet);
};

void vnkb_init(Vnkb *vnkb,GtkWidget *container);
void vnkb_cleanup(Vnkb *vnkb);

void vnkb_setup_widget (Vnkb *fish,GtkWidget *container);
void vnkb_show_preferences (Vnkb *mc);
void vnkb_show_about (Vnkb *mc);
void vnkb_get_sync_atoms(Vnkb *,int xvnkbSync);
void vnkb_set_event_filter(Vnkb *applet,int enable);
void vnkb_toggle_enabled(Vnkb *vnkb);


void vnkb_clicked_cb(GtkButton *button,Vnkb *vnkb);
GdkFilterReturn vnkb_event_filter_cb(GdkXEvent 	*xevent,
				     GdkEvent 	*event,
				     Vnkb *applet);
  

void vnkb_init_charset(Vnkb *applet);
void vnkb_init_method(Vnkb *applet);
void vnkb_init_enabled(Vnkb *applet);
void vnkb_init_spelling(Vnkb *applet);

void vnkb_update_charset(Vnkb *applet);
void vnkb_update_method(Vnkb *applet);
void vnkb_update_enabled(Vnkb *applet);
void vnkb_update_spelling(Vnkb *applet);
void vnkb_update_label(Vnkb *vnkb);

void vnkb_set_enabled(Vnkb *applet,gboolean enable);
void vnkb_set_spelling(Vnkb *applet,gboolean enable);
void vnkb_set_method(Vnkb *vnkb,int im);
void vnkb_set_charset(Vnkb *vnkb,int cs);

void vnkb_set_label_mode(Vnkb *vnkb,int mode);
void vnkb_set_driver(Vnkb *vnkb,int driver);

const char* charset_to_text(int cs);
const char* method_to_text(int im);

