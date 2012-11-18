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
#include "watcher.h"

/* {{{ proto EvPeriodic::__construct(double offset, double interval, EvLoop loop, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvPeriodic, __construct)
{
	double         offset;
	double         interval;
	zval          *self;
	php_ev_object *o_self;
	php_ev_object *o_loop;
	ev_periodic   *periodic_watcher;

	zval                  *loop;
	zval                  *data       = NULL;
	zend_fcall_info        fci        = empty_fcall_info;
	zend_fcall_info_cache  fcc        = empty_fcall_info_cache;
	long                   priority   = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ddOf|z!l",
				&offset, &interval, &loop, ev_loop_class_entry_ptr, &fci, &fcc,
				&data, &priority) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_REPEAT(interval);

	self             = getThis();
	o_self           = (php_ev_object *) zend_object_store_get_object(self TSRMLS_CC);
	o_loop           = (php_ev_object *) zend_object_store_get_object(loop TSRMLS_CC);
	periodic_watcher = (ev_periodic *) php_ev_new_watcher(sizeof(ev_periodic), self,
			PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(o_loop),
			&fci, &fcc, data, priority TSRMLS_CC);

	periodic_watcher->type = EV_PERIODIC;
	
	php_printf("ev_periodic_set(%f, %f)\n", offset, interval);
	ev_periodic_set(periodic_watcher, offset, interval, 0);

	o_self->ptr = (void *) periodic_watcher;
}
/* }}} */

/* {{{ proto void EvPeriodic::set(double offset, double interval) */
PHP_METHOD(EvPeriodic, set)
{
	double       offset;
	double       interval;
	ev_periodic *periodic_watcher;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dd",
				&offset, &interval) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_REPEAT(interval);

	periodic_watcher = (ev_periodic *) PHP_EV_WATCHER_FETCH_FROM_THIS();

	PHP_EV_WATCHER_RESET(ev_periodic, periodic_watcher, (periodic_watcher, offset, interval, 0));
}
/* }}} */

/* {{{ proto void EvPeriodic::again(void) */
PHP_METHOD(EvPeriodic, again)
{
	ev_periodic *periodic_watcher;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	periodic_watcher = (ev_periodic *) PHP_EV_WATCHER_FETCH_FROM_THIS();

	ev_periodic_again(php_ev_watcher_loop_ptr(periodic_watcher), periodic_watcher);
	PHP_EV_WATCHER_UNREF(periodic_watcher); 
}
/* }}} */

/* {{{ proto double EvPeriodic::at(void) */
PHP_METHOD(EvPeriodic, at)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_DOUBLE(ev_periodic_at((ev_periodic *)PHP_EV_WATCHER_FETCH_FROM_THIS()));
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
