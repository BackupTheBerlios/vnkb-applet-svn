SUBDIRS = po
INCLUDES = $(VNKB_CFLAGS) -DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" 
libexec_PROGRAMS = vnkb-applet
vnkb_applet_SOURCES = vnkb-applet.c xvnkb.h uksync.c uksync.h keycons.h

vnkb_applet_LDADD = $(VNKB_LIBS)

EXTRA_DIST = VnkbApplet_Factory.server.in.in	

serverdir       = $(libdir)/bonobo/servers
server_in_files = VnkbApplet_Factory.server.in
server_DATA     = $(server_in_files:.server.in=.server)

$(server_in_files): $(server_in_files).in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

CLEANFILES = $(server_DATA) $(server_DATA).in

@INTLTOOL_SERVER_RULE@

uidir = $(datadir)/gnome-2.0/ui
ui_DATA = VnkbApplet.xml

