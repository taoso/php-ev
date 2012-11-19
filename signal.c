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

/* {{{ proto EvSignal::__construct(int signum, EvLoop loop, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvSignal, __construct)
{
	long           signum;
	zval          *self;
	php_ev_object *o_self;
	php_ev_object *o_loop;
	ev_signal     *signal_watcher;

	zval                  *loop;
	zval                  *data       = NULL;
	zend_fcall_info        fci        = empty_fcall_info;
	zend_fcall_info_cache  fcc        = empty_fcall_info_cache;
	long                   priority   = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lOf|z!l",
				&signum, &loop, ev_loop_class_entry_ptr, &fci, &fcc,
				&data, &priority) == FAILURE) {
		return;
	}

	self          = getThis();
	o_self        = (php_ev_object *) zend_object_store_get_object(self TSRMLS_CC);
	o_loop        = (php_ev_object *) zend_object_store_get_object(loop TSRMLS_CC);
	signal_watcher = (ev_signal *) php_ev_new_watcher(sizeof(ev_signal), self,
			PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(o_loop),
			&fci, &fcc, data, priority TSRMLS_CC);

	signal_watcher->type = EV_SIGNAL;
	
	ev_signal_set(signal_watcher, signum);

	o_self->ptr = (void *) signal_watcher;
}
/* }}} */

/* {{{ proto void EvSignal::set(int signum) */
PHP_METHOD(EvSignal, set)
{
	long       signum;
	ev_signal *signal_watcher;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&signum) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_SIGNUM(signum);

	signal_watcher = (ev_signal *) PHP_EV_WATCHER_FETCH_FROM_THIS();

	PHP_EV_SIGNAL_RESET(signal_watcher, (signal_watcher, signum));
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
