#define VNKB_LABEL_DEFAULT 0
#define VNKB_LABEL_CUSTOM  1
#define VNKB_LABEL_IM      2

typedef struct _Vnkb Vnkb;
struct _Vnkb {
  gpointer panel;
  GtkWidget         	*label;
  GtkWidget         	*button;
  GtkWidget *widget_text_enabled,*widget_text_disabled;
  GtkListStore *store;		/* for preferences dialog (shortcut) */

  gboolean      	 enabled;
  gboolean      	 spelling;
  int           	 method;
  int           	 backup_method;
  int           	 charset;
  gboolean 		 initialized;
  gboolean		 filter_set;
  int label_mode;

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
};

void vnkb_setup_widget (Vnkb *fish,GtkWidget *container);
void vnkb_show_preferences (Vnkb *mc);
void vnkb_show_about (Vnkb *mc);
void vnkb_get_sync_atoms(int xvnkbSync);
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

extern Atom AIMCharset, AIMUsing, AIMMethod, AIMViqrStarGui, AIMViqrStarCore;
extern Atom ASuspend;
