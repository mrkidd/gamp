INCLUDES = \
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	$(GNOME_CFLAGS)						\
	$(GST_CFLAGS)						\
	-DDATADIR=\""$(datadir)"\"

bin_PROGRAMS = gamp

gamp_SOURCES = \
	main.c \
	gui-callbacks.c \
	GAPPlayer.c \
	GAPPlayer.h \
	main.h \
	playlist.c \
	playlist.h \
	gap-metadata.c \
	gap-metadata.h

gamp_LDFLAGS = 

gamp_LDADD =  \
	$(GNOME_LIBS)		\
	$(GST_LIBS)

