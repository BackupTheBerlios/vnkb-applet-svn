INCLUDES = $(VNKB_CFLAGS) -DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" -I..

bin_PROGRAMS = vnkb-docklet

vnkb_docklet_SOURCES = vnkb-docklet.c eggtrayicon.c eggtrayicon.h
vnkb_docklet_LDADD = $(VNKB_LIBS) ../libvnkb.a

EXTRA_DIST = vnkb-docklet.xml