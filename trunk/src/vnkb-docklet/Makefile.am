INCLUDES = $(VNKB_DOCKLET_CFLAGS) -DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" -I../libvnkb

bin_PROGRAMS = vnkb-docklet

vnkb_docklet_SOURCES = vnkb-docklet.c eggtrayicon.c eggtrayicon.h
vnkb_docklet_LDADD = $(VNKB_DOCKLET_LIBS) ../libvnkb/libvnkb.a

