AC_PREREQ([2.53])
 
AC_INIT([gamp], [0.4])
AC_CONFIG_SRCDIR(src/main.c)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)

dnl ** Requirements **
GNOME_VFS_REQUIRED=2.5.91
LIBGNOMEUI_REQUIRED=2.5.92
LIBGLADE_REQUIRED=2.3.6
GCONF_REQUIRED=2.5.90
GTK_REQUIRED=2.4.0

dnl ** Translations **
ALL_LINGUAS=""
GETTEXT_PACKAGE=gamp
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Define to the Gettext package name])
                                                                                                    
AM_GLIB_GNU_GETTEXT
AC_PROG_INTLTOOL

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

GNOME_MODULES="gtk+-2.0 >= $GTK_REQUIRED gnome-vfs-2.0 >= $GNOME_VFS_REQUIRED libgnomeui-2.0 >= $LIBGNOMEUI_REQUIRED libglade-2.0 >= $LIBGLADE_REQUIRED gconf-2.0 >= $GCONF_REQUIRED"
PKG_CHECK_MODULES(GNOME, $GNOME_MODULES)
AC_SUBST(GNOME_CFLAGS)
AC_SUBST(GNOME_LIBS)

GST_REQ=0.7.2
dnl start with 0.8
GST_MAJORMINOR=0.8
PKG_CHECK_MODULES(GST, \
  gstreamer-$GST_MAJORMINOR >= $GST_REQ \
  gstreamer-libs-$GST_MAJORMINOR >= $GST_REQ \
  gstreamer-gconf-$GST_MAJORMINOR >= $GST_REQ,
  HAVE_GST=yes,HAVE_GST=no)
 
dnl fall back to 0.7
if test "x$HAVE_GST" = "xno"; then
  GST_MAJORMINOR=0.7
  PKG_CHECK_MODULES(GST, \
    gstreamer-$GST_MAJORMINOR >= $GST_REQ \
    gstreamer-libs-$GST_MAJORMINOR >= $GST_REQ \
    gstreamer-gconf-$GST_MAJORMINOR >= $GST_REQ,
    HAVE_GST=yes,HAVE_GST=no)
fi
if test "x$HAVE_GST" = "xno"; then
	AC_MSG_ERROR(You need GStreamer installed)
fi
AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_LIBS)


AC_PATH_PROG(GCONFTOOL, gconftool-2)
AM_GCONF_SOURCE_2

dnl ================= popt  ===============================
                                                                                                    
AC_CHECK_LIB(popt, poptGetArgs, [POPT_LIBS="-lpopt"],
AC_MSG_ERROR([popt is required to build gamp.
You can download the latest version from ftp://ftp.rpm.org/pub/rpm/dist/rpm-4.0.x/]))
AC_SUBST(POPT_LIBS)

AC_OUTPUT([
Makefile
src/Makefile
pixmaps/Makefile
po/Makefile.in
])


echo "
	GStreamer version:	$GST_MAJORMINOR
"
