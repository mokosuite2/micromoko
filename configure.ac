AC_INIT([Micromoko], [0.1], [daniele.athome@gmail.com], [micromoko])
AC_CONFIG_SRCDIR(Makefile.am)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE

AC_ISC_POSIX
AC_GNU_SOURCE
AC_PROG_CC
AC_PROG_LIBTOOL

dnl Intl for gettext
IT_PROG_INTLTOOL([0.35.0])

if test "x$localedir" == x
then
    localedir='${datadir}/locale'
    AC_SUBST(localedir)
    AM_GLIB_DEFINE_LOCALEDIR([localedir])
fi

AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE="micromoko"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, ["$GETTEXT_PACKAGE"], [Name of the gettext message domain])

dnl Dependencies
ELEMENTARY_REQUIRED=0.0.0
EVAS_REQUIRED=0.0.0
GLIB_REQUIRED=2.18.0
MOKOSUITE_REQUIRED=1.0.99
REST_REQUIRED=0.6
SOUP_REQUIRED=0

PKG_CHECK_MODULES(GLIB,
  glib-2.0 >= $GLIB_REQUIRED
  gobject-2.0 >= $GLIB_REQUIRED
)

PKG_CHECK_MODULES(ELEMENTARY,
  elementary >= $ELEMENTARY_REQUIRED
)

PKG_CHECK_MODULES(EVAS,
  evas >= $EVAS_REQUIRED
)

PKG_CHECK_MODULES(MOKOSUITE,
  mokosuite-utils >= $MOKOSUITE_REQUIRED
  mokosuite-ui >= $MOKOSUITE_REQUIRED
)

PKG_CHECK_MODULES(REST,
  rest-0.6 >= $REST_REQUIRED
)

PKG_CHECK_MODULES(SOUP,
  libsoup-2.4 >= $SOUP_REQUIRED
)

AC_ARG_ENABLE(debug,
    AC_HELP_STRING([--enable-debug], [Enable debug build (default: disabled)]),
    enable_debug=$enableval,
    enable_debug=no)

if test "x$enable_debug" != "xno"; then
    AC_DEFINE(DEBUG, 1, [Define to enable debug build])
fi

AC_SUBST(VERSION)

EFL_WITH_BIN([edje], [edje-cc], [edje_cc])


AC_OUTPUT([
Makefile
src/Makefile
src/twitter/Makefile
data/Makefile
data/themes/Makefile
data/themes/gry/Makefile
po/Makefile.in
])
