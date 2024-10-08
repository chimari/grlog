dnl Process this file with autoconf to produce a configure script.
AC_INIT(main.c)
PACKAGE=grlog

dnl version number
MAJOR_VERSION=1
MINOR_VERSION=5
MICRO_VERSION=0
EXTRA_VERSION=-20240926
VERSION=$MAJOR_VERSION.$MINOR_VERSION.$MICRO_VERSION$EXTRA_VERSION

dnl set $target
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)
dnl AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
dnl AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)


AC_CONFIG_HEADERS([config.h libssh2_config.h])
dnl AC_ARG_PROGRAM
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

dnl if test x"${use_gtk1}" = "xyes"; then
dnl AM_PATH_GLIB(1.2.6,,
dnl        AC_MSG_ERROR(Test for GLIB failed. See the file 'INSTALL' for help.),
dnl         gthread)
dnl AM_PATH_GTK(1.2.0, ,
dnl             AC_MSG_ERROR(Cannot find GTK: Is gtk-config in path?))
dnl else
AM_PATH_GLIB_2_0(2.32.0,, AC_MSG_ERROR([glib-2.x is required]), gthread gobject)
dnl fi

dnl X11
AC_SEARCH_LIBS([XFree], [X11], [], [
  LDFLAGS="$LDFLAGS -L/usr/X11R6/lib -L/usr/X11R6/lib64"
  AC_MSG_CHECKING(["Looking for XFree in /usr/X11R6"])
  AC_SEARCH_LIBS([XFree], [X11], [], [
    AC_MSG_FAILURE(["Cannot find X11. Try setting LDFLAGS -L"], [-1])
  ])
])

dnl GTK 3 or 2 
AC_ARG_WITH(gtk3,
             AC_HELP_STRING([--with-gtk3],
                        [use Gtk+3 instead of Gtk+2]),
                        [use_gtk3=yes])
if test x"${use_gtk3}" = "xyes"; then
GTK3_CFLAGS="`pkg-config gtk+-3.0 --cflags`"
GTK3_LIBS="`pkg-config gtk+-3.0 --libs`"
dnl    AM_PATH_GTK_3_0(3.22.0,, AC_MSG_ERROR([gtk+-2.x is required]), gthread gobject)
AC_DEFINE([USE_GTK3],[],[Use Gtk+3.x instead of Gtk+2])
else
AM_PATH_GTK_2_0(2.24.0,, AC_MSG_ERROR([gtk+-2.x is required]), gthread gobject)
AC_DEFINE([USE_GTK2],[],[Use Gtk+2.x instead of Gtk+3])
GTK3_CFLAGS=""
GTK3_LIBS=""
fi
AC_SUBST(GTK3_LIBS)
AC_SUBST(GTK3_CFLAGS)


dnl Check for OpenSSL
	PKG_CHECK_MODULES(OPENSSL, openssl, [:], [:])
	if test "x$OPENSSL_LIBS" != "x"; then
		CFLAGS="$CFLAGS $OPENSSL_CFLAGS"
		LIBS="$LIBS $OPENSSL_LIBS"
		AC_MSG_CHECKING([if openssl is available])
		AC_MSG_RESULT(yes)
		AC_DEFINE(USE_SSL, 1, Define if you use OpenSSL to support SSL.)
	else
                AC_MSG_ERROR([OpenSSL is required.])
	fi

dnl Check for libssh2
	PKG_CHECK_MODULES(LIBSSH2, libssh2, [:], [:])
	if test "x$LIBSSH2_LIBS" != "x"; then
		CFLAGS="$CFLAGS $LIBSSH2_CFLAGS"
		LIBS="$LIBS $LIBSSH2_LIBS"
		AC_MSG_CHECKING([if libssh2 is available])
		AC_MSG_RESULT(yes)
	else
                AC_MSG_ERROR([LibSSH2 is required.])
	fi

dnl cfitsio
AC_ARG_WITH(cfitsio,
		 AC_HELP_STRING([--without-cfitsio],
		            [make without plot function]),
			    [without_cfitsio=yes])

AC_ARG_WITH(cfitsio-libraries,[  --with-cfitsio-libraries=DIR   cfitsio lib. dir. (def. /opt/share/cfitsio)], cfitsio_libraries="$withval", cfitsio_libraries="")
AC_ARG_WITH(cfitsio-includes,[  --with-cfitsio-includes=DIR   cfitsio include dir. (def. /opt/share/cfitsio)], cfitsio_includes="$withval", cfitsio_includes="")

if test x"${without_cfitsio}" = "xyes"; then
AC_DEFINE([WITHOUT_CFITSIO],[],[Without CFITSIO function])
CFITSIO_CFLAGS=""
CFITSIO_LIBS=""
else
 	if test "x$cfitsio_libraries" != "x" ; then
  		CFITSIO_LIBS="-L$cfitsio_libraries"
  	else
    		CFITSIO_LIBS="-L/usr/lib64"
  	fi

 	CFITSIO_LIBS="$CFITSIO_LIBS -lcfitsio -lnsl"

  	if test "x$cfitsio_includes" != "x" ; then
  		CFITSIO_CFLAGS="-I$cfitsio_includes"
  	else
    		CFITSIO_CFLAGS="-I/usr/include/cfitsio"
  	fi
fi
AC_SUBST(CFITSIO_LIBS)
AC_SUBST(CFITSIO_CFLAGS)

AC_CHECK_HEADERS([pwd.h sys/utsname.h])

AC_OUTPUT([
Makefile
icons/Makefile
version.h
])


dnl Output the configuration summary
echo ""
echo "####################"
echo " $PACKAGE $VERSION"
echo "####################"
echo ""
echo ""
echo "Configure finished, type 'make' to build."
