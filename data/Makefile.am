desktopdir = $(datadir)/applications
desktop_in_files = gamp.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

icondir = $(datadir)/pixmaps
icon_DATA = gamp.png

applicationsdir = $(datadir)/application-registry
applications_DATA = gamp.applications

keysdir = $(datadir)/mime-info
keys_DATA = gamp.keys

gladedir = $(pkgdatadir)
glade_DATA = gamp.glade open-location.glade
 
EXTRA_DIST = \
	$(desktop_in_files)		\
	$(desktop_DATA)			\
	$(applications_DATA)	\
	$(keys_DATA)			\
	$(glade_DATA)


CLEANFILES = \
	$(desktop_DATA)			\
	*.bak					\
	*.gladep
	
	

