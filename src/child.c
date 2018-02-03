
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

/* {{{ php_ev_child_object_ctor */
void php_ev_child_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start)
{
	zend_long      pid;
	zend_bool      trace;
	zval          *self;
	zval          *callback;
	zval          *data     = NULL;
	php_ev_object *o_self;
	ev_child      *w;
	zend_long      priority = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lbz|z!l",
				&pid, &trace, &callback, &data, &priority) == FAILURE) {
		return;
	}

	if (ctor) {
		self = getThis();
	} else {
		object_init_ex(return_value, ev_child_class_entry_ptr);
		self = return_value;
	}

	if (!loop) {
		loop = php_ev_default_loop();
	}

	PHP_EV_NEW_WATCHER(w, ev_child, EV_CHILD, self, loop, callback, data, priority);
	ev_child_set(w, pid, trace);

	o_self = Z_EV_OBJECT_P(self);
	o_self->ptr = (void *) w;

	if (start) {
		PHP_EV_WATCHER_START(ev_child, w);
	}
}
/* }}} */

/* {{{ proto EvChild::__construct(int pid, bool trace, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvChild, __construct)
{
	PHP_EV_WATCHER_CTOR(child, NULL);
}
/* }}} */

/* {{{ proto EvChild::createStopped(int pid, bool trace, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvChild, createStopped)
{
	PHP_EV_WATCHER_FACTORY_NS(child, NULL);
}
/* }}} */

/* {{{ proto void EvChild::set(int pid, bool trace) */
PHP_METHOD(EvChild, set)
{
	zend_long  pid;
	zend_bool  trace;
	ev_child  *w;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lb",
				&pid, &trace) == FAILURE) {
		return;
	}

	w = (ev_child *)PHP_EV_WATCHER_FETCH_FROM_THIS();

	PHP_EV_WATCHER_RESET(ev_child, w, (w, pid, trace));
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
