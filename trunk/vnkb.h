typedef struct _Vnkb Vnkb;
struct _Vnkb {
  gpointer panel;
  GtkWidget         	*label;
  GtkWidget         	*button;

  gboolean      	 enabled;
  int           	 method;
  int           	 backup_method;
  int           	 charset;
  gboolean 		 initialized;
  gboolean		 filter_set;

  void (*widget_setup)(Vnkb *vnkb,GtkWidget *container);
  void (*clicked_cb)(GtkButton *button,Vnkb *vnkb);
  GdkFilterReturn (*event_filter_cb)(GdkXEvent 	*xevent,
					      GdkEvent 	*event,
					      Vnkb *applet);
  
  void (*update_charset)(Vnkb *applet);
  void (*update_method)(Vnkb *applet);
  void (*update_enabled)(Vnkb *applet);
  void (*set_enabled)(Vnkb *applet,gboolean enable);
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

void vnkb_update_charset(Vnkb *applet);
void vnkb_update_method(Vnkb *applet);
void vnkb_update_enabled(Vnkb *applet);

void vnkb_set_enabled(Vnkb *applet,gboolean enable);
void vnkb_set_method(Vnkb *vnkb,int im);
void vnkb_set_charset(Vnkb *vnkb,int cs);


extern Atom AIMCharset, AIMUsing, AIMMethod, AIMViqrStarGui, AIMViqrStarCore;
extern Atom ASuspend;
