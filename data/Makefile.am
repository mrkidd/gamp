desktopdir = $(datadir)/applications
desktop_in_files = gamp.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

gladedir = $(pkgdatadir)
glade_DATA = gamp.glade open-location.glade
 
EXTRA_DIST = \
	$(desktop_in_files)		\
	$(desktop_DATA)			\
	$(glade_DATA)


CLEANFILES = \
	gamp.desktop
	

