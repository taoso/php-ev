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

/* {{{ php_ev_loop_object_ctor
 * Creates an instance of EvLoop class and puts it in retval
 * in_ctor: whether called from constructor of a loop class
 * is_default_loop: whether to return/create the default loop */
static void php_ev_loop_object_ctor(INTERNAL_FUNCTION_PARAMETERS, const zend_bool in_ctor, const zend_bool is_default_loop)
{
	php_ev_object *ev_obj;
	ev_loop       *loop;
	zval          *zdefault_loop_ptr        = NULL;
	zval          *data                     = NULL;
	zend_long      flags                    = EVFLAG_AUTO;
	double         io_collect_interval      = 0.;
	double         timeout_collect_interval = 0.;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|lz!dd",
				&flags, &data,
				&io_collect_interval, &timeout_collect_interval) == FAILURE) {
		return;
	}

	if (!in_ctor) {
		/* Factory method mode */
		if (is_default_loop) {
			zdefault_loop_ptr = php_ev_default_loop();

			if (!zdefault_loop_ptr) {
				loop = ev_default_loop(flags);
			} else {
				RETURN_ZVAL(zdefault_loop_ptr, 0, 0);
			}
		} else {
			loop = ev_loop_new(flags);
		}

		if (!loop) {
			php_error_docref(NULL, E_ERROR,
					"Failed to instanciate default loop, "
					"bad $LIBEV_FLAGS in environment?");
			return;
		}

		object_init_ex(return_value, ev_loop_class_entry_ptr);
		ev_obj = Z_EV_OBJECT_P(return_value);

		if (is_default_loop && !zdefault_loop_ptr) {
			if (!Z_ISUNDEF(MyG(default_loop))) {
				zval_dtor(&MyG(default_loop));
			}
			ZVAL_ZVAL(&MyG(default_loop), return_value, 1, 0);
		}
	} else {
		/* Create custom event loop(OOP API) */
		loop = ev_loop_new(flags);
		if (!loop) {
			php_error_docref(NULL, E_ERROR,
					"Failed to instanciate loop, bad backend, "
					"or bad $LIBEV_FLAGS in environment?");
			return;
		}

		ev_obj = Z_EV_OBJECT_P(getThis());
	}

	php_ev_loop *ptr = (php_ev_loop *)ecalloc(1, sizeof(php_ev_loop));
	ptr->loop = loop;

	if (data == NULL) {
		ZVAL_UNDEF(&ptr->data);
	} else {
		ZVAL_ZVAL(&ptr->data, data, 1, 0);
	}

	ptr->io_collect_interval      = io_collect_interval;
	ptr->timeout_collect_interval = timeout_collect_interval;
	ev_obj->ptr                   = (void *)ptr;

	ev_set_userdata(loop, (void *)(in_ctor ? getThis() : return_value));
}
/* }}} */

/* {{{ proto EvLoop EvLoop::defaultLoop([int flags = Ev::FLAG_AUTO[, mixed data = NULL[, double io_interval = 0.[, double timeout_interval = 0.]]]])
*/
PHP_METHOD(EvLoop, defaultLoop)
{
	php_ev_loop_object_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU,
			/* in ctor */FALSE, /* is_default_loop */TRUE);
}
/* }}} */

/* {{{ proto EvLoop EvLoop::__construct([int flags = Ev::FLAG_AUTO[[, mixed data = NULL[, double io_interval = 0.[, double timeout_interval = 0.]]]]) */
PHP_METHOD(EvLoop, __construct)
{
	php_ev_loop_object_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU,
			/* in ctor */TRUE, /* is_default_loop */FALSE);
}
/* }}} */

#define PHP_EV_LOOP_DECL                              \
	php_ev_object *ev_obj = Z_EV_OBJECT_P(getThis()); \
	PHP_EV_CONSTRUCT_CHECK(ev_obj);                   \
	EV_P = PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj)

#define PHP_EV_LOOP_METHOD_VOID(name, evname)       \
	PHP_METHOD(EvLoop, name)                        \
{                                                   \
	PHP_EV_LOOP_DECL;                               \
	if (zend_parse_parameters_none() != FAILURE) {  \
		ev_ ## evname(EV_A);                        \
	}                                               \
}

#define PHP_EV_LOOP_METHOD_INT_VOID(name, evname)  \
	PHP_METHOD(EvLoop, name)                       \
{                                                  \
	PHP_EV_LOOP_DECL;                              \
	if (zend_parse_parameters_none() != FAILURE) { \
		RETURN_LONG(ev_ ## evname(EV_A));          \
	}                                              \
}

PHP_EV_LOOP_METHOD_VOID(loopFork,      loop_fork)
PHP_EV_LOOP_METHOD_VOID(verify,        verify)
PHP_EV_LOOP_METHOD_VOID(invokePending, invoke_pending)
PHP_EV_LOOP_METHOD_VOID(nowUpdate,     now_update)
PHP_EV_LOOP_METHOD_VOID(suspend,       suspend)
PHP_EV_LOOP_METHOD_VOID(resume,        resume)
PHP_EV_LOOP_METHOD_INT_VOID(backend,   backend)

/* {{{ proto double EvLoop::now(void) */
PHP_METHOD(EvLoop, now)
{
	PHP_EV_LOOP_DECL;

	if (zend_parse_parameters_none() != FAILURE) {
		RETURN_DOUBLE((double)ev_now(EV_A));
	}
}
/* }}} */

/* {{{ proto void EvLoop::run([int flags = 0]) */
PHP_METHOD(EvLoop, run)
{
	PHP_EV_LOOP_DECL;
	zend_long flags = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &flags) != FAILURE) {
		ev_run(EV_A_ flags);
	}
}
/* }}} */

/* {{{ proto void EvLoop::stop([int how = 0]) */
PHP_METHOD(EvLoop, stop)
{
	PHP_EV_LOOP_DECL;
	zend_long how = EVBREAK_ONE;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &how) != FAILURE) {
		ev_break(EV_A_ how);
	}
}
/* }}} */

/* {{{ proto EvIo EvLoop::io(mixed fd, int events, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvLoop, io)
{
	PHP_EV_WATCHER_FACTORY(io, getThis());
}
/* }}} */

/* {{{ proto EvTimer EvLoop::timer(double after, double repeat, EvLoop loop, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvLoop, timer)
{
	PHP_EV_WATCHER_FACTORY(timer, getThis());
}
/* }}} */

#if EV_PERIODIC_ENABLE
/* {{{ proto EvPeriodic EvLoop::periodic(double offset, double interval, callable reschedule_cb, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvLoop, periodic)
{
	PHP_EV_WATCHER_FACTORY(periodic, getThis());
}
/* }}} */
#endif

#if EV_SIGNAL_ENABLE
/* {{{ proto EvSignal EvLoop::signal(int signum, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvLoop, signal)
{
	PHP_EV_WATCHER_FACTORY(signal, getThis());
}
/* }}} */
#endif

#if EV_CHILD_ENABLE
/* {{{ proto EvChild EvLoop::child(int pid, bool trace, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvLoop, child)
{
	PHP_EV_WATCHER_FACTORY(child, getThis());
}
/* }}} */
#endif

#if EV_STAT_ENABLE
/* {{{ proto EvStat EvLoop::stat(string path, double interval, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvLoop, stat)
{
	PHP_EV_WATCHER_FACTORY(stat, getThis());
}
/* }}} */
#endif

#if EV_IDLE_ENABLE
/* {{{ proto EvIdle EvLoop::idle(callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvLoop, idle)
{
	PHP_EV_WATCHER_FACTORY(idle, getThis());
}
/* }}} */
#endif

#if EV_CHECK_ENABLE
/* {{{ proto EvCheck EvLoop::check() */
PHP_METHOD(EvLoop, check)
{
	PHP_EV_WATCHER_FACTORY(check, getThis());
}
/* }}} */
#endif

#if EV_PREPARE_ENABLE
/* {{{ proto EvPrepare EvLoop::prepare() */
PHP_METHOD(EvLoop, prepare)
{
	PHP_EV_WATCHER_FACTORY(prepare, getThis());
}
/* }}} */
#endif

#if EV_EMBED_ENABLE
/* {{{ proto EvEmbed EvLoop::embed() */
PHP_METHOD(EvLoop, embed)
{
	PHP_EV_WATCHER_FACTORY(embed, getThis());
}
/* }}} */
#endif

#if EV_FORK_ENABLE
/* {{{ proto EvFork EvLoop::fork() */
PHP_METHOD(EvLoop, fork)
{
	PHP_EV_WATCHER_FACTORY(fork, getThis());
}
/* }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
