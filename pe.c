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

#include "embed.h"
#include "priv.h"
#include "watcher.h"


/* {{{ EvLoop property handlers */

/* {{{ ev_loop_prop_data_read  */
static int ev_loop_prop_data_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_ASSERT(obj->ptr);

	zval *data = PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj)->data;

	PHP_EV_PROP_ZVAL_READ(data);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_data_write  */
static int ev_loop_prop_data_write(php_ev_object *obj, zval *value TSRMLS_DC)
{
	PHP_EV_ASSERT(obj->ptr);

	zval **data_ptr_ptr = &(PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj))->data;

	PHP_EV_PROP_ZVAL_WRITE(data_ptr_ptr);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_backend_read */
static int ev_loop_prop_backend_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_ASSERT(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, ev_backend(PHP_EV_LOOP_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_is_default_loop_read */
static int ev_loop_prop_is_default_loop_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_ASSERT(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_BOOL(*retval, ev_is_default_loop(PHP_EV_LOOP_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_iteration_loop_read */
static int ev_loop_prop_iteration_loop_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_ASSERT(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, ev_iteration(PHP_EV_LOOP_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_pending_loop_read */
static int ev_loop_prop_pending_loop_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_ASSERT(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, ev_pending_count(PHP_EV_LOOP_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* }}} */

/* {{{ EvWatcher property handlers */

/* {{{ ev_watcher_prop_is_active_read */
static int ev_watcher_prop_is_active_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_ASSERT(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_BOOL(*retval, ev_is_active(PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_watcher_prop_data_read  */
static int ev_watcher_prop_data_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_ASSERT(obj->ptr);

	zval *data = PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj)->data;

	PHP_EV_PROP_ZVAL_READ(data);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_watcher_prop_data_write */
static int ev_watcher_prop_data_write(php_ev_object *obj, zval *value TSRMLS_DC)
{
	PHP_EV_ASSERT(obj->ptr);

	zval **data_ptr_ptr = &(PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj))->data;

	PHP_EV_PROP_ZVAL_WRITE(data_ptr_ptr);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_watcher_prop_is_pending_read */
static int ev_watcher_prop_is_pending_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_ASSERT(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_BOOL(*retval, ev_is_pending(PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_watcher_prop_priority_read */
static int ev_watcher_prop_priority_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_ASSERT(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, ev_priority(PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_watcher_prop_priority_write */
static int ev_watcher_prop_priority_write(php_ev_object *obj, zval *value TSRMLS_DC)
{
	PHP_EV_ASSERT(obj->ptr);

	long priority;
	ev_watcher *watcher = PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

    if (ev_is_active(watcher)) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
        		"Failed modifying active watcher");
        return FAILURE;
    }

	priority = Z_LVAL_P(value);
	if (priority < INT_MIN) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Priority value must be bigger than INT_MIN");
	    return FAILURE;
	}
	if (priority > INT_MAX) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Priority value must not exceed INT_MAX");
	    return FAILURE;
	}

	php_ev_set_watcher_priority(watcher, priority TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

/* }}} */


/* {{{ ev_loop_property_entries[] */
const php_ev_property_entry ev_loop_property_entries[] = {
	{"data",            sizeof("data")            - 1, ev_loop_prop_data_read,            ev_loop_prop_data_write},
	{"backend",         sizeof("backend")         - 1, ev_loop_prop_backend_read,         NULL},
	{"is_default_loop", sizeof("is_default_loop") - 1, ev_loop_prop_is_default_loop_read, NULL},
	{"iteration",       sizeof("iteration")       - 1, ev_loop_prop_iteration_loop_read,  NULL},
	{"pending",         sizeof("pending")         - 1, ev_loop_prop_pending_loop_read,    NULL},
    {NULL, 0, NULL, NULL}
};
/* }}} */

/* {{{ ev_loop_property_entry_info[] */
const zend_property_info ev_loop_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "data",            sizeof("data") - 1,            -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "backend",         sizeof("backend") - 1,         -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "is_default_loop", sizeof("is_default_loop") - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "iteration",       sizeof("iteration") - 1,       -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "pending",         sizeof("pending") - 1,         -1, 0, NULL, 0, NULL},
	{0,                NULL,             0,                             -1, 0, NULL, 0, NULL}
};
/* }}} */

/* {{{ ev_watcher_property_entries[] */
const php_ev_property_entry ev_watcher_property_entries[] = {
	{"is_active",  sizeof("is_active")  - 1, ev_watcher_prop_is_active_read,  NULL},
	{"data",       sizeof("data")       - 1, ev_watcher_prop_data_read,       ev_watcher_prop_data_write},
	{"is_pending", sizeof("is_pending") - 1, ev_watcher_prop_is_pending_read, NULL},
	{"priority",   sizeof("priority")   - 1, ev_watcher_prop_priority_read,   ev_watcher_prop_priority_write},
	{NULL, 0, NULL, NULL}
};
/* }}} */

/* {{{ ev_watcher_property_entry_info[] */
const zend_property_info ev_watcher_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "is_active",  sizeof("is_active")  - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "data",       sizeof("data")       - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "is_pending", sizeof("is_pending") - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "priority",   sizeof("priority")   - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "loop",       sizeof("loop")       - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
/* }}} */

/* {{{ ev_io_property_entries[] */
const php_ev_property_entry ev_io_property_entries[] = {
    {NULL, 0, NULL, NULL}
};
/* }}} */

/* {{{ ev_io_property_entry_info[] */
const zend_property_info ev_io_property_entry_info[] = {
	{0, NULL, 0, -1, 0, NULL, 0, NULL},
};
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
