libexec_PROGRAMS = vnkb-applet

INCLUDES = $(VNKB_CFLAGS) -DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"  -I../libvnkb

vnkb_applet_SOURCES = vnkb-applet.c 
vnkb_applet_LDADD = $(VNKB_LIBS) ../libvnkb/libvnkb.a

EXTRA_DIST = VnkbApplet_Factory.server.in.in	\
	intltool-extract.in			\
	intltool-merge.in			\
	intltool-update.in			\
	$(ui_DATA)

serverdir       = $(libdir)/bonobo/servers
server_in_files = VnkbApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files).in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_DATA) $(server_DATA).in

@INTLTOOL_SERVER_RULE@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = VnkbApplet.xml

