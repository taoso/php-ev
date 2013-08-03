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
#include "watcher.h"
#include <fcntl.h>

#define PHP_EV_PROP_REQUIRE(x)  \
	do {                        \
		if (UNEXPECTED(!(x))) { \
			return FAILURE;     \
		}                       \
	} while (0);

static inline void php_ev_prop_write_zval(zval **ppz, zval *value)
{
#if 0
	if (!*ppz) {
		/* if we assign referenced variable, we should separate it */
		Z_ADDREF_P(value);
		if (PZVAL_IS_REF(value)) {
			SEPARATE_ZVAL(&value);
		}
		*ppz = value;
	} else if (PZVAL_IS_REF(*ppz)) {
		zval garbage = **ppz; /* old value should be destroyed */

		/* To check: can't *ppz be some system variable like error_zval here? */
		Z_TYPE_PP(ppz) = Z_TYPE_P(value);
		(*ppz)->value = value->value;
		if (Z_REFCOUNT_P(value) > 0) {
			zval_copy_ctor(*ppz);
		}
		zval_dtor(&garbage);
	} else {
		zval *garbage = *ppz;

		/* if we assign referenced variable, we should separate it */
		Z_ADDREF_P(value);
		if (PZVAL_IS_REF(value)) {
			SEPARATE_ZVAL(&value);
		}
		*ppz = value;
		zval_ptr_dtor(&garbage);
	}

#else

	if (!*ppz) {
		MAKE_STD_ZVAL(*ppz);
	}

	/* Make a copy of the zval, avoid direct binding to the address
	 * of value, since it breaks refcount in read_property()
	 * causing further leaks and memory access violations */
	REPLACE_ZVAL_VALUE(ppz, value, PZVAL_IS_REF((zval *)value));

#endif
}

static inline void php_ev_prop_read_zval(zval *pz, zval **retval)
{
	if (!pz) {
		ALLOC_INIT_ZVAL(*retval);
		return;
	}

	MAKE_STD_ZVAL(*retval);
	ZVAL_ZVAL(*retval, pz, 1, 0);
}


/* {{{ EvLoop property handlers */

/* {{{ ev_loop_prop_data_get_ptr_ptr */
static zval **ev_loop_prop_data_get_ptr_ptr(php_ev_object *obj TSRMLS_DC)
{
	if (!obj->ptr) return NULL;

	if (!PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj)->data) {
		MAKE_STD_ZVAL(PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj)->data);
	}
	return &(PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj)->data);
}
/* }}} */

/* {{{ ev_loop_prop_data_read  */
static int ev_loop_prop_data_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	zval *data = PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj)->data;

	php_ev_prop_read_zval(data, retval);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_data_write  */
static int ev_loop_prop_data_write(php_ev_object *obj, zval *value TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	php_ev_prop_write_zval(&(PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj))->data, value);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_backend_read */
static int ev_loop_prop_backend_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, ev_backend(PHP_EV_LOOP_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_is_default_loop_read */
static int ev_loop_prop_is_default_loop_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_BOOL(*retval, ev_is_default_loop(PHP_EV_LOOP_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_iteration_loop_read */
static int ev_loop_prop_iteration_loop_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, ev_iteration(PHP_EV_LOOP_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_pending_loop_read */
static int ev_loop_prop_pending_loop_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, ev_pending_count(PHP_EV_LOOP_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_io_interval_read */
static int ev_loop_prop_io_interval_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	php_ev_loop *loop_obj = PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_DOUBLE(*retval, loop_obj->io_collect_interval);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_io_interval_write */
static int ev_loop_prop_io_interval_write(php_ev_object *obj, zval *value TSRMLS_DC)
{
	php_ev_loop *loop_obj;

	PHP_EV_PROP_REQUIRE(obj->ptr);

	loop_obj = PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj);

	loop_obj->io_collect_interval = Z_DVAL_P(value);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_timeout_interval_read */
static int ev_loop_prop_timeout_interval_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	php_ev_loop *loop_obj = PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_DOUBLE(*retval, loop_obj->timeout_collect_interval);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_timeout_interval_write */
static int ev_loop_prop_timeout_interval_write(php_ev_object *obj, zval *value TSRMLS_DC)
{
	php_ev_loop *loop_obj;

	PHP_EV_PROP_REQUIRE(obj->ptr);

	loop_obj = PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj);

	loop_obj->timeout_collect_interval = Z_DVAL_P(value);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_loop_prop_depth_read */
static int ev_loop_prop_depth_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, ev_depth(PHP_EV_LOOP_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* }}} */

/* {{{ EvWatcher property handlers */


/* {{{ ev_watcher_prop_is_active_read */
static int ev_watcher_prop_is_active_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_BOOL(*retval, ev_is_active(PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_watcher_prop_data_get_ptr_ptr */
static zval **ev_watcher_prop_data_get_ptr_ptr(php_ev_object *obj TSRMLS_DC)
{
	if (!obj->ptr) return NULL;

	zval *data = PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj)->data;
	if (!data) {
		MAKE_STD_ZVAL(PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj)->data);
	}

	return &PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj)->data;
}
/* }}} */

/* {{{ ev_watcher_prop_data_read  */
static int ev_watcher_prop_data_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	zval *data = PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj)->data;

	php_ev_prop_read_zval(data, retval);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_watcher_prop_data_write */
static int ev_watcher_prop_data_write(php_ev_object *obj, zval *value TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	zval **data = &(PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj))->data;
	php_ev_prop_write_zval(data, value);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_watcher_prop_is_pending_read */
static int ev_watcher_prop_is_pending_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_BOOL(*retval, ev_is_pending(PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_watcher_prop_priority_read */
static int ev_watcher_prop_priority_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);
	
	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, ev_priority(PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj)));

	return SUCCESS;
}
/* }}} */

/* {{{ ev_watcher_prop_priority_write */
static int ev_watcher_prop_priority_write(php_ev_object *obj, zval *value TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	long priority;
	ev_watcher *watcher = PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	int active = ev_is_active(watcher);

    if (active) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
        		"Modifying active watcher. Restarting the watcher internally.");
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

	if (active) {
		php_ev_stop_watcher(watcher TSRMLS_CC);
	}

	php_ev_set_watcher_priority(watcher, priority);

	if (active) {
		php_ev_start_watcher(watcher TSRMLS_CC);
	}

	return SUCCESS;
}
/* }}} */

/* }}} */

/* {{{ EvIo property handlers */

/* {{{ ev_io_prop_fd_read  */
static int ev_io_prop_fd_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	ev_io *io_watcher = (ev_io *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	if (io_watcher->fd <= 0 || fcntl(io_watcher->fd, F_GETFD) == -1) {
		// Invalid fd
		ALLOC_INIT_ZVAL(*retval); // NULL
		return SUCCESS;
	}

	php_stream *stream = php_stream_fopen_from_fd(io_watcher->fd, "r", NULL);

	if (stream) {
		MAKE_STD_ZVAL(*retval);
		php_stream_to_zval(stream, *retval);
	} else {
		ALLOC_INIT_ZVAL(*retval); // NULL
	}

	return SUCCESS;
}
/* }}} */

/* {{{ ev_io_prop_events_read */
static int ev_io_prop_events_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	ev_io *io_watcher = (ev_io *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, io_watcher->events);

	return SUCCESS;
}
/* }}} */

/* }}} */

/* {{{ EvTimer property handlers */

/* {{{ ev_timer_prop_repeat_read */
static int ev_timer_prop_repeat_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	ev_timer *timer_watcher = (ev_timer *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_DOUBLE(*retval, timer_watcher->repeat);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_timer_prop_repeat_write */
static int ev_timer_prop_repeat_write(php_ev_object *obj, zval *value TSRMLS_DC)
{
	ev_timer *timer_watcher;
	double    repeat;

	timer_watcher = (ev_timer *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	PHP_EV_PROP_REQUIRE(obj->ptr);

	repeat = Z_DVAL_P(value);

	PHP_EV_CHECK_REPEAT_RET(repeat, FAILURE);

	timer_watcher->repeat = (ev_tstamp) repeat;

	return SUCCESS;
}
/* }}} */

/* {{{ ev_timer_prop_remaining_read */
static int ev_timer_prop_remaining_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	ev_timer *timer_watcher = (ev_timer *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_DOUBLE(*retval, ev_timer_remaining(
				php_ev_watcher_loop_ptr(timer_watcher), timer_watcher));

	return SUCCESS;
}
/* }}} */

/* }}} */

#if EV_PERIODIC_ENABLE
/* {{{ EvPeriodic property handlers */

/* {{{ ev_periodic_prop_offset_read */
static int ev_periodic_prop_offset_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	ev_periodic *w = (ev_periodic *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_DOUBLE(*retval, w->offset);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_periodic_prop_offset_write */
static int ev_periodic_prop_offset_write(php_ev_object *obj, zval *value TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	ev_periodic *w = (ev_periodic *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	w->offset = (ev_tstamp) Z_DVAL_P(value);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_periodic_prop_interval_read */
static int ev_periodic_prop_interval_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	ev_periodic *w = (ev_periodic *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_DOUBLE(*retval, w->interval);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_periodic_prop_interval_write*/
static int ev_periodic_prop_interval_write(php_ev_object *obj, zval *value TSRMLS_DC)
{
	double interval;

	PHP_EV_PROP_REQUIRE(obj->ptr);

	ev_periodic *w = (ev_periodic *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);
	interval       = (ev_tstamp) Z_DVAL_P(value);

	PHP_EV_CHECK_REPEAT_RET(interval, FAILURE);

	w->interval = interval;

	return SUCCESS;
}
/* }}} */

/* }}} */
#endif

#if EV_SIGNAL_ENABLE
/* {{{ EvSignal property handlers */

/* {{{ ev_signal_prop_signum_read */
static int ev_signal_prop_signum_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	ev_signal *signal_watcher = (ev_signal *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, signal_watcher->signum);

	return SUCCESS;
}
/* }}} */

/* }}} */
#endif

#if EV_CHILD_ENABLE
/* {{{ EvChild property handlers */

/* {{{ ev_child_prop_pid_read */
static int ev_child_prop_pid_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	ev_child *child_watcher = (ev_child *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, child_watcher->pid);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_child_prop_rpid_read */
static int ev_child_prop_rpid_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	ev_child *child_watcher = (ev_child *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, child_watcher->rpid);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_child_prop_rstatus_read */
static int ev_child_prop_rstatus_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	ev_child *child_watcher = (ev_child *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_LONG(*retval, child_watcher->rstatus);

	return SUCCESS;
}
/* }}} */

/* }}} */
#endif

#if EV_STAT_ENABLE
/* {{{ EvStat property handlers */

/* {{{ ev_stat_prop_path_read */
static int ev_stat_prop_path_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	php_ev_stat *stat_ptr = (php_ev_stat *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_STRING(*retval, stat_ptr->path, 1);

	return SUCCESS;
}
/* }}} */

/* {{{ ev_stat_prop_interval_read */
static int ev_stat_prop_interval_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	ev_stat *stat_watcher = (ev_stat *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	MAKE_STD_ZVAL(*retval);
	ZVAL_DOUBLE(*retval, (double) stat_watcher->interval);

	return SUCCESS;
}
/* }}} */

/* }}} */
#endif

#if EV_EMBED_ENABLE
/* {{{ EvEmbed property handlers */

/* {{{ ev_embed_prop_other_read */
static int ev_embed_prop_other_read(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	PHP_EV_PROP_REQUIRE(obj->ptr);

	php_ev_embed *embed_ptr = (php_ev_embed *) PHP_EV_WATCHER_FETCH_FROM_OBJECT(obj);

	php_ev_prop_read_zval(embed_ptr->other, retval);

	return SUCCESS;
}
/* }}} */

/* }}} */
#endif

/* {{{ ev_loop_property_entries[] */
const php_ev_property_entry ev_loop_property_entries[] = {
	{"data",             sizeof("data")             - 1, ev_loop_prop_data_read,             ev_loop_prop_data_write,             ev_loop_prop_data_get_ptr_ptr},
	{"backend",          sizeof("backend")          - 1, ev_loop_prop_backend_read,          NULL,                                NULL},
	{"is_default_loop",  sizeof("is_default_loop")  - 1, ev_loop_prop_is_default_loop_read,  NULL,                                NULL},
	{"iteration",        sizeof("iteration")        - 1, ev_loop_prop_iteration_loop_read,   NULL,                                NULL},
	{"pending",          sizeof("pending")          - 1, ev_loop_prop_pending_loop_read,     NULL,                                NULL},
	{"io_interval",      sizeof("io_interval")      - 1, ev_loop_prop_io_interval_read,      ev_loop_prop_io_interval_write,      NULL},
	{"timeout_interval", sizeof("timeout_interval") - 1, ev_loop_prop_timeout_interval_read, ev_loop_prop_timeout_interval_write, NULL},
	{"depth",            sizeof("depth")            - 1, ev_loop_prop_depth_read,            NULL,                                NULL},
    {NULL, 0, NULL, NULL, NULL}
};
/* }}} */

/* {{{ ev_loop_property_entry_info[] */
const zend_property_info ev_loop_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "data",             sizeof("data")             - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "backend",          sizeof("backend")          - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "is_default_loop",  sizeof("is_default_loop")  - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "iteration",        sizeof("iteration")        - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "pending",          sizeof("pending")          - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "io_interval",      sizeof("io_interval")      - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "timeout_interval", sizeof("timeout_interval") - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "depth",            sizeof("depth")            - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
/* }}} */

/* {{{ ev_watcher_property_entries[] */
const php_ev_property_entry ev_watcher_property_entries[] = {
	{"is_active",  sizeof("is_active")  - 1, ev_watcher_prop_is_active_read,  NULL,                           NULL},
	{"data",       sizeof("data")       - 1, ev_watcher_prop_data_read,       ev_watcher_prop_data_write,     ev_watcher_prop_data_get_ptr_ptr},
	{"is_pending", sizeof("is_pending") - 1, ev_watcher_prop_is_pending_read, NULL,                           NULL},
	{"priority",   sizeof("priority")   - 1, ev_watcher_prop_priority_read,   ev_watcher_prop_priority_write, NULL},
	{NULL, 0, NULL, NULL, NULL}
};
/* }}} */

/* {{{ ev_watcher_property_entry_info[] */
const zend_property_info ev_watcher_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "is_active",  sizeof("is_active")  - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "data",       sizeof("data")       - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "is_pending", sizeof("is_pending") - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "priority",   sizeof("priority")   - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL}
};
/* }}} */

/* {{{ ev_io_property_entries[] */
const php_ev_property_entry ev_io_property_entries[] = {
	{"fd",     sizeof("fd")     - 1, ev_io_prop_fd_read,     NULL, NULL},
	{"events", sizeof("events") - 1, ev_io_prop_events_read, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}
};
/* }}} */

/* {{{ ev_io_property_entry_info[] */
const zend_property_info ev_io_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "fd",     sizeof("fd")     - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "events", sizeof("events") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL},
};
/* }}} */

/* {{{ ev_timer_property_entries[] */
const php_ev_property_entry ev_timer_property_entries[] = {
	{"repeat",    sizeof("repeat")    - 1, ev_timer_prop_repeat_read,    ev_timer_prop_repeat_write, NULL},
	{"remaining", sizeof("remaining") - 1, ev_timer_prop_remaining_read, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}
};
/* }}} */

/* {{{ ev_timer_property_entry_info[] */
const zend_property_info ev_timer_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "repeat",    sizeof("repeat")    - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "remaining", sizeof("remaining") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL},
};
/* }}} */

#if EV_PERIODIC_ENABLE
/* {{{ ev_periodic_property_entries[] */
const php_ev_property_entry ev_periodic_property_entries[] = {
	{"offset",   sizeof("offset")   - 1, ev_periodic_prop_offset_read,   ev_periodic_prop_offset_write, NULL},
	{"interval", sizeof("interval") - 1, ev_periodic_prop_interval_read, ev_periodic_prop_interval_write, NULL},
    {NULL, 0, NULL, NULL, NULL}
};
/* }}} */

/* {{{ ev_periodic_property_entry_info[] */
const zend_property_info ev_periodic_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "offset",   sizeof("offset")   - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "interval", sizeof("interval") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL},
};
/* }}} */
#endif

#if EV_SIGNAL_ENABLE
/* {{{ ev_signal_property_entries[] */
const php_ev_property_entry ev_signal_property_entries[] = {
	{"signum", sizeof("signum") - 1, ev_signal_prop_signum_read, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}
};
/* }}} */

/* {{{ ev_signal_property_entry_info[] */
const zend_property_info ev_signal_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "signum", sizeof("signum") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL},
};
/* }}} */
#endif

#if EV_CHILD_ENABLE
/* {{{ ev_child_property_entries[] */
const php_ev_property_entry ev_child_property_entries[] = {
	{"pid",     sizeof("pid")     - 1, ev_child_prop_pid_read,     NULL, NULL},
	{"rpid",    sizeof("rpid")    - 1, ev_child_prop_rpid_read,    NULL, NULL},
	{"rstatus", sizeof("rstatus") - 1, ev_child_prop_rstatus_read, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}
};
/* }}} */

/* {{{ ev_child_property_entry_info[] */
const zend_property_info ev_child_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "pid",     sizeof("pid")     - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "rpid",    sizeof("rpid")    - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "rstatus", sizeof("rstatus") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL},
};
/* }}} */
#endif

#if EV_STAT_ENABLE
/* {{{ ev_stat_property_entries[] */
const php_ev_property_entry ev_stat_property_entries[] = {
	{"path",     sizeof("path")     - 1, ev_stat_prop_path_read,     NULL, NULL},
	{"interval", sizeof("interval") - 1, ev_stat_prop_interval_read, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}
};
/* }}} */

/* {{{ ev_stat_property_entry_info[] */
const zend_property_info ev_stat_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "path",     sizeof("path")     - 1, -1, 0, NULL, 0, NULL},
	{ZEND_ACC_PUBLIC, "interval", sizeof("interval") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL},
};
/* }}} */
#endif

#if EV_EMBED_ENABLE
/* {{{ ev_embed_property_entries[] */
const php_ev_property_entry ev_embed_property_entries[] = {
	{"other", sizeof("other") - 1, ev_embed_prop_other_read, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL}
};
/* }}} */

/* {{{ ev_embed_property_entry_info[] */
const zend_property_info ev_embed_property_entry_info[] = {
	{ZEND_ACC_PUBLIC, "embed", sizeof("embed") - 1, -1, 0, NULL, 0, NULL},
	{0, NULL, 0, -1, 0, NULL, 0, NULL},
};
/* }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
