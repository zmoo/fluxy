AC_INIT

AC_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(fluxy,0.21)

AC_PROG_CXX
AC_PROG_INSTALL

PKG_CHECK_MODULES([GLIB], [glib-2.0])

AC_CHECK_LIB(event, event_init)
AC_CHECK_FUNC(event_loopbreak, AC_DEFINE(HAVE_LIBEVENT_LOOPBREAK, 1, [Wheter You have a recent libevent version]))

AC_OUTPUT(Makefile src/Makefile doc/Makefile munin/Makefile)
