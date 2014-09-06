/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2014 The PHP Group                                |
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
	long           signum;
	zval          *self;
	php_ev_object *o_self;
	php_ev_object *o_loop;
	ev_signal     *w;

	zval                  *data       = NULL;
	zend_fcall_info        fci        = empty_fcall_info;
	zend_fcall_info_cache  fcc        = empty_fcall_info_cache;
	long                   priority   = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lf|z!l",
				&signum, &fci, &fcc, &data, &priority) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_SIGNUM(signum);

	if (ctor) {
		self = getThis();
	} else {
		PHP_EV_INIT_CLASS_OBJECT(return_value, ev_signal_class_entry_ptr);
		self = return_value;
	}

	if (!loop) {
		loop = php_ev_default_loop(TSRMLS_C);
	}

	o_self = (php_ev_object *) zend_object_store_get_object(self TSRMLS_CC);
	o_loop = (php_ev_object *) zend_object_store_get_object(loop TSRMLS_CC);
	w      = (ev_signal *) php_ev_new_watcher(sizeof(ev_signal), self,
			PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(o_loop),
			&fci, &fcc, data, priority TSRMLS_CC);

	w->type = EV_SIGNAL;

	ev_signal_set(w, signum);

	o_self->ptr = (void *) w;

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
	long       signum;
	ev_signal *w;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&signum) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_SIGNUM(signum);

	w = (ev_signal *) PHP_EV_WATCHER_FETCH_FROM_THIS();

	PHP_EV_SIGNAL_RESET(w, (w, signum));
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
