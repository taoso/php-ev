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

/* {{{ php_ev_periodic_rescheduler */
static ev_tstamp php_ev_periodic_rescheduler(ev_periodic *w, ev_tstamp now)
{
	ev_tstamp         retval;
	zval             *self;
	php_ev_object    *ev_obj;
	php_ev_periodic  *periodic_ptr;
	zend_fcall_info  *pfci;

	TSRMLS_FETCH_FROM_CTX(php_ev_watcher_thread_ctx(w));

	/* Fetch php_ev_periodic to access rescheduler's fci and fcc */

	self   = php_ev_watcher_self(w);
	ev_obj = (php_ev_object *) zend_object_store_get_object(self TSRMLS_CC);

    if (!ev_obj->ptr) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "Watcher is not initialized");
        return now;
    }

	periodic_ptr = (php_ev_periodic *) ev_obj->ptr;
	pfci         = periodic_ptr->fci;

	if (pfci && ZEND_FCI_INITIALIZED(*pfci)) {
		zval            **args[2];
		zval             *key1;
		zval             *key2;
		zval             *retval_ptr;

		/* Setup callback args */
		key1 = self;
		args[0] = &key1;
		zval_add_ref(&key1);

		MAKE_STD_ZVAL(key2);
		args[1] = &key2;
		ZVAL_DOUBLE(key2, (double) now);

		/* Prepare callback */
		pfci->params         = args;
		pfci->retval_ptr_ptr = &retval_ptr;
		pfci->param_count    = 2;
		pfci->no_separation  = 0;

		if (zend_call_function(pfci, periodic_ptr->fcc TSRMLS_CC) == SUCCESS
		        && retval_ptr) {
		    retval = (ev_tstamp) Z_DVAL_P(retval_ptr);
		    if (retval < now)
		    	retval = now;

		    zval_ptr_dtor(&retval_ptr);
		} else {
			retval = now;
		    php_error_docref(NULL TSRMLS_CC, E_WARNING,
		            "An error occurred while invoking rescheduler callback");
		}

		zval_ptr_dtor(&key1);
		zval_ptr_dtor(&key2);
	} else {
		retval = now;
	}

	return retval;
}
/* }}} */


/* {{{ proto EvPeriodic::__construct(double offset, double interval, callable reschedule_cb, EvLoop loop, callable callback[, mixed data = NULL[, int priority = 0]]) 
 * NOTE: reschedule_cb could be NULL */
PHP_METHOD(EvPeriodic, __construct)
{
	double                 offset;
	double                 interval;
	zend_fcall_info        fci_reschedule;
	zend_fcall_info_cache  fcc_reschedule;

	zval                  *self;
	php_ev_object         *o_self;
	php_ev_object         *o_loop;
	ev_periodic           *periodic_watcher;

	zval                  *loop;
	zval                  *data             = NULL;
	zend_fcall_info        fci              = empty_fcall_info;
	zend_fcall_info_cache  fcc              = empty_fcall_info_cache;
	long                   priority         = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ddf!Of|z!l",
				&offset, &interval, &fci_reschedule, &fcc_reschedule,
				&loop, ev_loop_class_entry_ptr, &fci, &fcc,
				&data, &priority) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_REPEAT(interval);

	php_ev_periodic *periodic_ptr = (php_ev_periodic *) emalloc(sizeof(php_ev_periodic));
	memset(periodic_ptr, 0, sizeof(php_ev_periodic));

	periodic_watcher = &periodic_ptr->periodic;

	self             = getThis();
	o_self           = (php_ev_object *) zend_object_store_get_object(self TSRMLS_CC);
	o_loop           = (php_ev_object *) zend_object_store_get_object(loop TSRMLS_CC);

	php_ev_set_watcher((ev_watcher *)periodic_watcher, sizeof(ev_periodic), self,
			PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(o_loop),
			&fci, &fcc, data, priority TSRMLS_CC);

	periodic_watcher->type = EV_PERIODIC;


	if (ZEND_FCI_INITIALIZED(fci_reschedule)) { /* argument is not NULL */
		PHP_EV_COPY_FCALL_INFO(periodic_ptr->fci, periodic_ptr->fcc,
				&fci_reschedule, &fcc_reschedule);

		ev_periodic_set(periodic_watcher, offset, interval, php_ev_periodic_rescheduler);
	} else {
		ev_periodic_set(periodic_watcher, offset, interval, 0);
	}

	o_self->ptr     = (void *) periodic_ptr;
}
/* }}} */

/* {{{ proto void EvPeriodic::set(double offset, double interval[, callable reschedule_cb]) */
PHP_METHOD(EvPeriodic, set)
{
	double                offset;
	double                interval;
	zend_fcall_info       fci      = empty_fcall_info;
	zend_fcall_info_cache fcc      = empty_fcall_info_cache;

	ev_periodic     *periodic_watcher;
	php_ev_object   *ev_obj;
	php_ev_periodic *periodic_ptr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dd|f",
				&offset, &interval, &fci, &fcc) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_REPEAT(interval);

	ev_obj           = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	periodic_watcher = (ev_periodic *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(ev_obj);
	periodic_ptr     = (php_ev_periodic *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(ev_obj);
	
	/* Free fci and fcc within periodic_ptr, since they will be overwritten anyways */

	if (ZEND_FCI_INITIALIZED(*periodic_ptr->fci)) {
		PHP_EV_FREE_FCALL_INFO(periodic_ptr->fci, periodic_ptr->fcc);
	}

	/* Reconfigure reschedule_cb */

	if (ZEND_FCI_INITIALIZED(fci)) {
		PHP_EV_COPY_FCALL_INFO(periodic_ptr->fci, periodic_ptr->fcc, &fci, &fcc);

		PHP_EV_WATCHER_RESET(ev_periodic, periodic_watcher,
				(periodic_watcher, offset, interval, php_ev_periodic_rescheduler));
	} else {
		PHP_EV_WATCHER_RESET(ev_periodic, periodic_watcher,
				(periodic_watcher, offset, interval, 0));
	}
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
