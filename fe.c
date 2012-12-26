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

#include "fe.h"

/* {{{ ARGINFO */

/* TODO: Refactor: some arginfoes can be identical,
 * e.g. EvLoop::io() and EvIo::__construct() */

ZEND_BEGIN_ARG_INFO(arginfo_ev__void, 0)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_feed_signal_event, 0, 0, 1)
	ZEND_ARG_INFO(0, signum)
ZEND_END_ARG_INFO();

/* {{{ EvLoop */

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_default_loop, 0, 0, 0)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, io_interval)
	ZEND_ARG_INFO(0, timeout_interval)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_sleep, 0, 0, 1)
	ZEND_ARG_INFO(0, seconds)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_run, 0, 0, 0)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_stop, 0, 0, 0)
	ZEND_ARG_INFO(0, how)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_feed_signal, 0, 0, 1)
	ZEND_ARG_INFO(0, signum)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_loop_io, 0, 0, 3)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, events)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_loop_timer, 0, 0, 3)
	ZEND_ARG_INFO(0, after)
	ZEND_ARG_INFO(0, repeat)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

#if EV_PERIODIC_ENABLE
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_loop_periodic, 0, 0, 3)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, interval)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
#endif
#if EV_SIGNAL_ENABLE
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_loop_signal, 0, 0, 2)
	ZEND_ARG_INFO(0, signum)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
#endif
#if EV_CHILD_ENABLE
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_loop_child, 0, 0, 3)
	ZEND_ARG_INFO(0, pid)
	ZEND_ARG_INFO(0, trace)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
#endif
#if EV_STAT_ENABLE
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_loop_stat, 0, 0, 3)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, interval)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
#endif
#if EV_IDLE_ENABLE
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_loop_idle, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
#endif
#if EV_CHECK_ENABLE
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_loop_check, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
#endif
#if EV_PREPARE_ENABLE
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_loop_prepare, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
#endif
#if EV_EMBED_ENABLE
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_loop_embed, 0, 0, 1)
	ZEND_ARG_INFO(0, other)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
#endif
#if EV_FORK_ENABLE
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_loop_fork, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
#endif

/* EvLoop }}} */

/* {{{ EvWatcher */


ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_watcher_invoke, 0, 0, 1)
	ZEND_ARG_INFO(0, revents)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_watcher_feed, 0, 0, 1)
	ZEND_ARG_INFO(0, revents)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_watcher_keepalive, 0, 0, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_watcher_set_callback, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO();

/* EvWatcher }}} */

/* {{{ EvIo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_io, 0, 0, 3)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, events)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_io_set, 0, 0, 2)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_INFO(0, events)
ZEND_END_ARG_INFO();
/* EvIo }}} */

/* {{{ EvTimer */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_timer, 0, 0, 3)
	ZEND_ARG_INFO(0, after)
	ZEND_ARG_INFO(0, repeat)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_timer_set, 0, 0, 2)
	ZEND_ARG_INFO(0, after)
	ZEND_ARG_INFO(0, repeat)
ZEND_END_ARG_INFO();
/* }}} */

#if EV_PERIODIC_ENABLE
/* {{{ EvPeriodic */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_periodic, 0, 0, 4)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, interval)
	ZEND_ARG_INFO(0, reschedule_cb)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_periodic_set, 0, 0, 2)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, interval)
ZEND_END_ARG_INFO();
/* }}} */
#endif

#if EV_SIGNAL_ENABLE
/* {{{ EvSignal*/
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_signal, 0, 0, 2)
	ZEND_ARG_INFO(0, signum)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_signal_set, 0, 0, 1)
	ZEND_ARG_INFO(0, signum)
ZEND_END_ARG_INFO();
/* }}} */
#endif

#if EV_CHILD_ENABLE
/* {{{ EvChild*/
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_child, 0, 0, 3)
	ZEND_ARG_INFO(0, pid)
	ZEND_ARG_INFO(0, trace)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_child_set, 0, 0, 2)
	ZEND_ARG_INFO(0, pid)
	ZEND_ARG_INFO(0, trace)
ZEND_END_ARG_INFO();
/* }}} */
#endif

#if EV_STAT_ENABLE
/* {{{ EvStat */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_stat, 0, 0, 3)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, interval)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_stat_set, 0, 0, 2)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, interval)
ZEND_END_ARG_INFO();
/* }}} */
#endif

#if EV_IDLE_ENABLE
/* {{{ EvIdle */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_idle, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
/* }}} */
#endif

#if EV_CHECK_ENABLE
/* {{{ EvCheck */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_check, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
/* }}} */
#endif

#if EV_PREPARE_ENABLE
/* {{{ EvPrepare */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_prepare, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
/* }}} */
#endif

#if EV_EMBED_ENABLE
/* {{{ EvEmbed */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_embed, 0, 0, 1)
	ZEND_ARG_INFO(0, other)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_embed_set, 0, 0, 1)
	ZEND_ARG_INFO(0, other)
ZEND_END_ARG_INFO();
/* }}} */
#endif

#if EV_FORK_ENABLE
/* {{{ EvFork */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ev_fork, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, priority)
ZEND_END_ARG_INFO();
/* }}} */
#endif

/* ARGINFO }}} */

/* {{{ ev_class_entry_functions */
const zend_function_entry ev_class_entry_functions[] = {
	PHP_ME(Ev, supportedBackends,   arginfo_ev__void,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, recommendedBackends, arginfo_ev__void,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, embeddableBackends,  arginfo_ev__void,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, sleep,               arginfo_ev_sleep,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, time,                arginfo_ev__void,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, feedSignal,          arginfo_ev_feed_signal,       ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, feedSignalEvent,     arginfo_ev_feed_signal_event, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, run,                 arginfo_ev_run,               ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, now,                 arginfo_ev__void,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, stop,                arginfo_ev_stop,              ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, iteration,           arginfo_ev__void,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, depth,               arginfo_ev__void,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, verify,              arginfo_ev__void,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, backend,             arginfo_ev__void,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, nowUpdate,           arginfo_ev__void,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, suspend,             arginfo_ev__void,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)
	PHP_ME(Ev, resume,              arginfo_ev__void,             ZEND_ACC_PUBLIC | ZEND_ACC_STATIC | ZEND_ACC_FINAL)

	{ NULL, NULL, NULL }
};
/* }}} */

/* {{{ ev_loop_class_entry_functions */
const zend_function_entry ev_loop_class_entry_functions[] = {
	PHP_ME(EvLoop, __construct,   arginfo_ev_default_loop, ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EvLoop, defaultLoop,   arginfo_ev_default_loop, ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	PHP_ME(EvLoop, loopFork,      arginfo_ev__void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, verify,        arginfo_ev__void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, invokePending, arginfo_ev__void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, nowUpdate,     arginfo_ev__void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, suspend,       arginfo_ev__void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, resume,        arginfo_ev__void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, backend,       arginfo_ev__void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, now,           arginfo_ev__void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, run,           arginfo_ev_run,          ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, stop,          arginfo_ev_stop,         ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, io,            arginfo_ev_loop_io,      ZEND_ACC_PUBLIC)
	PHP_ME(EvLoop, timer,         arginfo_ev_loop_timer,   ZEND_ACC_PUBLIC)
#if EV_PERIODIC_ENABLE
	PHP_ME(EvLoop, periodic,      arginfo_ev_loop_periodic,     ZEND_ACC_PUBLIC)
#endif
#if EV_SIGNAL_ENABLE
	PHP_ME(EvLoop, signal,        arginfo_ev_loop_signal,       ZEND_ACC_PUBLIC)
#endif
#if EV_CHILD_ENABLE
	PHP_ME(EvLoop, child,         arginfo_ev_loop_child,        ZEND_ACC_PUBLIC)
#endif
#if EV_STAT_ENABLE
	PHP_ME(EvLoop, stat,          arginfo_ev_loop_stat,         ZEND_ACC_PUBLIC)
#endif
#if EV_IDLE_ENABLE
	PHP_ME(EvLoop, idle,          arginfo_ev_loop_idle,         ZEND_ACC_PUBLIC)
#endif
#if EV_CHECK_ENABLE
	PHP_ME(EvLoop, check,         arginfo_ev_loop_check,        ZEND_ACC_PUBLIC)
#endif
#if EV_PREPARE_ENABLE
	PHP_ME(EvLoop, prepare,       arginfo_ev_loop_prepare,      ZEND_ACC_PUBLIC)
#endif
#if EV_EMBED_ENABLE
	PHP_ME(EvLoop, embed,         arginfo_ev_loop_embed,        ZEND_ACC_PUBLIC)
#endif
#if EV_FORK_ENABLE
	PHP_ME(EvLoop, fork,          arginfo_ev_loop_fork,         ZEND_ACC_PUBLIC)
#endif

	{ NULL, NULL, NULL }
};
/* }}} */

/* {{{ ev_watcher_class_entry_functions */
const zend_function_entry ev_watcher_class_entry_functions[] = {
	PHP_ABSTRACT_ME(EvWatcher, __construct, NULL)

	PHP_ME(EvWatcher, start,       arginfo_ev__void,                ZEND_ACC_PUBLIC)
	PHP_ME(EvWatcher, stop,        arginfo_ev__void,                ZEND_ACC_PUBLIC)
	PHP_ME(EvWatcher, clear,       arginfo_ev__void,                ZEND_ACC_PUBLIC)
	PHP_ME(EvWatcher, invoke,      arginfo_ev_watcher_invoke,       ZEND_ACC_PUBLIC)
	PHP_ME(EvWatcher, feed,        arginfo_ev_watcher_feed,         ZEND_ACC_PUBLIC)
	PHP_ME(EvWatcher, getLoop,     arginfo_ev__void,                ZEND_ACC_PUBLIC)
	PHP_ME(EvWatcher, keepalive,   arginfo_ev_watcher_keepalive,    ZEND_ACC_PUBLIC)
	PHP_ME(EvWatcher, setCallback, arginfo_ev_watcher_set_callback, ZEND_ACC_PUBLIC)

	{ NULL, NULL, NULL }
};
/* }}} */

/* {{{ ev_io_class_entry_functions */
const zend_function_entry ev_io_class_entry_functions[] = {
	PHP_ME(EvIo, __construct,   arginfo_ev_io,     ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EvIo, set,           arginfo_ev_io_set, ZEND_ACC_PUBLIC)
	PHP_ME(EvIo, createStopped, arginfo_ev_io,     ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	{ NULL, NULL, NULL }
};
/* }}} */

/* {{{ ev_timer_class_entry_functions */
const zend_function_entry ev_timer_class_entry_functions[] = {
	PHP_ME(EvTimer, __construct,   arginfo_ev_timer,     ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EvTimer, set,           arginfo_ev_timer_set, ZEND_ACC_PUBLIC)
	PHP_ME(EvTimer, again,         arginfo_ev__void,     ZEND_ACC_PUBLIC)
	PHP_ME(EvTimer, createStopped, arginfo_ev_timer,     ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)

	{ NULL, NULL, NULL }
};
/* }}} */

#if EV_PERIODIC_ENABLE
/* {{{ ev_periodic_class_entry_functions */
const zend_function_entry ev_periodic_class_entry_functions[] = {
	PHP_ME(EvPeriodic, __construct,   arginfo_ev_periodic,     ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EvPeriodic, set,           arginfo_ev_periodic_set, ZEND_ACC_PUBLIC)
	PHP_ME(EvPeriodic, again,         arginfo_ev__void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvPeriodic, at,            arginfo_ev__void,        ZEND_ACC_PUBLIC)
	PHP_ME(EvPeriodic, createStopped, arginfo_ev_periodic,     ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)

	{ NULL, NULL, NULL }
};
/* }}} */
#endif

#if EV_SIGNAL_ENABLE
/* {{{ ev_signal_class_entry_functions */
const zend_function_entry ev_signal_class_entry_functions[] = {
	PHP_ME(EvSignal, __construct,   arginfo_ev_signal,     ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EvSignal, set,           arginfo_ev_signal_set, ZEND_ACC_PUBLIC)
	PHP_ME(EvSignal, createStopped, arginfo_ev_signal,     ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)

	{ NULL, NULL, NULL }
};
/* }}} */
#endif

#if EV_CHILD_ENABLE
/* {{{ ev_child_class_entry_functions */
const zend_function_entry ev_child_class_entry_functions[] = {
	PHP_ME(EvChild, __construct,   arginfo_ev_child,     ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EvChild, set,           arginfo_ev_child_set, ZEND_ACC_PUBLIC)
	PHP_ME(EvChild, createStopped, arginfo_ev_child,     ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)

	{ NULL, NULL, NULL }
};
/* }}} */
#endif

#if EV_STAT_ENABLE
/* {{{ ev_stat_class_entry_functions */
const zend_function_entry ev_stat_class_entry_functions[] = {
	PHP_ME(EvStat, __construct,   arginfo_ev_stat,     ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EvStat, set,           arginfo_ev_stat_set, ZEND_ACC_PUBLIC)
	PHP_ME(EvStat, attr,          arginfo_ev__void,    ZEND_ACC_PUBLIC)
	PHP_ME(EvStat, prev,          arginfo_ev__void,    ZEND_ACC_PUBLIC)
	PHP_ME(EvStat, stat,          arginfo_ev__void,    ZEND_ACC_PUBLIC)
	PHP_ME(EvStat, createStopped, arginfo_ev_stat,     ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)

	{ NULL, NULL, NULL }
};
/* }}} */
#endif

#if EV_IDLE_ENABLE
/* {{{ ev_idle_class_entry_functions */
const zend_function_entry ev_idle_class_entry_functions[] = {
	PHP_ME(EvIdle, __construct,   arginfo_ev_idle, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(EvIdle, createStopped, arginfo_ev_idle, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{ NULL, NULL, NULL }
};
/* }}} */
#endif

#if EV_CHECK_ENABLE
/* {{{ ev_check_class_entry_functions */
const zend_function_entry ev_check_class_entry_functions[] = {
	PHP_ME(EvCheck, __construct,   arginfo_ev_check, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(EvCheck, createStopped, arginfo_ev_check, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{ NULL, NULL, NULL }
};
/* }}} */
#endif

#if EV_PREPARE_ENABLE
/* {{{ ev_prepare_class_entry_functions */
const zend_function_entry ev_prepare_class_entry_functions[] = {
	PHP_ME(EvPrepare, __construct,   arginfo_ev_prepare, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(EvPrepare, createStopped, arginfo_ev_prepare, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{ NULL, NULL, NULL }
};
/* }}} */
#endif

#if EV_EMBED_ENABLE
/* {{{ ev_embed_class_entry_functions */
const zend_function_entry ev_embed_class_entry_functions[] = {
	PHP_ME(EvEmbed, __construct,   arginfo_ev_embed,     ZEND_ACC_PUBLIC  | ZEND_ACC_CTOR)
	PHP_ME(EvEmbed, set,           arginfo_ev_embed_set, ZEND_ACC_PUBLIC)
	PHP_ME(EvEmbed, sweep,         arginfo_ev__void,     ZEND_ACC_PUBLIC)
	PHP_ME(EvEmbed, createStopped, arginfo_ev_embed,     ZEND_ACC_PUBLIC  | ZEND_ACC_STATIC)
	{ NULL, NULL, NULL }
};
/* }}} */
#endif

#if EV_FORK_ENABLE
/* {{{ ev_fork_class_entry_functions */
const zend_function_entry ev_fork_class_entry_functions[] = {
	PHP_ME(EvFork, __construct,   arginfo_ev_fork, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(EvFork, createStopped, arginfo_ev_fork, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{ NULL, NULL, NULL }
};
/* }}} */
#endif
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
