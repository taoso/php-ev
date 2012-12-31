dnl +----------------------------------------------------------------------+
dnl | PHP Version 5                                                        |
dnl +----------------------------------------------------------------------+
dnl | Copyrght (C) 1997-2013 The PHP Group                                 |
dnl +----------------------------------------------------------------------+
dnl | This source file is subject to version 3.01 of the PHP license,      |
dnl | that is bundled with this package in the file LICENSE, and is        |
dnl | available through the world-wide-web at the following url:           |
dnl | http://www.php.net/license/3_01.txt                                  |
dnl | If you did not receive a copy of the PHP license and are unable to   |
dnl | obtain it through the world-wide-web, please send a note to          |
dnl | license@php.net so we can mail you a copy immediately.               |
dnl +----------------------------------------------------------------------+
dnl | Author: Ruslan Osmanov <osmanov@php.net>                             |
dnl +----------------------------------------------------------------------+

PHP_ARG_ENABLE(ev, whether to enable ev,
[  --enable-ev         Enable ev support], no)

PHP_ARG_ENABLE(ev-debug, for ev debug support,
[  --enable-ev-debug       Enable ev debug support], no, no)

PHP_ARG_ENABLE(ev-sockets, for sockets support,
[  --enable-ev-sockets     Enable sockets support in ev], yes, no)

if test "$PHP_EV" != "no"; then
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES -DHAVE_EV"
  AC_MSG_CHECKING(PHP version)
  AC_TRY_COMPILE([#include <php_version.h>], [
#if PHP_VERSION_ID < 50400
#error  this extension requires at least PHP version 5.4.0
#endif
],
[AC_MSG_RESULT(ok)],
[AC_MSG_ERROR([need at least PHP 5.4.0])])
  export CPPFLAGS="$OLD_CPPFLAGS"

  if test "$PHP_EV_DEBUG" != "no"; then
    CFLAGS="$CFLAGS -Wall -g -ggdb -O0"
    AC_DEFINE(PHP_EV_DEBUG, 1, [Enable ev debug support])
  else
    AC_DEFINE(NDEBUG, 1, [With NDEBUG defined assert generates no code])
  fi

  if test "$PHP_EV_SOCKETS" != "no"; then
    PHP_ADD_EXTENSION_DEP(ev, sockets, true)
    AC_DEFINE([PHP_EV_USE_SOCKETS], 1, [Whether to enable sockets support])
  fi

  AC_DEFINE(EV_H, "embed.h", [Wrapper for libev/ev.h])
  AC_DEFINE(HAVE_EV, 1, [ ])
  m4_include([libev/libev.m4])

  ev_src="libev/ev.c util.c ev.c watcher.c fe.c pe.c"
  PHP_NEW_EXTENSION(ev, $ev_src, $ext_shared,,$CFLAGS)
fi

dnl vim: ft=m4.sh fdm=marker cms=dnl\ %s
dnl vim: et ts=2 sts=2 sw=2
