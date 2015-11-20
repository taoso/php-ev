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
#include "zend_exceptions.h"

/* {{{ php_ev_periodic_rescheduler */
static ev_tstamp php_ev_periodic_rescheduler(ev_periodic *w, ev_tstamp now)
{
	ev_tstamp         tstamp;
	zval             *self;
	php_ev_object    *ev_obj;
	php_ev_periodic  *periodic_ptr;
	php_ev_func_info *pf;

	/* Fetch php_ev_periodic to access rescheduler's fci and fcc */

	self   = &php_ev_watcher_self(w);
	ev_obj = Z_EV_OBJECT_P(self);

	if (!ev_obj->ptr) {
		php_error_docref(NULL, E_WARNING, "Watcher is not initialized");
		return now;
	}

	periodic_ptr = (php_ev_periodic *)ev_obj->ptr;
	pf = &periodic_ptr->func;

	if (EXPECTED(pf->func_ptr)) {
		zval *retval = NULL;
		zval znow;

		ZVAL_DOUBLE(&znow, (double)now);

		zend_call_method(Z_ISUNDEF(pf->obj) ? NULL : &pf->obj, pf->ce, &pf->func_ptr,
				ZSTR_VAL(pf->func_ptr->common.function_name),
				ZSTR_LEN(pf->func_ptr->common.function_name),
				retval, 2, self, &znow);
		zend_exception_save();
		if (retval) {
			tstamp = (ev_tstamp)Z_DVAL_P(retval);
			if (tstamp < now) {
				tstamp = now;
			}
			zval_ptr_dtor(retval);
			retval = NULL;
		} else {
			tstamp = now;
		}
		zend_exception_restore();

	} else {
		tstamp = now;
	}

	return tstamp;
}
/* }}} */

/* {{{ php_ev_periodic_object_ctor */
void php_ev_periodic_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *zloop, zend_bool ctor, zend_bool start)
{
	zval             *self;
	zval             *callback;
	zval             *reschedule_callback = NULL;
	zval             *data                = NULL;
	php_ev_object    *o_self;
	ev_periodic      *w;
	php_ev_periodic  *periodic_ptr;
	zend_long         priority            = 0;
	double            offset;
	double            interval;
	char             *error               = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ddz!z|z!l",
				&offset, &interval,
				&reschedule_callback,
				&callback,
				&data, &priority) == FAILURE) {
		return;
	}

	if (!PHP_EV_IS_REPEAT_RANGE_VALID(interval)) {
		php_error_docref(NULL, E_ERROR, PHP_EV_REPEAT_RANGE_ERROR_STR(interval));
		return;
	}

	if (ctor) {
		self = getThis();
	} else {
		object_init_ex(return_value, ev_periodic_class_entry_ptr);
		self = return_value;
	}

	if (!zloop) {
		zloop = php_ev_default_loop();
	}

	periodic_ptr = ecalloc(1, sizeof(php_ev_periodic));
	if (UNEXPECTED(periodic_ptr == NULL)) {
		php_error_docref(NULL, E_ERROR, "Failed to allocate memory: php_ev_periodic");
		return;
	}
	w = &periodic_ptr->periodic;

	if (php_ev_set_watcher((ev_watcher *)w, EV_PERIODIC, self,
				zloop, callback, data, priority) == FAILURE) {
		efree(periodic_ptr);
		zend_throw_exception_ex(zend_ce_exception, 0, "Watcher configuration failed");
		return;
	}

	if (reschedule_callback) {
		if (php_ev_import_func_info(&periodic_ptr->func, reschedule_callback, error) == FAILURE) {
			zend_throw_exception_ex(zend_ce_exception, 0, "Reschedule callback is invalid: %s", error);
			if (error) {
				efree(error);
			}
			php_ev_watcher_dtor((ev_watcher *)w);
			efree(periodic_ptr);
			return;
		}
	}

	ev_periodic_set(w, offset, interval, (reschedule_callback ? php_ev_periodic_rescheduler : 0));

	o_self = Z_EV_OBJECT_P(self);
	o_self->ptr = (void *)periodic_ptr;

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

/* {{{ proto void EvPeriodic::set(double offset, double interval[, callable reschedule_cb = NULL]) */
PHP_METHOD(EvPeriodic, set)
{
	zval             *callback     = NULL;
	ev_periodic      *w;
	php_ev_object    *ev_obj;
	php_ev_periodic  *periodic_ptr;
	php_ev_func_info *pf;
	char             *error        = NULL;
	double            offset;
	double            interval;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "dd|z!",
				&offset, &interval, &callback) == FAILURE) {
		return;
	}

	PHP_EV_CHECK_REPEAT(interval);

	ev_obj       = Z_EV_OBJECT_P(getThis());
	periodic_ptr = (php_ev_periodic *)PHP_EV_WATCHER_FETCH_FROM_OBJECT(ev_obj);
	w            = (ev_periodic *) periodic_ptr;
	pf           = &periodic_ptr->func;

	/* Reconfigure reschedule_cb */
	if (EXPECTED(pf->func_ptr)) {
		if (php_ev_import_func_info(&periodic_ptr->func, callback, error) == FAILURE) {
			zend_throw_exception_ex(zend_ce_exception, 0,
					"Reschedule callback is invalid: %s", error);
			if (error) {
				efree(error);
			}
			return;
		}
		PHP_EV_WATCHER_RESET(ev_periodic, w, (w, offset, interval, php_ev_periodic_rescheduler));
	} else {
		PHP_EV_WATCHER_RESET(ev_periodic, w, (w, offset, interval, 0));
	}
}
/* }}} */

/* {{{ proto void EvPeriodic::again(void) */
PHP_METHOD(EvPeriodic, again)
{
	ev_periodic *w;

	if (zend_parse_parameters_none() != FAILURE) {
		w = (ev_periodic *)PHP_EV_WATCHER_FETCH_FROM_THIS();
		ev_periodic_again(php_ev_watcher_loop_ptr((ev_watcher *)w), w);
		PHP_EV_WATCHER_UNREF(w);
	}
}
/* }}} */

/* {{{ proto double EvPeriodic::at(void) */
PHP_METHOD(EvPeriodic, at)
{
	if (zend_parse_parameters_none() != FAILURE) {
		RETURN_DOUBLE(ev_periodic_at((ev_periodic *)PHP_EV_WATCHER_FETCH_FROM_THIS()));
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
