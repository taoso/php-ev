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
#include "util.h"
#include "watcher.h"

/* {{{ php_ev_io_object_ctor */
void php_ev_io_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start)
{
	zval          *self;
	zval          *callback;
	zval          *data     = NULL;
	zval          *z_fd;
	php_ev_object *o_self;
	ev_io         *w;
	php_socket_t   fd;
	zend_long      priority = 0;
	zend_long      events;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "zlz|z!l",
				&z_fd, &events,
				&callback, &data, &priority) == FAILURE) {
		return;
	}

	if (events & ~(EV_READ | EV_WRITE)) {
		php_error_docref(NULL, E_WARNING, "Invalid events mask");
		return;
	}

	fd = php_ev_zval_to_fd(z_fd);
	if (fd < 0) {
		/* php_ev_zval_to_fd reports errors if necessary */
		return;
	}

	if (ctor) {
		self = getThis();
	} else {
		object_init_ex(return_value, ev_io_class_entry_ptr);
		self = return_value;
	}

	if (!loop) {
		loop = php_ev_default_loop();
	}

	PHP_EV_NEW_WATCHER(w, ev_io, EV_IO, self, loop, callback, data, priority);
	ev_io_set(w, fd, events);

	o_self = Z_EV_OBJECT_P(self);
	o_self->ptr = (void *) w;

	zend_string *name = zend_string_init("fd", sizeof("fd")-1, 0);
	zend_update_property_ex(ev_io_class_entry_ptr, self, name, z_fd);

	if (start) {
		PHP_EV_WATCHER_START(ev_io, w);
	}
}
/* }}} */


/* {{{ proto EvIo::__construct(mixed fd, int events, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvIo, __construct)
{
	PHP_EV_WATCHER_CTOR(io, NULL);
}
/* }}} */

/* {{{ proto EvIo::createStopped(mixed fd, int events, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvIo, createStopped)
{
	PHP_EV_WATCHER_FACTORY_NS(io, NULL);
}
/* }}} */

/* {{{ proto void EvIo::set(resource fd, int events) */
PHP_METHOD(EvIo, set)
{
	zval         *z_fd;
	ev_io        *io_watcher;
	zend_long    events;
	php_socket_t fd;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "zl",
				&z_fd, &events) == FAILURE) {
		return;
	}

	if (events & ~(EV_READ | EV_WRITE)) {
		php_error_docref(NULL, E_WARNING, "Invalid events mask");
		return;
	}

	fd = php_ev_zval_to_fd(z_fd);

	io_watcher = (ev_io *)PHP_EV_WATCHER_FETCH_FROM_THIS();

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
