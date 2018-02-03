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

#ifndef PHP_EV_MACROS_H
#define PHP_EV_MACROS_H

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

#ifndef MAX
# define MAX(a, b)  (((a)>(b))?(a):(b))
#endif

#ifndef MIN
# define MIN(a, b)  (((a)<(b))?(a):(b))
#endif

#ifdef PHP_EV_DEBUG
# define PHP_EV_ASSERT(x) assert(x)
#else
# define PHP_EV_ASSERT(x)
#endif

#define MyG(v) ZEND_MODULE_GLOBALS_ACCESSOR(ev, v)

#define PHP_EV_EFREE(x) do { \
	if ((x)) { efree((x)); (x) = NULL; } \
} while (0)

#define PHP_EV_REGISTER_LONG_CONSTANT(name)                           \
	REGISTER_LONG_CONSTANT(#name, name, CONST_CS | CONST_PERSISTENT)

#define REGISTER_EV_CLASS_CONST_LONG(const_name, value)               \
	zend_declare_class_constant_long(ev_class_entry_ptr, #const_name, \
			sizeof(#const_name) - 1, (zend_long)value)

#define PHP_EV_REGISTER_CLASS_ENTRY(name, ce, ce_functions) \
{                                                           \
	zend_class_entry tmp_ce;                                \
	INIT_CLASS_ENTRY(tmp_ce, name, ce_functions);           \
	tmp_ce.create_object = php_ev_object_create;            \
	ce = zend_register_internal_class(&tmp_ce);             \
}

#define PHP_EV_REGISTER_CLASS_ENTRY_EX(name, ce, ce_functions, parent_ce) \
{                                                                         \
	zend_class_entry tmp_ce;                                              \
	INIT_CLASS_ENTRY_EX(tmp_ce, name, sizeof(name) - 1, ce_functions);    \
	tmp_ce.create_object = parent_ce->create_object;                      \
	ce = zend_register_internal_class_ex(&tmp_ce, parent_ce);             \
}

#define PHP_EV_ADD_CLASS_PROPERTIES(a, b)                                           \
{                                                                                   \
	int i = 0;                                                                      \
	while (b[i].name != NULL) {                                                     \
		php_ev_add_property((a), (b)[i].name, (b)[i].name_length,                   \
				(php_ev_read_t)(b)[i].read_func, (php_ev_write_t)(b)[i].write_func, \
				(php_ev_get_prop_ptr_ptr_t)(b)[i].get_ptr_ptr_func);                \
		i++;                                                                        \
	}                                                                               \
}

#define PHP_EV_DECL_CLASS_PROPERTIES(a, b)                                                 \
{                                                                                          \
	int i = 0;                                                                             \
	while (b[i].name != NULL) {                                                            \
		zend_declare_property_null((a), (b)[i].name, (b)[i].name_length, ZEND_ACC_PUBLIC); \
		i++;                                                                               \
	}                                                                                      \
}

#define PHP_EV_DECL_PROP_NULL(ce, name, attr) \
	zend_declare_property_null(ce, #name, sizeof(#name) - 1, attr)

#define PHP_EV_CONSTRUCT_CHECK(ev_obj) do {                         \
	if (!ev_obj->ptr) {                                             \
		php_error_docref(NULL, E_ERROR, "Loop is not initialized"); \
		return;                                                     \
	}                                                               \
} while (0)

#define PHP_EV_FREE_FCALL_INFO(fci) do {   \
	if ((fci).size != 0) {                   \
		zval_ptr_dtor(&(fci).function_name); \
		if ((fci).object) {                  \
			OBJ_RELEASE((fci).object);       \
		}                                    \
		(fci).size = 0;                      \
	}                                        \
} while (0)

#define PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj) ((obj) ? ((obj)->ptr ? (php_ev_loop *)(obj)->ptr : NULL) : NULL)
#define PHP_EV_WATCHER_FETCH_FROM_OBJECT(o)       ((ev_watcher *) o->ptr)
#define PHP_EV_WATCHER_FETCH_FROM_THIS()          PHP_EV_WATCHER_FETCH_FROM_OBJECT(Z_EV_OBJECT_P(getThis()))
#define PHP_EV_LOOP_FETCH_FROM_OBJECT(obj)        ((obj) ? PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT((obj))->loop : NULL)

#define PHP_EV_CHECK_PENDING_WATCHER(w) do {                                 \
	if (ev_is_pending(w)) {                                                  \
		php_error_docref(NULL, E_ERROR, "Failed modifying pending watcher"); \
		return;                                                              \
	}                                                                        \
} while (0)

#define PHP_EV_EXIT_LOOP(__loop) ev_break((__loop), EVBREAK_ALL)

#define PHP_EV_REPEAT_RANGE_ERROR_STR(repeat) # repeat " value must be >= 0."
#define PHP_EV_IS_REPEAT_RANGE_VALID(x) ((x) >= 0.)
#define PHP_EV_CHECK_REPEAT(repeat) do {                                        \
	if (!PHP_EV_IS_REPEAT_RANGE_VALID(repeat)) {                                 \
		php_error_docref(NULL, E_ERROR, PHP_EV_REPEAT_RANGE_ERROR_STR(repeat)); \
		return;                                                                 \
	}                                                                           \
} while (0)
#define PHP_EV_CHECK_REPEAT_RET(repeat, ret) do {                               \
	if (!PHP_EV_IS_REPEAT_RANGE_VALID(repeat)) {                                 \
		php_error_docref(NULL, E_ERROR, PHP_EV_REPEAT_RANGE_ERROR_STR(repeat)); \
		return (ret);                                                           \
	}                                                                           \
} while (0)

#define PHP_EV_SIGNUM_ERROR_STR "Invalid signal value"
#define PHP_EV_IS_SIGNUM_VALID(x) ((x) >= 0)
#define PHP_EV_CHECK_SIGNUM(num) do {                             \
	if (PHP_EV_IS_SIGNUM_VALID(num)) {                            \
		php_error_docref(NULL, E_ERROR, PHP_EV_SIGNUM_ERROR_STR); \
		return;                                                   \
	}                                                             \
} while (0)

#endif /* PHP_EV_MACROS_H*/
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
