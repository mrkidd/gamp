## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = \
	$(GNOME_CFLAGS)\
	 -I/home/mrkidd/garnome/include

gamp_CFLAGS =\
	 -Wall\
	 -O1\
	 -g

bin_PROGRAMS = gamp

gamp_SOURCES = \
	main.c\
	gui-callbacks.c\
	GAPPlayer.c\
	GAPPlayer.h\
	main.h\
	playlist.c\
	playlist.h\
	playlist-gui.c

gamp_LDFLAGS = \
	 -L/home/mrkidd/garnome/lib

gamp_LDADD =  \
	$(GNOME_LIBS)

