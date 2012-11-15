Ev - PECL extension

DESCRIPTION
===========

Ev is a PECL extension providing inteface to libev library - high performance
full-featured event loop written in C.


ABOUT LIBEV
-----------

Libev is an event loop: you register interest in certain events (such as a file
descriptor being readable or a timeout occurring), and it will manage these
event sources and provide your program with events.

To do this, it must take more or less complete control over your process (or
thread) by executing the event loop handler, and will then communicate events
via a callback mechanism.

You register interest in certain events by registering so-called event watchers,
and then hand it over to libev by starting the watcher.

For details refer to the libev's homepage:
<http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#EMBEDDING>

For installation instructions see file named INSTALL.


LIBEV IS EMBEDDED
-----------------

You don't need to install libev separately, since it is embedded in this
extension.


PORTABILITY
-----------

Currently GNU/Linux platforms supported only.


AUTHORS
=======

Ruslan Osmanov <osmanov@php.net>


COPYRIGHT
=========

Copyright (c) 2012 Ruslan Osmanov <osmanov@php.net>

This project is subject to version 3.01 of the PHP license, that is bundled
with this package in the file LICENSE, and is available through the
world-wide-web at the following url: http://www.php.net/license/3_01.txt If you
did not receive a copy of the PHP license and are unable to obtain it through
the world-wide-web, please send a note to license@php.net so we can mail you a
copy immediately.

vim: tw=80 ft=markdown

