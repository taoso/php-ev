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

#include "php_ev.h"
#include "util.h"
#include "zend_exceptions.h"

/* Defined in ev.c */
extern zend_class_entry *ev_loop_class_entry_ptr;

/* {{{ php_ev_watcher_callback() */
void php_ev_watcher_callback(EV_P_ ev_watcher *watcher, int revents)
{
	php_ev_func_info *pf = &php_ev_watcher_func(watcher);

	/* libev might have stopped watcher */
	if (php_ev_watcher_flags(watcher) & PHP_EV_WATCHER_FLAG_UNREFED
			&& !ev_is_active(watcher)) {
		PHP_EV_WATCHER_REF(watcher);
	}

	if (UNEXPECTED(revents & EV_ERROR)) {
		int errorno = errno;
		php_error_docref(NULL, E_WARNING, "Libev error(%d): %s", errorno, strerror(errorno));
		PHP_EV_EXIT_LOOP(EV_A);
		return;
	}

	if (EXPECTED(pf->func_ptr)) {
		zval *retval   = NULL;

		zval  zrevents;
		ZVAL_LONG(&zrevents, (zend_long)revents);

		zend_call_method(Z_ISUNDEF(pf->obj) ? NULL : &pf->obj, pf->ce, &pf->func_ptr,
				ZSTR_VAL(pf->func_ptr->common.function_name),
				ZSTR_LEN(pf->func_ptr->common.function_name),
				retval, MIN(2, pf->func_ptr->common.num_args),
				&php_ev_watcher_self(watcher), &zrevents);
		zend_exception_save();
		if (retval) {
			zval_ptr_dtor(retval);
			retval = NULL;
		}
		zend_exception_restore();
	}
}
/* }}} */

/* {{{ php_ev_set_watcher()
 * Configure preallocated watcher of the specified type, configure common watcher fields
 */
int php_ev_set_watcher(ev_watcher *w, int type, zval *zself, zval *zloop, zval *zcb, zval *data, int priority)
{
	php_ev_object    *loop_obj;
	php_ev_loop      *loop_ptr;
	php_ev_func_info *pf;
	char             *error    = NULL;

	PHP_EV_ASSERT(w);
	PHP_EV_ASSERT(zloop);

	pf = &php_ev_watcher_func(w);

	if (php_ev_import_func_info(pf, zcb, error) == FAILURE) {
		zend_throw_exception_ex(zend_ce_exception, 0, "Invalid callback: %s", error);
		if (error) {
			efree(error);
		}
		return FAILURE;
	}
	PHP_EV_EFREE(error);

	loop_obj = Z_EV_OBJECT_P(zloop);
	PHP_EV_ASSERT(loop_obj);

	loop_ptr = PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(loop_obj);
	PHP_EV_ASSERT(loop_ptr);

	/* Re-link the doubly linked list */
	ev_watcher *w_next = loop_ptr->w;
	loop_ptr->w        = w;
	if (w_next) {
		php_ev_watcher_next(w)      = (void *)w_next;
		php_ev_watcher_prev(w_next) = (void *)w;
	}

	ev_init((ev_watcher *)w, (pf->func_ptr ? php_ev_watcher_callback : 0));

	if (data == NULL) {
		ZVAL_UNDEF(&php_ev_watcher_data(w));
	} else {
		ZVAL_COPY(&php_ev_watcher_data(w), data);
	}

	ZVAL_ZVAL(&php_ev_watcher_self(w), zself, 0, 0);

	php_ev_watcher_type(w)  = type;
	php_ev_watcher_loop(w)  = loop_ptr;
	php_ev_watcher_flags(w) = PHP_EV_WATCHER_FLAG_KEEP_ALIVE;
	php_ev_set_watcher_priority(w, priority);

	return SUCCESS;
}
/* }}} */

void php_ev_watcher_dtor(ev_watcher *w)/*{{{*/
{
	ev_watcher  *w_next  , *w_prev;
	php_ev_loop *loop_ptr;

	if (UNEXPECTED(w == NULL)) {
		return;
	}

	/* What if we do it in php_ev_loop_free_storage()?*/
	php_ev_stop_watcher(w);

	/* Re-link the list of watchers */

	w_next = php_ev_watcher_next(w);
	w_prev = php_ev_watcher_prev(w);

	if (w_prev) {
		php_ev_watcher_next(w_prev) = w_next;
	}
	if (w_next) {
		php_ev_watcher_prev(w_next) = w_prev;
	}

	loop_ptr = php_ev_watcher_loop(w);
	if (loop_ptr) {
		if (loop_ptr->w == w) {
			loop_ptr->w = w_next;
		}
	}

	php_ev_func_info_free(&php_ev_watcher_func(w), FALSE);

	if (!Z_ISUNDEF(php_ev_watcher_data(w))) {
		zval_ptr_dtor(&php_ev_watcher_data(w));
		ZVAL_UNDEF(&php_ev_watcher_data(w));
	}

	php_ev_watcher_next(w) = php_ev_watcher_prev(w) = NULL;
}
/*}}}*/

ev_watcher * php_ev_new_watcher(size_t size, int type, zval *zself, zval *zloop, zval *zcb, zval *data, int priority)/*{{{*/
{
	ev_watcher *w = ecalloc(1, size);
	if (UNEXPECTED(w == NULL)) {
		return w;
	}
	return (EXPECTED(php_ev_set_watcher(w, type, zself, zloop, zcb, data, priority) == SUCCESS) ? w : NULL);
}
/*}}}*/

/* {{{ php_ev_start_watcher() */
void php_ev_start_watcher(ev_watcher *watcher)
{
	switch (watcher->type) {
		case EV_IO:
			PHP_EV_WATCHER_START(ev_io, watcher);
			break;
		case EV_TIMER:
			PHP_EV_WATCHER_START(ev_timer, watcher);
			break;
#if EV_PERIODIC_ENABLE
		case EV_PERIODIC:
			PHP_EV_WATCHER_START(ev_periodic, watcher);
			break;
#endif
#if EV_SIGNAL_ENABLE
		case EV_SIGNAL:
			PHP_EV_SIGNAL_START((ev_signal *) watcher);
			break;
#endif
#if EV_CHILD_ENABLE
		case EV_CHILD:
			PHP_EV_WATCHER_START(ev_child, watcher);
			break;
#endif
#if EV_STAT_ENABLE
		case EV_STAT:
			PHP_EV_WATCHER_START(ev_stat, watcher);
			break;
#endif
#if EV_IDLE_ENABLE
		case EV_IDLE:
			PHP_EV_WATCHER_START(ev_idle, watcher);
			break;
#endif
#if EV_PREPARE_ENABLE
		case EV_PREPARE:
			PHP_EV_WATCHER_START(ev_prepare, watcher);
			break;
#endif
#if EV_CHECK_ENABLE
		case EV_CHECK:
			PHP_EV_WATCHER_START(ev_check, watcher);
			break;
#endif
#if EV_EMBED_ENABLE
		case EV_EMBED:
			PHP_EV_WATCHER_START(ev_embed, watcher);
			break;
#endif
#if EV_FORK_ENABLE
		case EV_FORK:
			PHP_EV_WATCHER_START(ev_fork, watcher);
			break;
#endif
#if EV_ASYNC_ENABLE
		case EV_ASYNC:
			PHP_EV_WATCHER_START(ev_async, watcher);
			break;
#endif
		default:
			break;
	}
}
/* }}} */

/* {{{ php_ev_stop_watcher() */
void php_ev_stop_watcher(ev_watcher *watcher)
{
	switch (php_ev_watcher_type(watcher)) {
		case EV_IO:
			PHP_EV_WATCHER_STOP(ev_io, watcher);
			break;
		case EV_TIMER:
			PHP_EV_WATCHER_STOP(ev_timer, watcher);
			break;
#if EV_PERIODIC_ENABLE
		case EV_PERIODIC:
			PHP_EV_WATCHER_STOP(ev_periodic, watcher);
			break;
#endif
#if EV_SIGNAL_ENABLE
		case EV_SIGNAL:
			PHP_EV_SIGNAL_STOP((ev_signal *) watcher);
			break;
#endif
#if EV_CHILD_ENABLE
		case EV_CHILD:
			PHP_EV_WATCHER_STOP(ev_child, watcher);
			break;
#endif
#if EV_STAT_ENABLE
		case EV_STAT:
			PHP_EV_WATCHER_STOP(ev_stat, watcher);
			break;
#endif
#if EV_IDLE_ENABLE
		case EV_IDLE:
			PHP_EV_WATCHER_STOP(ev_idle, watcher);
			break;
#endif
#if EV_PREPARE_ENABLE
		case EV_PREPARE:
			PHP_EV_WATCHER_STOP(ev_prepare, watcher);
			break;
#endif
#if EV_CHECK_ENABLE
		case EV_CHECK:
			PHP_EV_WATCHER_STOP(ev_check, watcher);
			break;
#endif
#if EV_EMBED_ENABLE
		case EV_EMBED:
			PHP_EV_WATCHER_STOP(ev_embed, watcher);
			break;
#endif
#if EV_FORK_ENABLE
		case EV_FORK:
			PHP_EV_WATCHER_STOP(ev_fork, watcher);
			break;
#endif
#if EV_ASYNC_ENABLE
		case EV_ASYNC:
			PHP_EV_WATCHER_STOP(ev_async, watcher);
			break;
#endif
		default:
			break;
	}
}
/* }}} */


/* {{{ Methods */

/* {{{ proto void EvWatcher::start(void) */
PHP_METHOD(EvWatcher, start)
{
	php_ev_object *o_self;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	o_self = Z_EV_OBJECT_P(getThis());

	php_ev_start_watcher(PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self));
}
/* }}} */

/* {{{ proto void EvWatcher::stop(void) */
PHP_METHOD(EvWatcher, stop)
{
	php_ev_object *o_self;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	o_self = Z_EV_OBJECT_P(getThis());

	php_ev_stop_watcher(PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self));
}
/* }}} */

/* {{{ proto int EvWatcher::clear(void) */
PHP_METHOD(EvWatcher, clear)
{
	ev_watcher *w;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	w = PHP_EV_WATCHER_FETCH_FROM_OBJECT(Z_EV_OBJECT_P(getThis()));

	RETURN_LONG((long)ev_clear_pending(php_ev_watcher_loop_ptr(w), w));
}
/* }}} */

/* {{{ proto void EvWatcher::invoke(int revents) */
PHP_METHOD(EvWatcher, invoke)
{
	ev_watcher *w;
	zend_long   revents;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &revents) == FAILURE) {
		return;
	}

	w = PHP_EV_WATCHER_FETCH_FROM_OBJECT(Z_EV_OBJECT_P(getThis()));

	ev_invoke(php_ev_watcher_loop_ptr(w), w, (int)revents);
}
/* }}} */

/* {{{ proto void EvWatcher::feed(int revents) */
PHP_METHOD(EvWatcher, feed)
{
	ev_watcher    *w;
	zend_long      revents;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",
				&revents) == FAILURE) {
		return;
	}

	w = PHP_EV_WATCHER_FETCH_FROM_OBJECT(Z_EV_OBJECT_P(getThis()));

	ev_feed_event(php_ev_watcher_loop_ptr(w), w, (int)revents);
}
/* }}} */

/* {{{ proto EvLoop EvWatcher::getLoop(void) */
PHP_METHOD(EvWatcher, getLoop)
{
	php_ev_loop   *o_loop;
	ev_watcher    *w;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	w      = PHP_EV_WATCHER_FETCH_FROM_OBJECT(Z_EV_OBJECT_P(getThis()));
	o_loop = php_ev_watcher_loop(w);

	zval *zloop = (zval *)ev_userdata(o_loop->loop);

    if (!zloop) {
    	RETURN_NULL();
    }
	RETVAL_ZVAL(zloop, 1, 0);
}
/* }}} */

/* {{{ proto int EvWatcher::keepalive([bool value = TRUE]) */
PHP_METHOD(EvWatcher, keepalive)
{
	ev_watcher *w;
	zend_bool n_value = TRUE;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &n_value) == FAILURE) {
		return;
	}

	w = PHP_EV_WATCHER_FETCH_FROM_THIS();

	/* Returning previous state */
	RETVAL_BOOL((php_ev_watcher_flags(w) & PHP_EV_WATCHER_FLAG_KEEP_ALIVE));
	n_value = n_value != FALSE ? PHP_EV_WATCHER_FLAG_KEEP_ALIVE : 0;

	/* Update watcher flags, if keepalive flag changed */
	if ((n_value ^ php_ev_watcher_flags(w)) & PHP_EV_WATCHER_FLAG_KEEP_ALIVE) {
		php_ev_watcher_flags(w) = (php_ev_watcher_flags(w) & ~PHP_EV_WATCHER_FLAG_KEEP_ALIVE) | n_value;
		PHP_EV_WATCHER_REF(w);
		PHP_EV_WATCHER_UNREF(w);
	}
}
/* }}} */

/* {{{ proto void EvWatcher::setCallback(callable callback) */
PHP_METHOD(EvWatcher, setCallback)
{
	ev_watcher       *w     = PHP_EV_WATCHER_FETCH_FROM_THIS();
	php_ev_func_info *pf    = &php_ev_watcher_func(w);
	char             *error = NULL;
	zval             *zcb;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &zcb) == FAILURE) {
		return;
	}

	if (php_ev_import_func_info(pf, zcb, error) == FAILURE) {
		zend_throw_exception_ex(zend_ce_exception, 0, "Invalid callback: %s", error);
		if (error) {
			efree(error);
		}
		return;
	}
}
/* }}} */

/* Methods }}} */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
