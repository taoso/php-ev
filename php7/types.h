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

#ifndef PHP_EV_TYPES_H
#define PHP_EV_TYPES_H

struct ev_watcher;

typedef struct _php_ev_func_info {
	zend_function    *func_ptr;
	zend_class_entry *ce;
	zval              obj;
	zval              closure;
} php_ev_func_info;

/* php_ev_object represents Ev* class object */
typedef struct _php_ev_object {
	void        *ptr; /* Pointer to ev_watcher, php_ev_loop or php_ev_periodic */
	HashTable   *prop_handler;
	zend_object  zo;
} php_ev_object;

static zend_always_inline php_ev_object * php_ev_object_fetch_object(zend_object *obj) {
      return (EXPECTED(obj) ? (php_ev_object *)((char *)obj - XtOffsetOf(php_ev_object, zo)) : NULL);
}

#define Z_EV_OBJECT_P(zv) (EXPECTED(zv) ? php_ev_object_fetch_object(Z_OBJ_P(zv)) : NULL)

/* php_ev_loop pointer is stored in php_ev_object.ptr struct member */
typedef struct _php_ev_loop {
	struct ev_loop    *loop;
	zval               data;                       /* User custom data attached to event loop                  */
	double             io_collect_interval;        /* If > 0, ev_io_collect_interval is called internally      */
	double             timeout_collect_interval;   /* If > 0, ev_timeout_collect_interval is called internally */
	struct ev_watcher *w;                          /* Head of linked list                                      */
} php_ev_loop;

/* Property handlers */
typedef zval *(*php_ev_read_t)(php_ev_object *obj, zval *retval);
typedef int (*php_ev_write_t)(php_ev_object *obj, zval *newval);
typedef zval *(*php_ev_get_prop_ptr_ptr_t)(php_ev_object *obj);

/* Property of an Ev* class */
typedef struct _php_ev_property_entry {
	const char                *name;
	size_t                     name_length;
	php_ev_read_t              read_func;
	php_ev_write_t             write_func;
	php_ev_get_prop_ptr_ptr_t  get_ptr_ptr_func;
} php_ev_property_entry;

typedef struct _php_ev_prop_handler {
	zend_string               *name;
	php_ev_read_t              read_func;
	php_ev_write_t             write_func;
	php_ev_get_prop_ptr_ptr_t  get_ptr_ptr_func;
} php_ev_prop_handler;

typedef struct _php_ev_object_lookup {
	php_ev_object *obj_ptr;
} php_ev_object_lookup;


#endif /* PHP_EV_TYPES_H */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
