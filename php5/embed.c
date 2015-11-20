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

/* {{{ php_ev_embed_object_ctor */
void php_ev_embed_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start)
{
	zval                  *self;
	php_ev_object         *o_self;
	php_ev_object         *o_loop;
	php_ev_object         *o_loop_other;
	ev_embed              *w;
	ev_loop               *loop_other_ptr;
	php_ev_embed          *embed_ptr;

	zval                  *loop_other;
	zval                  *data           = NULL;
	zend_fcall_info        fci            = empty_fcall_info;
	zend_fcall_info_cache  fcc            = empty_fcall_info_cache;
	long                   priority       = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|fz!l",
				&loop_other, ev_loop_class_entry_ptr, &fci, &fcc,
				&data, &priority) == FAILURE) {
		return;
	}

	o_loop_other   = (php_ev_object *) zend_object_store_get_object(loop_other TSRMLS_CC);
	loop_other_ptr = PHP_EV_LOOP_FETCH_FROM_OBJECT(o_loop_other);

	if (UNEXPECTED(!(ev_backend(loop_other_ptr) & ev_embeddable_backends()))) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
        		"Passed loop is not embeddable. Check out your backends.");
        return;
	}

	PHP_EV_ASSERT(loop_other_ptr);

	if (ctor) {
		self = getThis();
	} else {
		PHP_EV_INIT_CLASS_OBJECT(return_value, ev_embed_class_entry_ptr);
		self = return_value; 
	}

	if (!loop) {
		loop = php_ev_default_loop(TSRMLS_C);
	}

	embed_ptr = (php_ev_embed *) emalloc(sizeof(php_ev_embed));
	memset(embed_ptr, 0, sizeof(php_ev_embed));

	w = &embed_ptr->embed;

	o_self = (php_ev_object *) zend_object_store_get_object(self TSRMLS_CC);
	o_loop = (php_ev_object *) zend_object_store_get_object(loop TSRMLS_CC);

	php_ev_set_watcher((ev_watcher *)w, sizeof(ev_embed), self,
			PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(o_loop),
			&fci, &fcc, data, priority TSRMLS_CC);

	w->type = EV_EMBED;

	ev_embed_set(w, PHP_EV_LOOP_FETCH_FROM_OBJECT(o_loop_other));

	o_self->ptr = (void *) embed_ptr;

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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
				&loop_other) == FAILURE) {
		return;
	}

	o_loop_other   = (php_ev_object *) zend_object_store_get_object(loop_other TSRMLS_CC);
	loop_other_ptr = PHP_EV_LOOP_FETCH_FROM_OBJECT(o_loop_other);

	if (!(ev_backend(loop_other_ptr) & ev_embeddable_backends())) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
        		"Passed loop is not embeddable. Check out your backends.");
        return;
	}

	w  = (ev_embed *) PHP_EV_WATCHER_FETCH_FROM_THIS();

	PHP_EV_ASSERT(PHP_EV_LOOP_FETCH_FROM_OBJECT(o_loop_other));
	PHP_EV_WATCHER_RESET(ev_embed, w, (w, PHP_EV_LOOP_FETCH_FROM_OBJECT(o_loop_other)));
}
/* }}} */

/* {{{ proto void EvEmbed::sweep(void) */
PHP_METHOD(EvEmbed, sweep)
{
	ev_embed *w;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	w = (ev_embed *) PHP_EV_WATCHER_FETCH_FROM_THIS();

	ev_embed_sweep(php_ev_watcher_loop_ptr(w), w);
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
