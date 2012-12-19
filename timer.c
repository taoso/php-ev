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

/* {{{ php_ev_timer_object_ctor */
void php_ev_timer_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop)
{
	double         after;
	double         repeat;
	zval          *self          = NULL;
	php_ev_object *o_self;
	php_ev_object *o_loop;
	ev_timer      *timer_watcher;

	zval                  *data       = NULL;
	zend_fcall_info        fci        = empty_fcall_info;
	zend_fcall_info_cache  fcc        = empty_fcall_info_cache;
	long                   priority   = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ddf|z!l",
				&after, &repeat, &fci, &fcc,
				&data, &priority) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_REPEAT(repeat);

	/* If loop is NULL, then we're in __construct() */
	if (loop) {
		PHP_EV_INIT_CLASS_OBJECT(return_value, ev_timer_class_entry_ptr);

		PHP_EV_ASSERT((self == NULL));
		self = return_value; 
	} else {
		loop = php_ev_default_loop(TSRMLS_C);
		self = getThis();
	}

	o_self        = (php_ev_object *) zend_object_store_get_object(self TSRMLS_CC);
	o_loop        = (php_ev_object *) zend_object_store_get_object(loop TSRMLS_CC);
	timer_watcher = (ev_timer *) php_ev_new_watcher(sizeof(ev_timer), self,
			PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(o_loop),
			&fci, &fcc, data, priority TSRMLS_CC);

	timer_watcher->type = EV_TIMER;
	
	ev_timer_set(timer_watcher, after, repeat);

	o_self->ptr = (void *) timer_watcher;
}
/* }}} */


/* {{{ proto EvTimer::__construct(double after, double repeat, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvTimer, __construct)
{
	php_ev_timer_object_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, NULL);
}
/* }}} */

/* {{{ proto void EvTimer::set(double after, double repeat) */
PHP_METHOD(EvTimer, set)
{
	double      after;
	double      repeat;
	ev_timer   *timer_watcher;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dd",
				&after, &repeat) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_REPEAT(repeat);

	timer_watcher = (ev_timer *) PHP_EV_WATCHER_FETCH_FROM_THIS();

	PHP_EV_WATCHER_RESET(ev_timer, timer_watcher, (timer_watcher, after, repeat));
}
/* }}} */

/* {{{ proto void EvTimer::again(void) */
PHP_METHOD(EvTimer, again)
{
	ev_timer *timer_watcher;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	timer_watcher = (ev_timer *) PHP_EV_WATCHER_FETCH_FROM_THIS();

	PHP_EV_CHECK_REPEAT(timer_watcher->repeat);

	ev_timer_again(php_ev_watcher_loop_ptr(timer_watcher), timer_watcher);
	PHP_EV_WATCHER_UNREF(timer_watcher); 
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
