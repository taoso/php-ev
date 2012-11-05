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

/* {{{ php_ev_loop_object_ctor
 * Creates an instance of EvLoop class and puts it in retval
 * in_ctor: whether called from constructor of a loop class 
 * default_loop: whether to return/create the default loop */
static void php_ev_loop_object_ctor(INTERNAL_FUNCTION_PARAMETERS, const zend_bool in_ctor, const zend_bool is_default_loop)
{
	php_ev_object          *ev_obj;
	ev_loop                *loop;
	zval                  **default_loop_ptr_ptr     = NULL;
	zval                   *self                     = getThis();

	long                    flags                    = EVFLAG_AUTO;
	double                  io_collect_interval      = 0.;
	double                  timeout_collect_interval = 0.;
	zval                   *data                     = NULL;
	zend_fcall_info         fci                      = empty_fcall_info;
	zend_fcall_info_cache   fcc                      = empty_fcall_info_cache;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|lf!z!dd",
				&flags, &fci, &fcc, &data, 
				&io_collect_interval, &timeout_collect_interval) == FAILURE) {
		return;
	}

	default_loop_ptr_ptr = &MyG(default_loop);

	if (!in_ctor) {
		/* Factory method mode */
		if (is_default_loop) {
			if (!*default_loop_ptr_ptr) {
				loop = ev_default_loop(flags);
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"Returning previously created default loop");
				RETURN_ZVAL(*default_loop_ptr_ptr, /* copy */ 1, /* dtor*/ 0);
				return;
			}
		} else {
			loop = ev_loop_new(flags);
		}

		if (!loop) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR,
					"Failed to instanciate default loop, "
					"bad $LIBEV_FLAGS in environment?");
			return;
		}

		/* Assign return value */
		Z_TYPE_P(return_value) = IS_OBJECT;
		object_init_ex(return_value, ev_loop_class_entry_ptr);
		Z_SET_REFCOUNT_P(return_value, 1);
		Z_UNSET_ISREF_P(return_value);

		ev_obj = (php_ev_object *) zend_object_store_get_object(return_value TSRMLS_CC);

		/* Save return_value in MyG(default_loop) */
		if (!*default_loop_ptr_ptr) {
			MAKE_STD_ZVAL(*default_loop_ptr_ptr);
			REPLACE_ZVAL_VALUE(default_loop_ptr_ptr, return_value, 1);
		}
	} else {
		/* Create custom event loop(OOP API) */
		loop = ev_loop_new(flags);
		if (!loop) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR,
					"Failed to instanciate loop, bad backend, "
					"or bad $LIBEV_FLAGS in environment?");
			return;
		}

		ev_obj = (php_ev_object *) zend_object_store_get_object(self TSRMLS_CC);
	}

	php_ev_loop *ptr = (php_ev_loop *) emalloc(sizeof(php_ev_loop));
	memset(ptr, 0, sizeof(php_ev_loop));
	ptr->loop = loop;

	PHP_EV_COPY_FCALL_INFO(ptr->fci, ptr->fcc, &fci, &fcc);

	if (data) {
		Z_ADDREF_P(data);
	}
	ptr->data                     = data;
	ptr->io_collect_interval      = io_collect_interval;
	ptr->timeout_collect_interval = timeout_collect_interval;
	ev_obj->ptr                   = (void *) ptr;

	ev_set_userdata(loop, (void *) return_value); 
}
/* }}} */

/* {{{ proto EvLoop EvLoop::default_loop([int flags = EVLAG_AUTO[, callback callback = NULL[, mixed data = NULL[, double io_collect_interval = 0.[, double timeout_collect_interval = 0.]]]]])
*/
PHP_METHOD(EvLoop, default_loop)
{
	php_ev_loop_object_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU,
			/* in ctor */FALSE, /* is_default_loop */TRUE);
}
/* }}} */

/* {{{ proto EvLoop EvLoop::__construct([int flags = EVLAG_AUTO[, callback callback = NULL[, mixed data = NULL[, double io_collect_interval = 0.[, double timeout_collect_interval = 0.]]]]]) */
PHP_METHOD(EvLoop, __construct) 
{
	php_ev_loop_object_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU,
			/* in ctor */TRUE, /* is_default_loop */FALSE);
}
/* }}} */


#define PHP_EV_LOOP_METHOD_VOID(name)                  \
    PHP_METHOD(EvLoop, name)                           \
    {                                                  \
        PHP_EV_LOOP_FETCH_FROM_THIS;                   \
                                                       \
        if (zend_parse_parameters_none() == FAILURE) { \
            return;                                    \
        }                                              \
                                                       \
        ev_##name(EV_A);                               \
    }

#define PHP_EV_LOOP_METHOD_INT_VOID(name)              \
    PHP_METHOD(EvLoop, name)                           \
    {                                                  \
        if (zend_parse_parameters_none() == FAILURE) { \
            return;                                    \
        }                                              \
                                                       \
        RETURN_LONG((long)ev_##name());                \
    }

#define PHP_EV_LOOP_METHOD_DOUBLE_VOID(name)           \
    PHP_METHOD(EvLoop, name)                           \
    {                                                  \
        if (zend_parse_parameters_none() == FAILURE) { \
            return;                                    \
        }                                              \
                                                       \
        RETURN_DOUBLE((double)ev_##name());            \
    }

PHP_EV_LOOP_METHOD_VOID(loop_fork)
PHP_EV_LOOP_METHOD_VOID(verify)
PHP_EV_LOOP_METHOD_VOID(invoke_pending)
PHP_EV_LOOP_METHOD_VOID(now_update)
PHP_EV_LOOP_METHOD_VOID(suspend)
PHP_EV_LOOP_METHOD_VOID(resume)

PHP_EV_LOOP_METHOD_INT_VOID(supported_backends)
PHP_EV_LOOP_METHOD_INT_VOID(recommended_backends)
PHP_EV_LOOP_METHOD_INT_VOID(embeddable_backends)

/* {{{ proto void sleep(double seconds) */
PHP_METHOD(EvLoop, sleep) 
{
	double seconds;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &seconds) == FAILURE) {
		return;
	}

	ev_sleep(seconds);
}
/* }}} */

PHP_EV_LOOP_METHOD_DOUBLE_VOID(time)

/* {{{ proto double EvLoop::now(void) */
PHP_METHOD(EvLoop, now)
{
	PHP_EV_LOOP_FETCH_FROM_THIS;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	RETURN_DOUBLE((double)ev_now(EV_A));
}
/* }}} */

/* {{{ proto void EvLoop::run([int flags = 0]) */
PHP_METHOD(EvLoop, run)
{
	PHP_EV_LOOP_FETCH_FROM_THIS;

	long flags = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &flags) == FAILURE) {
		return;
	}

	ev_run(EV_A_ flags);
}
/* }}} */

/* {{{ proto void EvLoop::break([int how = 0]) */
PHP_METHOD(EvLoop, break)
{
	PHP_EV_LOOP_FETCH_FROM_THIS;

	long how = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &how) == FAILURE) {
		return;
	}

	ev_break(EV_A_ how);
}
/* }}} */

/* {{{ proto void EvLoop::feed_signal(int signum) */
PHP_METHOD(EvLoop, feed_signal)
{
	long signum;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &signum) == FAILURE) {
		return;
	}

	ev_feed_signal(signum);
}
/* }}} */

/* {{{ proto void EvLoop::feed_signal_event(int signum) */
PHP_METHOD(EvLoop, feed_signal_event)
{
	long			signum;
	php_ev_object	*ev_obj;
	zval			**default_loop_ptr_ptr = &MyG(default_loop);
	
	if (!*default_loop_ptr_ptr) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"The default loop is not initialized");
		return;
	}

	/* Fetch the default loop */
	ev_obj = (php_ev_object *) zend_object_store_get_object(*default_loop_ptr_ptr TSRMLS_CC);
	EV_P = PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &signum) == FAILURE) {
		return;
	}

	ev_feed_signal_event(EV_A_ signum);
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
