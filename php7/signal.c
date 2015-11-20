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

/* {{{ php_ev_signal_object_ctor */
void php_ev_signal_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start)
{
	zend_long      signum;
	zval          *self;
	zval          *callback;
	zval          *data     = NULL;
	php_ev_object *o_self;
	ev_signal     *w;
	zend_long      priority = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lz|z!l",
				&signum, &callback, &data, &priority) == FAILURE) {
		return;
	}

	if (!PHP_EV_IS_SIGNUM_VALID(signum)) {
		php_error_docref(NULL, E_ERROR, PHP_EV_SIGNUM_ERROR_STR);
		return;
	}

	if (ctor) {
		self = getThis();
	} else {
		object_init_ex(return_value, ev_signal_class_entry_ptr);
		self = return_value;
	}

	if (!loop) {
		loop = php_ev_default_loop();
	}

	PHP_EV_NEW_WATCHER(w, ev_signal, EV_SIGNAL, self, loop, callback, data, priority);
	ev_signal_set(w, signum);

	o_self = Z_EV_OBJECT_P(self);
	o_self->ptr = (void *)w;

	if (start) {
		PHP_EV_SIGNAL_START(w);
	}
}
/* }}} */

/* {{{ proto EvSignal::__construct(int signum, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvSignal, __construct)
{
	PHP_EV_WATCHER_CTOR(signal, NULL);
}
/* }}} */

/* {{{ proto EvSignal::createStopped(int signum, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvSignal, createStopped)
{
	PHP_EV_WATCHER_FACTORY_NS(signal, NULL);
}
/* }}} */

/* {{{ proto void EvSignal::set(int signum) */
PHP_METHOD(EvSignal, set)
{
	zend_long signum;
	ev_signal *w;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &signum) != FAILURE) {
		PHP_EV_CHECK_SIGNUM(signum);
		w = (ev_signal *)PHP_EV_WATCHER_FETCH_FROM_THIS();
		PHP_EV_SIGNAL_RESET(w, (w, signum));
	}
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
