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

if test "$PHP_EV" != "no"; then
  AC_MSG_CHECKING(whether Ev supports the current PHP version)
  tmp_php_version=$PHP_VERSION
  if test -z "$tmp_php_version"; then
    if test -z "$PHP_CONFIG"; then
      AC_MSG_ERROR([php-config not found])
    fi
    PHP_EV_VERSION_ORIG=`$PHP_CONFIG --version`;
  else
    PHP_EV_VERSION_ORIG=$tmp_php_version
  fi

  if test -z $PHP_EV_VERSION_ORIG; then
    AC_MSG_ERROR([failed to detect PHP version, please file a bug])
  fi

  PHP_EV_VERSION_MASK=`echo ${PHP_EV_VERSION_ORIG} | $AWK 'BEGIN { FS = "."; } { printf "%d", ($1 * 1000 + $2) * 1000 + $3;}'`
  if test $PHP_EV_VERSION_MASK -lt 5004000; then
    AC_MSG_ERROR([need at least PHP 5.4.0])
  else
    AC_MSG_RESULT([ok])
  fi

  if test "$PHP_EV_DEBUG" != "no"; then
    PHP_EV_CFLAGS="$PHP_EV_CFLAGS -Wall -g -ggdb -O0"
    AC_DEFINE(PHP_EV_DEBUG, 1, [Enable ev debug support])
  else
    AC_DEFINE(NDEBUG, 1, [With NDEBUG defined assert generates no code])
  fi

  PHP_ADD_EXTENSION_DEP(ev, sockets, true)

  AC_DEFINE(EV_H, "embed.h", [Wrapper for libev/ev.h])
  AC_DEFINE(HAVE_EV, 1, [ ])

  if test "$ext_shared" != "yes" && test "$ext_shared" != "shared"; then
    PHP_EV_CONFIG_H='\"main/php_config.h\"'
    AC_DEFINE(EV_CONFIG_H, "main/php_config.h", [Overide config.h included in libev/ev.c])
    PHP_EV_CFLAGS="$PHP_EV_CFLAGS -DEV_CONFIG_H="$PHP_EV_CONFIG_H
    define('PHP_EV_STATIC', 1)
  fi

  m4_include(ifdef('PHP_EV_STATIC',PHP_EXT_BUILDDIR(ev)[/],)[libev/libev.m4])

  LIBS="$LIBS -lpthread"
  PHP_ADD_LIBRARY(pthread)

  PHP_EV_CFLAGS="-I@ext_srcdir@/libev $PHP_EV_CFLAGS"

  ev_src="libev/ev.c util.c ev.c watcher.c fe.c pe.c"
  PHP_NEW_EXTENSION(ev, $ev_src, $ext_shared,,$PHP_EV_CFLAGS)

  PHP_ADD_BUILD_DIR($ext_builddir/libev)

  PHP_ADD_MAKEFILE_FRAGMENT
fi

dnl vim: ft=m4.sh fdm=marker cms=dnl\ %s
dnl vim: et ts=2 sts=2 sw=2
