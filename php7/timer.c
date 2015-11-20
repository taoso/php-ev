/*
   +----------------------------------------------------------------------+
   | PHP Version 7                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2015 The PHP Group                                |
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
#include "watcher.h"

/* {{{ php_ev_timer_object_ctor */
void php_ev_timer_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start)
{
	zval          *self;
	zval          *data     = NULL;
	php_ev_object *intern;
	ev_timer      *w;
	zend_long      priority = 0;
	zval          *callback;
	double         after;
	double         repeat;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ddz|z!l",
				&after, &repeat, &callback, &data, &priority) == FAILURE) {
		return;
	}

	if (!PHP_EV_IS_REPEAT_RANGE_VALID(repeat)) {
		php_error_docref(NULL, E_ERROR, PHP_EV_REPEAT_RANGE_ERROR_STR(repeat));
		return;
	}

	if (ctor) {
		self = getThis();
	} else {
		object_init_ex(return_value, ev_timer_class_entry_ptr);
		self = return_value;
	}

	if (!loop) {
		loop = php_ev_default_loop();
	}

	PHP_EV_NEW_WATCHER(w, ev_timer, EV_TIMER, self, loop, callback, data, priority);
	ev_timer_set(w, after, repeat);

	intern = Z_EV_OBJECT_P(self);
	intern->ptr = (void *)w;

	if (start) {
		PHP_EV_WATCHER_START(ev_timer, (ev_watcher *)w);
	}
}
/* }}} */


/* {{{ proto EvTimer::__construct(double after, double repeat, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvTimer, __construct)
{
	PHP_EV_WATCHER_CTOR(timer, NULL);
}
/* }}} */

/* {{{ proto EvTimer::createStopped(double after, double repeat, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvTimer, createStopped)
{
	PHP_EV_WATCHER_FACTORY_NS(timer, NULL);
}
/* }}} */

/* {{{ proto void EvTimer::set(double after, double repeat) */
PHP_METHOD(EvTimer, set)
{
	double    after;
	double    repeat;
	ev_timer *w;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "dd", &after, &repeat) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_REPEAT(repeat);
	w = (ev_timer *)PHP_EV_WATCHER_FETCH_FROM_THIS();
	PHP_EV_WATCHER_RESET(ev_timer, w, (w, after, repeat));
}
/* }}} */

/* {{{ proto void EvTimer::again(void) */
PHP_METHOD(EvTimer, again)
{
	ev_timer *w;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	w = (ev_timer *)PHP_EV_WATCHER_FETCH_FROM_THIS();

	PHP_EV_CHECK_REPEAT(w->repeat);

	ev_timer_again(php_ev_watcher_loop_ptr((ev_watcher *)w), w);
	PHP_EV_WATCHER_UNREF(w);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
