noinst_LIBRARIES = libvnkb.a

INCLUDES = $(VNKB_CFLAGS)\
	 -DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DVNKB_GLADEDIR=\""$(gladedir)"\"
libvnkb_a_SOURCES =  xvnkb.h xunikey.h\
	uksync.c uksync.h\
	keycons.h\
	vnkb.c vnkb.h\
	pref.c config.c\
	eggcellrendererkeys.c eggcellrendererkeys.h\
	eggaccelerators.h eggaccelerators.c

EXTRA_DIST = $(glade_DATA)

#libvnkb_a_DATA = vnkb-preferences.glade
gladedir   = $(datadir)/vnkb-applet
glade_DATA = vnkb-preferences.glade
