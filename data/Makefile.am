## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

SUBDIRS = pixmaps

gamp.glade: gamp.glade.in
	 sed -e 's,\@PACKAGE_PIXMAPS_DIR\@,@PACKAGE_PIXMAPS_DIR@,g' gamp.glade.in > gamp.glade

gamp_datadir = $(prefix)/@NO_PREFIX_PACKAGE_DATA_DIR@

gamp_data_DATA = \
	open-location.glade \
	gamp.glade

EXTRA_DIST = $(gamp_data_DATA) gamp.glade.in

