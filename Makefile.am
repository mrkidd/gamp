SUBDIRS = src pixmaps po


desktopdir = $(datadir)/applications
desktop_in_files = gamp.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
 
gladedir = $(pkgdatadir)
glade_DATA = gamp.glade open-location.glade
 
EXTRA_DIST = \
	AUTHORS					\
	MAINTAINERS				\
	NEWS					\
	README					\
	intltool-merge.in		\
	intltool-update.in		\
	intltool-extract.in		\
	omf.make				\
	xmldocs.make			\
	$(desktop_in_files)		\
	$(glade_DATA)
	
DISTCLEANFILES = \
	po/.intltool-merge-cache	\
	$(desktop_DATA)				\
	intltool-extract			\
	intltool-merge				\
	intltool-update


