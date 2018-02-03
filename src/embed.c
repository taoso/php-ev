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

/* {{{ php_ev_embed_object_ctor */
void php_ev_embed_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *zloop, zend_bool ctor, zend_bool start)
{
	zval          *self;
	zval          *loop_other;
	zval          *callback       = NULL;
	zval          *data           = NULL;
	php_ev_object *o_self;
	php_ev_object *o_loop_other;
	ev_embed      *w;
	ev_loop       *loop_other_ptr;
	php_ev_embed  *embed_ptr;
	zend_long      priority       = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O|z!z!l",
				&loop_other, ev_loop_class_entry_ptr,
				&callback, &data, &priority) == FAILURE) {
		return;
	}

	o_loop_other   = Z_EV_OBJECT_P(loop_other);
	loop_other_ptr = PHP_EV_LOOP_FETCH_FROM_OBJECT(o_loop_other);
	PHP_EV_ASSERT(loop_other_ptr);

	if (UNEXPECTED(!(ev_backend(loop_other_ptr) & ev_embeddable_backends()))) {
		php_error_docref(NULL, E_WARNING,
				"Passed loop is not embeddable. Check out your backends.");
		return;
	}

	if (ctor) {
		self = getThis();
	} else {
		object_init_ex(return_value, ev_embed_class_entry_ptr);
		self = return_value;
	}

	if (!zloop) {
		zloop = php_ev_default_loop();
	}

	embed_ptr = (php_ev_embed *)ecalloc(1, sizeof(php_ev_embed));
	if (UNEXPECTED(embed_ptr == NULL)) {
		php_error_docref(NULL, E_ERROR, "Failed to allocate memory: php_ev_embed");
		return;
	}
	w = &embed_ptr->embed;

	if (php_ev_set_watcher((ev_watcher *)w, EV_EMBED, self,
				zloop, callback, data, priority) == FAILURE) {
		efree(embed_ptr);
		zend_throw_exception_ex(zend_ce_exception, 0, "Watcher configuration failed");
		return;
	}
	ev_embed_set(w, PHP_EV_LOOP_FETCH_FROM_OBJECT(o_loop_other));

	o_self = Z_EV_OBJECT_P(self);
	o_self->ptr = (void *)embed_ptr;

	if (start) {
		PHP_EV_WATCHER_START(ev_embed, w);
	}
}
/* }}} */


/* {{{ proto EvEmbed::__construct(EvLoop other, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvEmbed, __construct)
{
	PHP_EV_WATCHER_CTOR(embed, NULL);
}
/* }}} */

/* {{{ proto EvEmbed::createStopped(EvLoop other, callable callback[, mixed data = NULL[, int priority = 0]]) */
PHP_METHOD(EvEmbed, createStopped)
{
	PHP_EV_WATCHER_FACTORY_NS(embed, NULL);
}
/* }}} */

/* {{{ proto void EvEmbed::set(EvLoop other) */
PHP_METHOD(EvEmbed, set)
{
	zval          *loop_other;
	ev_embed      *w;
	php_ev_object *o_loop_other;
	ev_loop       *loop_other_ptr;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &loop_other) == FAILURE) {
		return;
	}

	o_loop_other   = Z_EV_OBJECT_P(loop_other);
	loop_other_ptr = PHP_EV_LOOP_FETCH_FROM_OBJECT(o_loop_other);

	if (!(ev_backend(loop_other_ptr) & ev_embeddable_backends())) {
		php_error_docref(NULL, E_ERROR,
				"Passed loop is not embeddable. Check out your backends.");
		return;
	}

	w  = (ev_embed *)PHP_EV_WATCHER_FETCH_FROM_THIS();

	PHP_EV_ASSERT(PHP_EV_LOOP_FETCH_FROM_OBJECT(o_loop_other));
	PHP_EV_WATCHER_RESET(ev_embed, w, (w, PHP_EV_LOOP_FETCH_FROM_OBJECT(o_loop_other)));
}
/* }}} */

/* {{{ proto void EvEmbed::sweep(void) */
PHP_METHOD(EvEmbed, sweep)
{
	ev_embed *w;

	if (zend_parse_parameters_none() != FAILURE) {
		w = (ev_embed *) PHP_EV_WATCHER_FETCH_FROM_THIS();
		ev_embed_sweep(php_ev_watcher_loop_ptr((ev_watcher *)w), w);
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
