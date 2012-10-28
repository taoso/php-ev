/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2012 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Ruslan Osmanov <osmanov@php.net>                             |
   +----------------------------------------------------------------------+
*/

#include "php_ev_fe.h"

/* {{{ ARGINFO */

/* {{{ EvLoop */
ZEND_BEGIN_ARG_INFO(arginfo_ev__loop_void, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_default_loop, 0, 0, 0)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, io_collect_interval)
	ZEND_ARG_INFO(0, timeout_collect_interval)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_sleep, 0, 0, 1)
	ZEND_ARG_INFO(0, seconds)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_run, 0, 0, 0)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_break, 0, 0, 0)
	ZEND_ARG_INFO(0, how)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_feed_signal, 0, 0, 1)
	ZEND_ARG_INFO(0, signum)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_feed_signal_event, 0, 0, 1)
	ZEND_ARG_INFO(0, signum)
ZEND_END_ARG_INFO();
/* EvLoop }}} */

/* {{{ EvWatcher */
ZEND_BEGIN_ARG_INFO(arginfo_ev__watcher_void, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_watcher_invoke, 0, 0, 1)
	ZEND_ARG_INFO(0, revents)
ZEND_END_ARG_INFO();
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_watcher_feed, 0, 0, 1)
	ZEND_ARG_INFO(0, revents)
ZEND_END_ARG_INFO();
/* EvWatcher }}} */

/* {{{ EvIo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_io, 0, 0, 3)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, events)
	ZEND_ARG_INFO(0, loop)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
/* EvIo }}} */
/* ARGINFO }}} */


/* {{{ ev_functions[] */
const zend_function_entry ev_functions[] = {
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ ev_loop_class_entry_functions */
const zend_function_entry ev_loop_class_entry_functions[] = {
	PHP_ME(EvLoop, __construct,          arginfo_ev_default_loop,      ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EvLoop, default_loop,         arginfo_ev_default_loop,      ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	PHP_ME(EvLoop, loop_fork,            arginfo_ev__loop_void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, verify,               arginfo_ev__loop_void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, invoke_pending,       arginfo_ev__loop_void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, now_update,           arginfo_ev__loop_void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, suspend,              arginfo_ev__loop_void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, resume,               arginfo_ev__loop_void,        ZEND_ACC_PUBLIC)

	PHP_ME(EvLoop, supported_backends,   arginfo_ev__loop_void,        ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	PHP_ME(EvLoop, recommended_backends, arginfo_ev__loop_void,        ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	PHP_ME(EvLoop, embeddable_backends,  arginfo_ev__loop_void,        ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)

	PHP_ME(EvLoop, sleep,                arginfo_ev_sleep,             ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	PHP_ME(EvLoop, time,                 arginfo_ev__loop_void,        ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	PHP_ME(EvLoop, now,                  arginfo_ev__loop_void,        ZEND_ACC_PUBLIC)

	PHP_ME(EvLoop, run,                  arginfo_ev_run,               ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, break,                arginfo_ev_break,             ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, feed_signal,          arginfo_ev_feed_signal,       ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	PHP_ME(EvLoop, feed_signal_event,    arginfo_ev_feed_signal_event, ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)

	{ NULL, NULL, NULL }
};
/* }}} */

/* {{{ ev_watcher_class_entry_functions */
const zend_function_entry ev_watcher_class_entry_functions[] = {
	PHP_ABSTRACT_ME(EvWatcher, __construct, NULL)

	PHP_ME(EvWatcher, start,   arginfo_ev__watcher_void,  ZEND_ACC_PUBLIC)
	PHP_ME(EvWatcher, stop,    arginfo_ev__watcher_void,  ZEND_ACC_PUBLIC)
	PHP_ME(EvWatcher, clear,   arginfo_ev__watcher_void,  ZEND_ACC_PUBLIC)
	PHP_ME(EvWatcher, invoke,  arginfo_ev_watcher_invoke, ZEND_ACC_PUBLIC)
	PHP_ME(EvWatcher, feed,    arginfo_ev_watcher_feed,   ZEND_ACC_PUBLIC)
	PHP_ME(EvWatcher, getLoop, arginfo_ev__watcher_void,  ZEND_ACC_PUBLIC)

	{ NULL, NULL, NULL }
};
/* }}} */

/* {{{ ev_io_class_entry_functions */
const zend_function_entry ev_io_class_entry_functions[] = {
	PHP_ME(EvIo, __construct, arginfo_ev_io, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	{ NULL, NULL, NULL }
};
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
