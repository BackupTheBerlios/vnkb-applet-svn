SUBDIRS = vnkb-applet vnkb-docklet

noinst_LIBRARIES = libvnkb.a

INCLUDES = $(VNKB_CFLAGS) -DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"  -I..
libvnkb_a_SOURCES =  xvnkb.h uksync.c uksync.h keycons.h vnkb.c vnkb.h

