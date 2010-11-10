AC_INIT([Micromoko], [0.1], [daniele.athome@gmail.com], [micromoko])
AC_CONFIG_SRCDIR(Makefile.am)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE

AC_ISC_POSIX
AC_GNU_SOURCE
AC_PROG_CC
AC_PROG_LIBTOOL

AM_GNU_GETTEXT(external)
AM_GNU_GETTEXT_VERSION([0.12.1])

dnl Dependencies
ELEMENTARY_REQUIRED=0.0.0
GLIB_REQUIRED=2.18.0
MOKOSUITE_REQUIRED=1.0.99
REST_REQUIRED=0.6

PKG_CHECK_MODULES(GLIB,
  glib-2.0 >= $GLIB_REQUIRED
  gobject-2.0 >= $GLIB_REQUIRED
)

PKG_CHECK_MODULES(ELEMENTARY,
  elementary >= $ELEMENTARY_REQUIRED
)

PKG_CHECK_MODULES(MOKOSUITE,
  mokosuite-utils >= $MOKOSUITE_REQUIRED
  mokosuite-ui >= $MOKOSUITE_REQUIRED
)

PKG_CHECK_MODULES(REST,
  rest-0.6 >= $REST_REQUIRED
)

AC_ARG_ENABLE(debug,
    AC_HELP_STRING([--enable-debug], [Enable debug build (default: disabled)]),
    enable_debug=$enableval,
    enable_debug=no)

if test "x$enable_debug" != "xno"; then
    AC_DEFINE(DEBUG, 1, [Define to enable debug build])
fi

AC_SUBST(VERSION)

AH_TEMPLATE([USE_NSS], [Define to use NSS instead of OpenSSL])

AC_ARG_ENABLE(nss, AC_HELP_STRING([--enable-nss],[use NSS instead of OpenSSL]))

dnl ** crypto/hash lib (OpenSSL or NSS)
AS_IF([test "${enable_nss}" == "yes"], [
  PKG_CHECK_MODULES(NSS, nss, [ AC_DEFINE(USE_NSS, 1) USE_NSS=1 PC_REQ="$PC_REQ nss" ])
  HASH_LIBS=${NSS_LIBS}
  HASH_CFLAGS=${NSS_CFLAGS}
  report_hash="NSS"
], [
  AC_CHECK_HEADERS(openssl/hmac.h)
  if test -z "${HASH_LIBS}"; then
  HASH_LIBS="-lcrypto"
  fi
  if test -z "${HASH_CFLAGS}"; then
  HASH_CFLAGS=""
  fi
  report_hash="OpenSSL"
  PC_LIB="$PC_LIB -lcrypto"
  PC_REQ="$PC_REQ libcrypto"
  AC_MSG_NOTICE([

  NOTE: OpenSSL is not compatible with GPL applications.
  Even if only linked with GPL code you are not allowed to distibute your app.
  However liboauth provides an exeption (to the GPL) to circumvent this issue
  (see README, src/hash.c). Nevertheless, double-check your licensing.

  liboauth itself is licensed under MIT license and comatible with the GPL.

  Either way, you are probably better off using NSS (configure --enable-nss);
  future versions of liboauth will default to the Mozilla NSS.

  see http://people.gnome.org/~markmc/openssl-and-the-gpl.html
  ])
])

AC_SUBST(HASH_LIBS)
AC_SUBST(HASH_CFLAGS)


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
