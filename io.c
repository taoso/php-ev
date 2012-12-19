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
#include "util.h"
#include "watcher.h"

/* {{{ php_ev_io_object_ctor */
void php_ev_io_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop)
{
	zval                  *self       = NULL;
	zval                  *z_fd;
	php_ev_object         *o_self;
	php_ev_object         *o_loop;
	ev_io                 *io_watcher;
	php_socket_t           fd;

	zval                  *data       = NULL;
	zend_fcall_info        fci        = empty_fcall_info;
	zend_fcall_info_cache  fcc        = empty_fcall_info_cache;
	long                   priority   = 0;
	long                   events;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zlf|z!l",
				&z_fd, &events, &fci, &fcc,
				&data, &priority) == FAILURE) {
		return;
	}

	if (events & ~(EV_READ | EV_WRITE)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid events mask");
		return;
	}

	fd = php_ev_zval_to_fd(&z_fd TSRMLS_CC);
	if (fd < 0) {
		/* php_ev_zval_to_fd reports errors if necessary */
		return;
	}

	/* If loop is NULL, then we're in __construct() */
	if (loop) {
		PHP_EV_INIT_CLASS_OBJECT(return_value, ev_io_class_entry_ptr);

		PHP_EV_ASSERT((self == NULL));
		self = return_value; 
	} else {
		loop = php_ev_default_loop(TSRMLS_C);
		self = getThis();
	}

	o_self     = (php_ev_object *) zend_object_store_get_object(self TSRMLS_CC);
	o_loop     = (php_ev_object *) zend_object_store_get_object(loop TSRMLS_CC);
	io_watcher = (ev_io *) php_ev_new_watcher(sizeof(ev_io), self,
			PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(o_loop),
			&fci, &fcc, data, priority TSRMLS_CC);

	io_watcher->type = EV_IO;
	
	ev_io_set(io_watcher, fd, events);

	o_self->ptr = (void *) io_watcher;
}
/* }}} */


/* {{{ proto EvIo::__construct(mixed fd, int events, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvIo, __construct)
{
	php_ev_io_object_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, NULL);
}
/* }}} */

/* {{{ proto void EvIo::set(resource fd, int events) */
PHP_METHOD(EvIo, set)
{
	zval         *z_fd;
	ev_io        *io_watcher;
	long          events;
	php_socket_t  fd;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl",
				&z_fd, &events) == FAILURE) {
		return;
	}

	if (events & ~(EV_READ | EV_WRITE)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid events mask");
		return;
	}

#if 0
	php_stream *fd_stream;
	php_stream_from_zval_no_verify(fd_stream, &z_fd);
	if (fd_stream == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed obtaining fd");
		return;
	}
#endif
	fd = php_ev_zval_to_fd(&z_fd TSRMLS_CC);

	io_watcher = (ev_io *) PHP_EV_WATCHER_FETCH_FROM_THIS();

#if 0
	PHP_EV_WATCHER_RESET(ev_io, io_watcher, (io_watcher, Z_LVAL_P(z_fd), events));
#endif
	PHP_EV_WATCHER_RESET(ev_io, io_watcher, (io_watcher, fd, events));
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
