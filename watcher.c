/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
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

/* Defined in ev.c */
extern zend_class_entry *ev_loop_class_entry_ptr;

/* {{{ php_ev_watcher_callback() */
void php_ev_watcher_callback(EV_P_ ev_watcher *watcher, int revents)
{
	zval            **args[2];
	zval             *key2;
	zval             *retval_ptr;
	zval             *self       = php_ev_watcher_self(watcher);
	zend_fcall_info  *pfci       = php_ev_watcher_fci(watcher);

	TSRMLS_FETCH_FROM_CTX(php_ev_watcher_thread_ctx(watcher));

	/* libev might have stopped watcher */
	if (UNEXPECTED(php_ev_watcher_flags(watcher) & PHP_EV_WATCHER_FLAG_UNREFED
			&& !ev_is_active(watcher))) {
		PHP_EV_WATCHER_REF(watcher);
	}

	if (revents & EV_ERROR) {
		int errorno = errno;
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Got unspecified libev error in revents, errno = %d, err = %s", errorno, strerror(errorno));

		PHP_EV_EXIT_LOOP(EV_A);
	} else if (ZEND_FCI_INITIALIZED(*pfci)) {
		/* Setup callback args */
		args[0] = &self;
		Z_ADDREF_P(self);

		MAKE_STD_ZVAL(key2);
		args[1] = &key2;
		ZVAL_LONG(key2, revents);

		/* Prepare callback */
		pfci->params         = args;
		pfci->retval_ptr_ptr = &retval_ptr;
		pfci->param_count    = 2;
		pfci->no_separation  = 1;

		if (zend_call_function(pfci, php_ev_watcher_fcc(watcher) TSRMLS_CC) == SUCCESS
		        && retval_ptr) {
		    zval_ptr_dtor(&retval_ptr);
		} else {
		    php_error_docref(NULL TSRMLS_CC, E_WARNING,
		            "An error occurred while invoking the callback");
		}

		zval_ptr_dtor(&self);
		zval_ptr_dtor(&key2);
	}
}
/* }}} */

/* {{{ php_ev_set_watcher()
 * Configure preallocated watcher of the specified type, initialize common watcher fields
 */
void php_ev_set_watcher(ev_watcher *w, size_t size, zval *self, php_ev_loop *o_loop, const zend_fcall_info *pfci, const zend_fcall_info_cache *pfcc, zval *data, int priority TSRMLS_DC)
{
	/* Re-link the doubly linked list */

	ev_watcher *w_next = o_loop->w;
	o_loop->w          = w;

	if (w_next) {
		php_ev_watcher_next(w)      = (void *) w_next;
		php_ev_watcher_prev(w_next) = (void *) w;
	}

	ev_init((ev_watcher *) w, (ZEND_FCI_INITIALIZED(*pfci) ? php_ev_watcher_callback : 0));

	if (data) {
		Z_ADDREF_P(data);
	}

#if 0
	Z_ADDREF_P(self);
#endif

	php_ev_watcher_self(w)  = self;
	php_ev_watcher_data(w)  = data;
	php_ev_watcher_loop(w)  = o_loop;
	php_ev_watcher_flags(w) = PHP_EV_WATCHER_FLAG_KEEP_ALIVE | PHP_EV_WATCHER_FLAG_SELF_UNREFED;

	PHP_EV_COPY_FCALL_INFO(php_ev_watcher_fci(w), php_ev_watcher_fcc(w), pfci, pfcc);

	php_ev_set_watcher_priority(w, priority);

	TSRMLS_SET_CTX(php_ev_watcher_thread_ctx(w));
}
/* }}} */

/* {{{ php_ev_new_watcher()
 * Create watcher of the specified type, initialize common watcher fields
 */
void *php_ev_new_watcher(size_t size, zval *self, php_ev_loop *o_loop, const zend_fcall_info *pfci, const zend_fcall_info_cache *pfcc, zval *data, int priority TSRMLS_DC)
{
	void *w = emalloc(size);
	memset(w, 0, size);

	php_ev_set_watcher((ev_watcher *) w, size, self, o_loop, pfci, pfcc, data, priority TSRMLS_CC);

	return w;
}
/* }}} */

/* {{{ php_ev_start_watcher() */
void php_ev_start_watcher(ev_watcher *watcher TSRMLS_DC)
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
void php_ev_stop_watcher(ev_watcher *watcher TSRMLS_DC)
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

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	php_ev_start_watcher(PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self) TSRMLS_CC);
}
/* }}} */

/* {{{ proto void EvWatcher::stop(void) */
PHP_METHOD(EvWatcher, stop)
{
	php_ev_object *o_self;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	php_ev_stop_watcher(PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self) TSRMLS_CC);
}
/* }}} */

/* {{{ proto int EvWatcher::clear(void) */
PHP_METHOD(EvWatcher, clear)
{
	php_ev_object *o_self;
	ev_watcher    *w;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	w      = PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self);

	RETURN_LONG((long)ev_clear_pending(php_ev_watcher_loop_ptr(w), w));
}
/* }}} */

/* {{{ proto void EvWatcher::invoke(int revents) */
PHP_METHOD(EvWatcher, invoke)
{
	php_ev_object *o_self;
	ev_watcher    *w;
	long           revents;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&revents) == FAILURE) {
		return;
	}

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	w      = PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self);

	ev_invoke(php_ev_watcher_loop_ptr(w), w, (int)revents);
}
/* }}} */

/* {{{ proto void EvWatcher::feed(int revents) */
PHP_METHOD(EvWatcher, feed)
{
	php_ev_object *o_self;
	ev_watcher    *w;
	long           revents;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
				&revents) == FAILURE) {
		return;
	}

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	w      = PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self);

	ev_feed_event(php_ev_watcher_loop_ptr(w), w, (int)revents);
}
/* }}} */

/* {{{ proto EvLoop EvWatcher::getLoop(void) */
PHP_METHOD(EvWatcher, getLoop)
{
	php_ev_object *o_self;
	php_ev_loop   *o_loop;
	ev_watcher    *w;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	o_self = (php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	w      = PHP_EV_WATCHER_FETCH_FROM_OBJECT(o_self);
	o_loop = php_ev_watcher_loop(w);

	zval *zloop = (zval *) ev_userdata(o_loop->loop);

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
	zend_bool   n_value = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &n_value) == FAILURE) {
		return;
	}

	w = PHP_EV_WATCHER_FETCH_FROM_THIS();

	/* Returning previous state */
	RETVAL_BOOL((php_ev_watcher_flags(w) & PHP_EV_WATCHER_FLAG_KEEP_ALIVE));
	n_value = n_value ? PHP_EV_WATCHER_FLAG_KEEP_ALIVE : 0;

	/* Update watcher flags, if keepalive flag changed */
	if (ZEND_NUM_ARGS() > 0
			&& ((n_value ^ php_ev_watcher_flags(w)) & PHP_EV_WATCHER_FLAG_KEEP_ALIVE)) {
		php_ev_watcher_flags(w) = (php_ev_watcher_flags(w) & ~PHP_EV_WATCHER_FLAG_KEEP_ALIVE) | n_value;
		PHP_EV_WATCHER_REF(w);
		PHP_EV_WATCHER_UNREF(w);
	}
}
/* }}} */

/* {{{ proto void EvWatcher::setCallback(callable callback) */
PHP_METHOD(EvWatcher, setCallback)
{
	ev_watcher            *w;
	zend_fcall_info        fci;
	zend_fcall_info_cache  fcc;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f",
				&fci, &fcc) == FAILURE) {
		return;
	}

	w = PHP_EV_WATCHER_FETCH_FROM_THIS();

	PHP_EV_FREE_FCALL_INFO(w->fci, w->fcc);
	PHP_EV_COPY_FCALL_INFO(w->fci, w->fcc, &fci, &fcc);
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
