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
		pfci->no_separation  = 1;

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

/* {{{ php_ev_periodic_object_ctor */
void php_ev_periodic_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start)
{
	double                 offset;
	double                 interval;
	zend_fcall_info        fci_reschedule;
	zend_fcall_info_cache  fcc_reschedule;

	zval                  *self;
	php_ev_object         *o_self;
	php_ev_object         *o_loop;
	ev_periodic           *w;
	php_ev_periodic       *periodic_ptr;

	zval                  *data             = NULL;
	zend_fcall_info        fci              = empty_fcall_info;
	zend_fcall_info_cache  fcc              = empty_fcall_info_cache;
	long                   priority         = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ddf!f|z!l",
				&offset, &interval, &fci_reschedule, &fcc_reschedule,
				&fci, &fcc, &data, &priority) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_REPEAT(interval);

	if (ctor) {
		self = getThis();
	} else {
		PHP_EV_INIT_CLASS_OBJECT(return_value, ev_periodic_class_entry_ptr);
		self = return_value;
	}

	if (!loop) {
		loop = php_ev_default_loop(TSRMLS_C);
	}

	periodic_ptr = (php_ev_periodic *) emalloc(sizeof(php_ev_periodic));
	memset(periodic_ptr, 0, sizeof(php_ev_periodic));

	w = &periodic_ptr->periodic;

	o_self = (php_ev_object *) zend_object_store_get_object(self TSRMLS_CC);
	o_loop = (php_ev_object *) zend_object_store_get_object(loop TSRMLS_CC);

	php_ev_set_watcher((ev_watcher *)w, sizeof(ev_periodic), self,
			PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(o_loop),
			&fci, &fcc, data, priority TSRMLS_CC);

	w->type = EV_PERIODIC;

	if (ZEND_FCI_INITIALIZED(fci_reschedule)) { /* argument is not NULL */
		PHP_EV_COPY_FCALL_INFO(periodic_ptr->fci, periodic_ptr->fcc,
				&fci_reschedule, &fcc_reschedule);

		ev_periodic_set(w, offset, interval, php_ev_periodic_rescheduler);
	} else {
		ev_periodic_set(w, offset, interval, 0);
	}

	o_self->ptr = (void *) periodic_ptr;

	if (start) {
		PHP_EV_WATCHER_START(ev_periodic, w);
	}
}
/* }}} */


/* {{{ proto EvPeriodic::__construct(double offset, double interval, callable reschedule_cb, callable callback[, mixed data = NULL[, int priority = 0]])
 * NOTE: reschedule_cb could be NULL */
PHP_METHOD(EvPeriodic, __construct)
{
	PHP_EV_WATCHER_CTOR(periodic, NULL);
}
/* }}} */

/* {{{ proto EvPeriodic::createStopped(double offset, double interval, callable reschedule_cb, callable callback[, mixed data = NULL[, int priority = 0]])
 * NOTE: reschedule_cb could be NULL */
PHP_METHOD(EvPeriodic, createStopped)
{
	PHP_EV_WATCHER_FACTORY_NS(periodic, NULL);
}
/* }}} */

/* {{{ proto void EvPeriodic::set(double offset, double interval[, callable reschedule_cb]) */
PHP_METHOD(EvPeriodic, set)
{
	double                 offset;
	double                 interval;
	zend_fcall_info        fci          = empty_fcall_info;
	zend_fcall_info_cache  fcc          = empty_fcall_info_cache;

	ev_periodic           *w;
	php_ev_object         *ev_obj;
	php_ev_periodic       *periodic_ptr;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dd|f",
				&offset, &interval, &fci, &fcc) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_REPEAT(interval);

	ev_obj       = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	periodic_ptr = (php_ev_periodic *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(ev_obj);
	w            = (ev_periodic *) periodic_ptr;

	/* Free fci and fcc within periodic_ptr, since they will be overwritten anyways */

	if (periodic_ptr->fci && ZEND_FCI_INITIALIZED(*periodic_ptr->fci)) {
		PHP_EV_FREE_FCALL_INFO(periodic_ptr->fci, periodic_ptr->fcc);
	}

	/* Reconfigure reschedule_cb */

	if (ZEND_FCI_INITIALIZED(fci)) {
		PHP_EV_COPY_FCALL_INFO(periodic_ptr->fci, periodic_ptr->fcc, &fci, &fcc);

		PHP_EV_WATCHER_RESET(ev_periodic, w,
				(w, offset, interval, php_ev_periodic_rescheduler));
	} else {
		PHP_EV_WATCHER_RESET(ev_periodic, w,
				(w, offset, interval, 0));
	}
}
/* }}} */

/* {{{ proto void EvPeriodic::again(void) */
PHP_METHOD(EvPeriodic, again)
{
	ev_periodic *w;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	w = (ev_periodic *) PHP_EV_WATCHER_FETCH_FROM_THIS();

	ev_periodic_again(php_ev_watcher_loop_ptr(w), w);
	PHP_EV_WATCHER_UNREF(w);
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
