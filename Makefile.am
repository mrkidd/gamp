SUBDIRS = src pixmaps po

EXTRA_DIST = \
	$(desktop_in_files)		\
        gamp.glade              \
        open-location.glade     \
        gamp.png

desktopdir = $(datadir)/applications
desktop_in_files = gamp.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
 
gladedir = $(pkgdatadir)
glade_DATA = gamp.glade open-location.glade
 
@INTLTOOL_DESKTOP_RULE@
 
icondir = $(datadir)/pixmaps
icon_DATA = gamp.png
 
install-data-local:
