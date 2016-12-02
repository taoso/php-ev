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

#include "php_ev.h"
#include "util.h"

#if HAVE_EV

ZEND_DECLARE_MODULE_GLOBALS(ev)
static PHP_GINIT_FUNCTION(ev);

zend_class_entry *ev_class_entry_ptr;
zend_class_entry *ev_loop_class_entry_ptr;
zend_class_entry *ev_watcher_class_entry_ptr;
zend_class_entry *ev_io_class_entry_ptr;
zend_class_entry *ev_timer_class_entry_ptr;
#if EV_PERIODIC_ENABLE
zend_class_entry *ev_periodic_class_entry_ptr;
#endif
#if EV_SIGNAL_ENABLE
zend_class_entry *ev_signal_class_entry_ptr;
#endif
#if EV_CHILD_ENABLE
zend_class_entry *ev_child_class_entry_ptr;
#endif
#if EV_STAT_ENABLE
zend_class_entry *ev_stat_class_entry_ptr;
#endif
#if EV_IDLE_ENABLE
zend_class_entry *ev_idle_class_entry_ptr;
#endif
#if EV_CHECK_ENABLE
zend_class_entry *ev_check_class_entry_ptr;
#endif
#if EV_PREPARE_ENABLE
zend_class_entry *ev_prepare_class_entry_ptr;
#endif
#if EV_EMBED_ENABLE
zend_class_entry *ev_embed_class_entry_ptr;
#endif
#if EV_FORK_ENABLE
zend_class_entry *ev_fork_class_entry_ptr;
#endif

static HashTable classes;
static HashTable php_ev_properties;
static HashTable php_ev_watcher_properties;
static HashTable php_ev_io_properties;
static HashTable php_ev_timer_properties;
#if EV_PERIODIC_ENABLE
static HashTable php_ev_periodic_properties;
#endif
#if EV_SIGNAL_ENABLE
static HashTable php_ev_signal_properties;
#endif
#if EV_CHILD_ENABLE
static HashTable php_ev_child_properties;
#endif
#if EV_STAT_ENABLE
static HashTable php_ev_stat_properties;
#endif
#if EV_EMBED_ENABLE
static HashTable php_ev_embed_properties;
#endif

static zend_object_handlers ev_object_handlers;
static zend_object_handlers ev_object_loop_handlers;
static zend_object_handlers ev_object_io_handlers;
static zend_object_handlers ev_object_timer_handlers;
#if EV_PERIODIC_ENABLE
static zend_object_handlers ev_object_periodic_handlers;
#endif
#if EV_SIGNAL_ENABLE
static zend_object_handlers ev_object_signal_handlers;
#endif
#if EV_CHILD_ENABLE
static zend_object_handlers ev_object_child_handlers;
#endif
#if EV_STAT_ENABLE
static zend_object_handlers ev_object_stat_handlers;
#endif
#if EV_IDLE_ENABLE
static zend_object_handlers ev_object_idle_handlers;
#endif
#if EV_CHECK_ENABLE
static zend_object_handlers ev_object_check_handlers;
#endif
#if EV_PREPARE_ENABLE
static zend_object_handlers ev_object_prepare_handlers;
#endif
#if EV_EMBED_ENABLE
static zend_object_handlers ev_object_embed_handlers;
#endif
#if EV_FORK_ENABLE
static zend_object_handlers ev_object_fork_handlers;
#endif

static const zend_module_dep ev_deps[] = {
	ZEND_MOD_OPTIONAL("sockets")
	{NULL, NULL, NULL}
};

/* {{{ ev_module_entry */
zend_module_entry ev_module_entry = {
#if ZEND_MODULE_API_NO >= 20050922
	STANDARD_MODULE_HEADER_EX,
	NULL,
	ev_deps,
#elif ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
    "ev",
    NULL, /* functions */
    PHP_MINIT(ev),
    PHP_MSHUTDOWN(ev),
    PHP_RINIT(ev),
    NULL, /*PHP_RSHUTDOWN(ev),*/
    PHP_MINFO(ev),
    PHP_EV_VERSION,
    PHP_MODULE_GLOBALS(ev),
    PHP_GINIT(ev),
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_EV
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(ev)
#endif

/* {{{ Private functions */

static void free_prop_handler(zval *el)
{
	pefree(Z_PTR_P(el), 1);
}

static void copy_prop_handler(zval *zv)
{
	php_ev_prop_handler *hnd = Z_PTR_P(zv);
	Z_PTR_P(zv) = malloc(sizeof(php_ev_prop_handler));
	memcpy(Z_PTR_P(zv), hnd, sizeof(php_ev_prop_handler));
}

/* {{{ php_ev_default_loop */
zval * php_ev_default_loop()
{
	zval *default_loop_ptr = &MyG(default_loop);

	if (!Z_ISUNDEF_P(default_loop_ptr)) {
		return default_loop_ptr;
	}


	php_ev_object  *ev_obj;
	struct ev_loop *loop   = ev_default_loop(EVFLAG_AUTO);

	if (!loop) {
		php_error_docref(NULL, E_ERROR,
				"Failed to instanciate default loop, "
				"bad $LIBEV_FLAGS in environment?");
		return NULL;
	}

	object_init_ex(default_loop_ptr, ev_loop_class_entry_ptr);
	ev_obj = Z_EV_OBJECT_P(default_loop_ptr);

	php_ev_loop *ptr = (php_ev_loop *)ecalloc(1, sizeof(php_ev_loop));
	ptr->loop = loop;
	ev_obj->ptr = (void *)ptr;

	ev_set_userdata(loop, (void *)default_loop_ptr);

	return default_loop_ptr;
}
/* }}} */

/* {{{ php_ev_prop_read_default */
static zval * php_ev_prop_read_default(php_ev_object *obj, zval *retval)
{
	php_error_docref(NULL, E_ERROR, "Cannot read property");
	return NULL;
}
/* }}} */

/* {{{ php_ev_prop_write_default */
static int php_ev_prop_write_default(php_ev_object *obj, zval *newval)
{
	php_error_docref(NULL, E_ERROR, "Cannot write property");
	return FAILURE;
}
/* }}} */

/* {{{ php_ev_add_property */
static void php_ev_add_property(HashTable *h, const char *name, size_t name_len, php_ev_read_t read_func, php_ev_write_t write_func, php_ev_get_prop_ptr_ptr_t get_ptr_ptr_func)
{
	php_ev_prop_handler p;

	p.name             = zend_string_init(name, name_len, 1);
	p.read_func        = (read_func) ? read_func : php_ev_prop_read_default;
	p.write_func       = (write_func) ? write_func: php_ev_prop_write_default;
	p.get_ptr_ptr_func = get_ptr_ptr_func;
	zend_hash_add_mem(h, p.name, &p, sizeof(php_ev_prop_handler));
	zend_string_release(p.name);
}
/* }}} */

/* {{{ php_ev_read_property */
static zval * php_ev_read_property(zval *object, zval *member, int type, void **cache_slot, zval *rv)
{
	zval                 tmp_member;
	zval                *retval;
	php_ev_object       *obj = Z_EV_OBJECT_P(object);
	php_ev_prop_handler *hnd = NULL;

	if (Z_TYPE_P(member) != IS_STRING) {
		ZVAL_COPY(&tmp_member, member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	if (obj->prop_handler != NULL) {
		hnd = zend_hash_find_ptr(obj->prop_handler, Z_STR_P(member));
	}

	if (hnd) {
		retval = hnd->read_func(obj, rv);
		if (retval == NULL) {
			retval = &EG(uninitialized_zval);
		}
	} else {
		zend_object_handlers *std_hnd = zend_get_std_object_handlers();
		retval = std_hnd->read_property(object, member, type, cache_slot, rv);
	}

	if (member == &tmp_member) {
		zval_dtor(&tmp_member);
	}

	return retval;
}
/* }}} */

/* {{{ php_ev_write_property */
static void php_ev_write_property(zval *object, zval *member, zval *value, void **cache_slot)
{
	zval                 tmp_member;
	php_ev_object       *obj;
	php_ev_prop_handler *hnd = NULL;

	if (Z_TYPE_P(member) != IS_STRING) {
		ZVAL_COPY(&tmp_member, member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	obj = Z_EV_OBJECT_P(object);

	if (obj->prop_handler != NULL) {
	    hnd = zend_hash_find_ptr(obj->prop_handler, Z_STR_P(member));
	}

	if (hnd) {
	    hnd->write_func(obj, value);
	} else {
	    zend_object_handlers *std_hnd = zend_get_std_object_handlers();
	    std_hnd->write_property(object, member, value, cache_slot);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}
}
/* }}} */

/* {{{ php_ev_has_property */
static int php_ev_has_property(zval *object, zval *member, int has_set_exists, void **cache_slot)
{
	php_ev_object       *obj = Z_EV_OBJECT_P(object);
	php_ev_prop_handler *p;
	int                 ret  = 0;

	if ((p = zend_hash_find_ptr(obj->prop_handler, Z_STR_P(member))) != NULL) {
		switch (has_set_exists) {
			case 2:
				ret = 1;
				break;
			case 1: {
						zval rv;
						zval *value = php_ev_read_property(object, member, BP_VAR_IS, cache_slot, &rv);
						if (value != &EG(uninitialized_zval)) {
							convert_to_boolean(value);
							ret = Z_TYPE_P(value) == IS_TRUE ? 1 : 0;
						}
						break;
					}
			case 0: {
					   zval rv;
					   zval *value = php_ev_read_property(object, member, BP_VAR_IS, cache_slot, &rv);
					   if (value != &EG(uninitialized_zval)) {
						   ret = Z_TYPE_P(value) != IS_NULL ? 1 : 0;
						   zval_ptr_dtor(value);
					   }
					   break;
				   }
			default:
				   php_error_docref(NULL, E_WARNING, "Invalid value for has_set_exists");
		}
	} else {
		zend_object_handlers *std_hnd = zend_get_std_object_handlers();
		ret = std_hnd->has_property(object, member, has_set_exists, cache_slot);
	}

	return ret;
} /* }}} */

/* {{{ php_ev_object_get_debug_info */
static HashTable * php_ev_object_get_debug_info(zval *object, int *is_temp)
{
	php_ev_object       *obj = Z_EV_OBJECT_P(object);
	HashTable           *props   = obj->prop_handler;
	HashTable           *retval;
	php_ev_prop_handler *entry;

	ALLOC_HASHTABLE(retval);
	ZEND_INIT_SYMTABLE_EX(retval, zend_hash_num_elements(props) + 1, 0);

	ZEND_HASH_FOREACH_PTR(props, entry) {
		zval rv, member;
		zval *value;
		ZVAL_STR(&member, entry->name);
		value = php_ev_read_property(object, &member, BP_VAR_IS, 0, &rv);
		if (value != &EG(uninitialized_zval)) {
			zend_hash_add(retval, Z_STR(member), value);
		}
	} ZEND_HASH_FOREACH_END();

	*is_temp = 1;

	return retval;
}
/* }}} */

/* {{{ php_ev_get_properties */
static HashTable * php_ev_get_properties(zval *object)
{
	php_ev_object       *obj   = Z_EV_OBJECT_P(object);
	HashTable           *props = zend_std_get_properties(object);
	php_ev_prop_handler *hnd;
	zend_string         *key;

	if (obj->prop_handler == NULL) {
		return NULL;
	}

	ZEND_HASH_FOREACH_STR_KEY_PTR(obj->prop_handler, key, hnd) {
		zval zret;
		if (hnd->read_func && hnd->read_func(obj, &zret)) {
			zend_hash_update(props, key, &zret);
		}
	} ZEND_HASH_FOREACH_END();

	return props;
}
/* }}} */

/* {{{ php_ev_get_gc */
static HashTable * php_ev_get_gc(zval *object, zval **gc_data, int *gc_count)
{
#if 0
	php_ev_object *ev_obj = Z_EV_OBJECT_P(object);
	ev_watcher *w;

	if (EXPECTED(ev_obj)) {
		w = (ev_watcher *)ev_obj->ptr;
		PHP_EV_ASSERT(w);

		*gc_data = &php_ev_watcher_self(w);
		*gc_count = 1;
	} else {
		*gc_data = NULL;
		*gc_count = 0;
	}
#else
	*gc_data = NULL;
	*gc_count = 0;
#endif

	return zend_std_get_properties(object);
}
/* }}} */

static HashTable * php_ev_loop_get_gc(zval *object, zval **gc_data, int *gc_count)/*{{{*/
{
#if 0
	php_ev_loop *loop_ptr;
	php_ev_object *ev_obj = Z_EV_OBJECT_P(object);

	PHP_EV_ASSERT(ev_obj);
	if (EXPECTED(ev_obj && ev_obj->ptr)) {
		loop_ptr = (php_ev_loop *)ev_obj->ptr;
		if (loop_ptr->loop) {
			*gc_data = (zval *)ev_userdata(loop_ptr->loop);
			*gc_count = 1;
		} else {
			*gc_data = NULL;
			*gc_count = 0;
		}
	} else {
		*gc_data = NULL;
		*gc_count = 0;
	}
#else
	*gc_data = NULL;
	*gc_count = 0;
#endif

	return zend_std_get_properties(object);
}
/*}}}*/

/* {{{ php_ev_get_property_ptr_ptr */
static zval * php_ev_get_property_ptr_ptr(zval *object, zval *member, int type, void **cache_slot)
{
	php_ev_object       *obj        = Z_EV_OBJECT_P(object);
	zval                *retval     = NULL;
	php_ev_prop_handler *hnd        = NULL;
	zval tmp_member;

	if (Z_TYPE_P(member) != IS_STRING) {
		ZVAL_COPY(&tmp_member, member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
		cache_slot = NULL;
	}

	if (obj->prop_handler != NULL) {
		hnd = zend_hash_find_ptr(obj->prop_handler, Z_STR_P(member));
	}

	if (hnd && hnd->get_ptr_ptr_func != NULL) {
		retval = hnd->get_ptr_ptr_func(obj);
	} else {
		zend_object_handlers *std_hnd = zend_get_std_object_handlers();
		retval = std_hnd->get_property_ptr_ptr(object, member, type, cache_slot);
	}

	if (Z_ISUNDEF_P(retval)) {
		ZVAL_NULL(retval);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}

	return retval;
}
/* }}} */


static void php_ev_object_dtor(zend_object *object)/*{{{*/
{
	zend_objects_destroy_object(object);
}
/*}}}*/

static void php_ev_watcher_object_dtor(zend_object *object)/*{{{*/
{
	php_ev_object *intern = php_ev_object_fetch_object(object);
	PHP_EV_ASSERT(intern);

	ev_watcher *w = (ev_watcher *)intern->ptr;
	PHP_EV_ASSERT(w);

	php_ev_watcher_dtor(w);
	if (Z_REFCOUNT(php_ev_watcher_self(w)) > 1) {
		zval_ptr_dtor(&php_ev_watcher_self(w));
	}
	php_ev_object_dtor(object);

}
/*}}}*/

static void php_ev_loop_dtor(php_ev_loop *ptr)/*{{{*/
{
	PHP_EV_ASSERT(ptr);
	if (UNEXPECTED(!ptr)) {
		return;
	}

	if (ptr->loop) {
		/* Don't free memory of watchers here.
		 * They have special GC handlers for this purpose */
		ev_watcher *w = ptr->w;
		while (w) {
			/* Watcher is stopped in it's cleanup handler
			 * php_ev_stop_watcher(w);*/
			php_ev_watcher_loop(w) = NULL;

			w = php_ev_watcher_next(w);
		}

		ev_loop_destroy(ptr->loop);
		ptr->loop = NULL;
	}

	if (!Z_ISUNDEF(ptr->data)) {
		zval_ptr_dtor(&ptr->data);
	}
}
/*}}}*/

static void php_ev_loop_object_dtor(zend_object *object)/*{{{*/
{
	php_ev_object *intern = php_ev_object_fetch_object(object);
	PHP_EV_ASSERT(intern);

	php_ev_loop *loop_ptr = (php_ev_loop *)intern->ptr;
	if (EXPECTED(intern)) {
		if (ev_is_default_loop(loop_ptr->loop)) {
			if (!Z_ISUNDEF(MyG(default_loop))) {
				zval_dtor(&MyG(default_loop));
				ZVAL_UNDEF(&MyG(default_loop));
			}
		}
#if 0
		php_ev_loop_dtor((php_ev_loop *)intern->ptr);
#endif
	}

	php_ev_object_dtor(object);
}
/*}}}*/

#if EV_PERIODIC_ENABLE
static void php_ev_periodic_object_dtor(zend_object *object)/*{{{*/
{
	php_ev_object *intern = php_ev_object_fetch_object(object);
	PHP_EV_ASSERT(intern);

	ev_watcher *w = (ev_watcher *)intern->ptr;
	PHP_EV_ASSERT(w);

	if (EXPECTED(intern->ptr)) {
		php_ev_periodic *periodic_ptr = (php_ev_periodic *)intern->ptr;
		php_ev_func_info_free(&periodic_ptr->func, FALSE);
	}
	php_ev_watcher_dtor(w);

	php_ev_object_dtor(object);

}/*}}}*/
#endif

#if EV_STAT_ENABLE
static void php_ev_stat_object_dtor(zend_object *object)/*{{{*/
{
	php_ev_object *intern = php_ev_object_fetch_object(object);
	PHP_EV_ASSERT(intern);

	ev_watcher *w = (ev_watcher *)intern->ptr;
	PHP_EV_ASSERT(w);

	if (EXPECTED(intern->ptr)) {
		php_ev_stat *stat_ptr = (php_ev_stat *)intern->ptr;
		PHP_EV_EFREE(stat_ptr->path);
	}
	php_ev_watcher_dtor(w);

	php_ev_object_dtor(object);

}/*}}}*/
#endif

#if EV_EMBED_ENABLE
static void php_ev_embed_object_dtor(zend_object *object)/*{{{*/
{
	php_ev_object *intern = php_ev_object_fetch_object(object);
	PHP_EV_ASSERT(intern);

	ev_watcher *w = (ev_watcher *)intern->ptr;
	PHP_EV_ASSERT(w);

	if (EXPECTED(intern->ptr)) {
		php_ev_embed *embed_ptr = (php_ev_embed *)intern->ptr;

		if (Z_ISUNDEF(embed_ptr->other)) {
			zval_ptr_dtor(&embed_ptr->other);
			ZVAL_UNDEF(&embed_ptr->other);
		}
	}
	php_ev_watcher_dtor(w);

	php_ev_object_dtor(object);

}/*}}}*/
#endif



/* {{{ php_ev_object_free_storage
 * Common Ev object cleaner */
static void php_ev_object_free_storage(zend_object *object)
{
	php_ev_object *intern = php_ev_object_fetch_object(object);

	zend_object_std_dtor(&intern->zo);
	PHP_EV_EFREE(intern->ptr);
#if 0
	efree(intern);
#else
	OBJ_RELEASE(object);
#endif
}
/* }}} */

/* {{{ php_ev_loop_free_storage */
static void php_ev_loop_free_storage(zend_object *object)
{
	php_ev_object *intern = php_ev_object_fetch_object(object);
	php_ev_loop *loop_ptr;

	if (EXPECTED(intern)) {
		loop_ptr = (php_ev_loop *)intern->ptr;
		php_ev_loop_dtor(loop_ptr);
	}

	php_ev_object_free_storage(object);
}
/* }}} */

/* {{{ php_ev_watcher_free_storage()
 * There must *not* be EvWatcher instances created by user.
 * This is a helper for derived watcher class objects. */
static void php_ev_watcher_free_storage(ev_watcher *ptr)
{
	php_ev_watcher_dtor(ptr);
}
/* }}} */

/* {{{ php_ev_io_free_storage() */
static void php_ev_io_free_storage(zend_object *object)
{
	php_ev_object *obj_ptr = php_ev_object_fetch_object(object);

	if (EXPECTED(obj_ptr->ptr)) {
		ev_io *ptr = (ev_io *)obj_ptr->ptr;
		php_ev_watcher_free_storage((ev_watcher *)ptr);
	}

	php_ev_object_free_storage(object);
}
/* }}} */

/* {{{ php_ev_timer_free_storage() */
static void php_ev_timer_free_storage(zend_object *object)
{
	php_ev_object *intern = php_ev_object_fetch_object(object);

	php_ev_watcher_free_storage((ev_watcher *)intern->ptr);
	php_ev_object_free_storage(object);
}
/* }}} */

#if EV_PERIODIC_ENABLE
/* {{{ php_ev_periodic_free_storage() */
static void php_ev_periodic_free_storage(zend_object *object)
{
	php_ev_object *ev_obj = php_ev_object_fetch_object(object);

	if (EXPECTED(ev_obj->ptr)) {
		php_ev_periodic *periodic_ptr = (php_ev_periodic *)ev_obj->ptr;

		php_ev_func_info_free(&periodic_ptr->func, FALSE);
		php_ev_watcher_free_storage((ev_watcher *)&periodic_ptr->periodic);
	}

	php_ev_object_free_storage(object);
}
/* }}} */
#endif

#if EV_SIGNAL_ENABLE
/* {{{ php_ev_signal_free_storage() */
static void php_ev_signal_free_storage(zend_object *object)
{
	php_ev_object *ev_obj = php_ev_object_fetch_object(object);

	if (EXPECTED(ev_obj->ptr)) {
		ev_signal *ptr = (ev_signal *) ev_obj->ptr;
		php_ev_watcher_free_storage((ev_watcher *)ptr);
	}

	php_ev_object_free_storage(object);
}
/* }}} */
#endif

#if EV_CHILD_ENABLE
/* {{{ php_ev_child_free_storage() */
static void php_ev_child_free_storage(zend_object *object)
{
	php_ev_object *obj_ptr = php_ev_object_fetch_object(object);

	if (EXPECTED(obj_ptr->ptr)) {
		ev_child *ptr = (ev_child *) obj_ptr->ptr;
		php_ev_watcher_free_storage((ev_watcher *)ptr);
	}

	php_ev_object_free_storage(object);
}
/* }}} */
#endif

#if EV_STAT_ENABLE
/* {{{ php_ev_stat_free_storage() */
static void php_ev_stat_free_storage(zend_object *object)
{
	php_ev_object *ev_obj = php_ev_object_fetch_object(object);

	if (EXPECTED(ev_obj->ptr)) {
		php_ev_stat *stat_ptr = (php_ev_stat *)ev_obj->ptr;
		PHP_EV_EFREE(stat_ptr->path);
		php_ev_watcher_free_storage((ev_watcher *)stat_ptr);
	}

	php_ev_object_free_storage(object);
}
/* }}} */
#endif

#if EV_IDLE_ENABLE
/* {{{ php_ev_idle_free_storage() */
static void php_ev_idle_free_storage(zend_object *object)
{
	php_ev_object *obj_ptr = php_ev_object_fetch_object(object);

	if (EXPECTED(obj_ptr->ptr)) {
		ev_idle *ptr = (ev_idle *) obj_ptr->ptr;
		php_ev_watcher_free_storage((ev_watcher *)ptr);
	}

	php_ev_object_free_storage(object);
}
/* }}} */
#endif

#if EV_CHECK_ENABLE
/* {{{ php_ev_check_free_storage() */
static void php_ev_check_free_storage(zend_object *object)
{
	php_ev_object *obj_ptr = php_ev_object_fetch_object(object);

	if (EXPECTED(obj_ptr->ptr)) {
		ev_check *ptr = (ev_check *) obj_ptr->ptr;
		php_ev_watcher_free_storage((ev_watcher *)ptr);
	}

	php_ev_object_free_storage(object);
}
/* }}} */
#endif

#if EV_PREPARE_ENABLE
/* {{{ php_ev_prepare_free_storage() */
static void php_ev_prepare_free_storage(zend_object *object)
{
	php_ev_object *obj_ptr = php_ev_object_fetch_object(object);

	if (EXPECTED(obj_ptr->ptr)) {
		ev_prepare *ptr = (ev_prepare *) obj_ptr->ptr;
		php_ev_watcher_free_storage((ev_watcher *)ptr);
	}

	php_ev_object_free_storage(object);
}
/* }}} */
#endif

#if EV_EMBED_ENABLE
/* {{{ php_ev_embed_free_storage() */
static void php_ev_embed_free_storage(zend_object *object)
{
	php_ev_object *obj_ptr = php_ev_object_fetch_object(object);

	if (EXPECTED(obj_ptr->ptr)) {
		php_ev_embed *embed_ptr = (php_ev_embed *)obj_ptr->ptr;

		if (Z_ISUNDEF(embed_ptr->other)) {
			zval_ptr_dtor(&embed_ptr->other);
			ZVAL_UNDEF(&embed_ptr->other);
		}

		php_ev_watcher_free_storage((ev_watcher *)&embed_ptr->embed);
	}

	php_ev_object_free_storage(object);
}
/* }}} */
#endif

#if EV_FORK_ENABLE
/* {{{ php_ev_fork_free_storage() */
static void php_ev_fork_free_storage(zend_object *object)
{
	php_ev_object *obj_ptr = php_ev_object_fetch_object(object);

	if (EXPECTED(obj_ptr->ptr)) {
		ev_fork *ptr = (ev_fork *) obj_ptr->ptr;
		php_ev_watcher_free_storage((ev_watcher *)ptr);
	}

	php_ev_object_free_storage(object);
}
/* }}} */
#endif

php_ev_object * php_ev_object_new(zend_class_entry *ce)/*{{{*/
{
	php_ev_object    *intern;
	zend_class_entry *ce_parent;

	intern = ecalloc(1, sizeof(php_ev_object) + zend_object_properties_size(ce));

	ce_parent = ce;
	while (ce_parent->type != ZEND_INTERNAL_CLASS && ce_parent->parent != NULL) {
		ce_parent = ce_parent->parent;
	}
	intern->prop_handler = zend_hash_find_ptr(&classes, ce_parent->name);

	zend_object_std_init(&intern->zo, ce);
	object_properties_init(&intern->zo, ce);

	return intern;
}/*}}}*/

/* {{{ php_ev_object_create
 Custom Ev object(class instance) creator */
zend_object * php_ev_object_create(zend_class_entry *ce)
{
	php_ev_object        *intern;
	zend_object_handlers *handlers;

	intern = php_ev_object_new(ce);

	if (instanceof_function(ce, ev_loop_class_entry_ptr)) {
		/* EvLoop */
		handlers = &ev_object_loop_handlers;
	} else if (instanceof_function(ce, ev_io_class_entry_ptr)) {
		/* EvIo */
		handlers = &ev_object_io_handlers;
	} else if (instanceof_function(ce, ev_timer_class_entry_ptr)) {
		/* EvTimer */
		handlers = &ev_object_timer_handlers;
#if EV_PERIODIC_ENABLE
	} else if (instanceof_function(ce, ev_periodic_class_entry_ptr)) {
		/* EvPeriodic */
		handlers = &ev_object_periodic_handlers;
#endif
#if EV_SIGNAL_ENABLE
	} else if (instanceof_function(ce, ev_signal_class_entry_ptr)) {
		/* EvSignal*/
		handlers = &ev_object_signal_handlers;
#endif
#if EV_CHILD_ENABLE
	} else if (instanceof_function(ce, ev_child_class_entry_ptr)) {
		/* EvChild */
		handlers = &ev_object_child_handlers;
#endif
#if EV_STAT_ENABLE
	} else if (instanceof_function(ce, ev_stat_class_entry_ptr)) {
		/* EvStat */
		handlers = &ev_object_stat_handlers;
#endif
#if EV_IDLE_ENABLE
	} else if (instanceof_function(ce, ev_idle_class_entry_ptr)) {
		/* EvIdle */
		handlers = &ev_object_idle_handlers;
#endif
#if EV_CHECK_ENABLE
	} else if (instanceof_function(ce, ev_check_class_entry_ptr)) {
		/* EvCheck */
		handlers = &ev_object_check_handlers;
#endif
#if EV_PREPARE_ENABLE
	} else if (instanceof_function(ce, ev_prepare_class_entry_ptr)) {
		/* EvPrepare */
		handlers = &ev_object_prepare_handlers;
#endif
#if EV_EMBED_ENABLE
	} else if (instanceof_function(ce, ev_embed_class_entry_ptr)) {
		/* EvEmbed */
		handlers = &ev_object_embed_handlers;
#endif
#if EV_FORK_ENABLE
	} else if (instanceof_function(ce, ev_fork_class_entry_ptr)) {
		/* EvFork */
		handlers = &ev_object_fork_handlers;
#endif
	} else {
	 	handlers = &ev_object_handlers;
	}

	intern->zo.handlers = handlers;

	return &intern->zo;
}
/* }}} */

/* {{{ php_ev_register_classes
 * Registers all Ev classes. Should be called in MINIT */
static inline void php_ev_register_classes()
{
	zend_class_entry *ce;

	/* {{{ Ev */
	PHP_EV_REGISTER_CLASS_ENTRY("Ev", ev_class_entry_ptr, ev_class_entry_functions);
	ce = ev_class_entry_ptr;
	ce->ce_flags |= ZEND_ACC_FINAL;
	/* }}} */

	/* {{{ EvLoop */
	PHP_EV_REGISTER_CLASS_ENTRY("EvLoop", ev_loop_class_entry_ptr, ev_loop_class_entry_functions);
	ce = ev_loop_class_entry_ptr;
	ce->ce_flags |= ZEND_ACC_FINAL;
	zend_hash_init(&php_ev_properties, 0, NULL, free_prop_handler, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_properties, ev_loop_property_entries);
	PHP_EV_DECL_PROP_NULL(ce, data,             ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, backend,          ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, is_default_loop,  ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, iteration,        ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, pending,          ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, io_interval,      ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, timeout_interval, ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, depth,            ZEND_ACC_PUBLIC);
	zend_hash_add_ptr(&classes, ce->name, &php_ev_properties);
	/* }}} */

	/* {{{ EvWatcher */
	PHP_EV_REGISTER_CLASS_ENTRY("EvWatcher", ev_watcher_class_entry_ptr, ev_watcher_class_entry_functions);
	ce = ev_watcher_class_entry_ptr;
	ce->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	zend_hash_init(&php_ev_watcher_properties, 0, NULL, free_prop_handler, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_watcher_properties, ev_watcher_property_entries);
	PHP_EV_DECL_PROP_NULL(ce, is_active,  ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, data,       ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, is_pending, ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, priority,   ZEND_ACC_PUBLIC);
	zend_hash_add_ptr(&classes, ce->name, &php_ev_watcher_properties);
	/* }}} */

	/* {{{ EvIo */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvIo", ev_io_class_entry_ptr, ev_io_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_io_class_entry_ptr;
	zend_hash_init(&php_ev_io_properties, 0, NULL, free_prop_handler, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_io_properties, ev_io_property_entries);
	zend_hash_merge(&php_ev_io_properties, &php_ev_watcher_properties, copy_prop_handler, 0);
	PHP_EV_DECL_PROP_NULL(ce, fd,     ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, events, ZEND_ACC_PUBLIC);
	zend_hash_add_ptr(&classes, ce->name, &php_ev_io_properties);
	/* }}} */

	/* {{{ EvTimer */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvTimer", ev_timer_class_entry_ptr, ev_timer_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_timer_class_entry_ptr;
	zend_hash_init(&php_ev_timer_properties, 0, NULL, free_prop_handler, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_timer_properties, ev_timer_property_entries);
	zend_hash_merge(&php_ev_timer_properties, &php_ev_watcher_properties, copy_prop_handler, 0);
	PHP_EV_DECL_PROP_NULL(ce, repeat,    ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, remaining, ZEND_ACC_PUBLIC);
	zend_hash_add_ptr(&classes, ce->name, &php_ev_timer_properties);
	/* }}} */

#if EV_PERIODIC_ENABLE
	/* {{{ EvPeriodic */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvPeriodic", ev_periodic_class_entry_ptr, ev_periodic_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_periodic_class_entry_ptr;
	zend_hash_init(&php_ev_periodic_properties, 0, NULL, free_prop_handler, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_periodic_properties, ev_periodic_property_entries);
	zend_hash_merge(&php_ev_periodic_properties, &php_ev_watcher_properties, copy_prop_handler, 0);
	PHP_EV_DECL_PROP_NULL(ce, offset,   ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, interval, ZEND_ACC_PUBLIC);
	zend_hash_add_ptr(&classes, ce->name, &php_ev_periodic_properties);
	/* }}} */
#endif

#if EV_SIGNAL_ENABLE
	/* {{{ EvSignal*/
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvSignal", ev_signal_class_entry_ptr, ev_signal_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_signal_class_entry_ptr;
	zend_hash_init(&php_ev_signal_properties, 0, NULL, free_prop_handler, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_signal_properties, ev_signal_property_entries);
	zend_hash_merge(&php_ev_signal_properties, &php_ev_watcher_properties, copy_prop_handler, 0);
	PHP_EV_DECL_PROP_NULL(ce, signum, ZEND_ACC_PUBLIC);
	zend_hash_add_ptr(&classes, ce->name, &php_ev_signal_properties);
	/* }}} */
#endif

#if EV_CHILD_ENABLE
	/* {{{ EvChild*/
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvChild", ev_child_class_entry_ptr, ev_child_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_child_class_entry_ptr;
	zend_hash_init(&php_ev_child_properties, 0, NULL, free_prop_handler, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_child_properties, ev_child_property_entries);
	zend_hash_merge(&php_ev_child_properties, &php_ev_watcher_properties, copy_prop_handler, 0);
	PHP_EV_DECL_PROP_NULL(ce, pid,     ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, rpid,    ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, rstatus, ZEND_ACC_PUBLIC);
	zend_hash_add_ptr(&classes, ce->name, &php_ev_child_properties);
	/* }}} */
#endif

#if EV_STAT_ENABLE
	/* {{{ EvStat */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvStat", ev_stat_class_entry_ptr, ev_stat_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_stat_class_entry_ptr;
	zend_hash_init(&php_ev_stat_properties, 0, NULL, free_prop_handler, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_stat_properties, ev_stat_property_entries);
	zend_hash_merge(&php_ev_stat_properties, &php_ev_watcher_properties, copy_prop_handler, 0);
	PHP_EV_DECL_PROP_NULL(ce, path,     ZEND_ACC_PUBLIC);
	PHP_EV_DECL_PROP_NULL(ce, interval, ZEND_ACC_PUBLIC);
	zend_hash_add_ptr(&classes, ce->name, &php_ev_stat_properties);
	/* }}} */
#endif

#if EV_IDLE_ENABLE
	/* {{{ EvIdle */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvIdle", ev_idle_class_entry_ptr, ev_idle_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_idle_class_entry_ptr;
	zend_hash_add_ptr(&classes, ce->name, &php_ev_watcher_properties);
	/* }}} */
#endif

#if EV_CHECK_ENABLE
	/* {{{ EvCheck */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvCheck", ev_check_class_entry_ptr, ev_check_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_check_class_entry_ptr;
	zend_hash_add_ptr(&classes, ce->name, &php_ev_watcher_properties);
	/* }}} */
#endif

#if EV_PREPARE_ENABLE
	/* {{{ EvPrepare */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvPrepare", ev_prepare_class_entry_ptr, ev_prepare_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_prepare_class_entry_ptr;
	zend_hash_add_ptr(&classes, ce->name, &php_ev_watcher_properties);
	/* }}} */
#endif

#if EV_EMBED_ENABLE
	/* {{{ EvEmbed */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvEmbed", ev_embed_class_entry_ptr, ev_embed_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_embed_class_entry_ptr;
	zend_hash_init(&php_ev_embed_properties, 0, NULL, free_prop_handler, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_embed_properties, ev_embed_property_entries);
	zend_hash_merge(&php_ev_embed_properties, &php_ev_watcher_properties, copy_prop_handler, 0);
	PHP_EV_DECL_PROP_NULL(ce, embed, ZEND_ACC_PUBLIC);
	zend_hash_add_ptr(&classes, ce->name, &php_ev_embed_properties);
	/* }}} */
#endif

#if EV_CHECK_ENABLE
	/* {{{ EvFork */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvFork", ev_fork_class_entry_ptr, ev_fork_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_fork_class_entry_ptr;
	zend_hash_add_ptr(&classes, ce->name, &php_ev_watcher_properties);
	/* }}} */
#endif
}
/* }}} */

/* Private functions }}} */


/* {{{ PHP_GINIT_FUNCTION */
static PHP_GINIT_FUNCTION(ev)
{
#if defined(COMPILE_DL_EV) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	ZVAL_UNDEF(&ev_globals->default_loop);

	memset(ev_globals->signal_loops, 0, sizeof(ev_globals->signal_loops));
}
/* }}} */


/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(ev)
{
	zend_object_handlers *std_hnd = zend_get_std_object_handlers();

	memcpy(&ev_object_handlers, std_hnd, sizeof(zend_object_handlers));
	ev_object_handlers.offset               = XtOffsetOf(php_ev_object, zo);
	ev_object_handlers.free_obj             = php_ev_object_free_storage;
	ev_object_handlers.clone_obj            = NULL;
	ev_object_handlers.dtor_obj             = php_ev_watcher_object_dtor;
	ev_object_handlers.read_property        = php_ev_read_property;
	ev_object_handlers.write_property       = php_ev_write_property;
	ev_object_handlers.get_property_ptr_ptr = php_ev_get_property_ptr_ptr; /* std_hnd->get_property_ptr_ptr */;
	ev_object_handlers.has_property         = php_ev_has_property;
	ev_object_handlers.get_debug_info       = php_ev_object_get_debug_info;
	ev_object_handlers.get_properties       = php_ev_get_properties;
	ev_object_handlers.get_gc               = php_ev_get_gc;

#define SET_EV_FREE_OBJ(NAME) \
	memcpy(&ev_object_ ## NAME ##  _handlers, &ev_object_handlers, sizeof(zend_object_handlers)); \
	ev_object_ ## NAME ## _handlers.free_obj = php_ev_ ## NAME ## _free_storage

	SET_EV_FREE_OBJ(loop);
	ev_object_loop_handlers.get_gc   = php_ev_loop_get_gc;
	ev_object_loop_handlers.dtor_obj = php_ev_loop_object_dtor;

	SET_EV_FREE_OBJ(io);
	SET_EV_FREE_OBJ(timer);
#if EV_PERIODIC_ENABLE
	SET_EV_FREE_OBJ(periodic);
	ev_object_periodic_handlers.dtor_obj = php_ev_periodic_object_dtor;
#endif
#if EV_SIGNAL_ENABLE
	SET_EV_FREE_OBJ(signal);
#endif
#if EV_CHILD_ENABLE
	SET_EV_FREE_OBJ(child);
#endif
#if EV_STAT_ENABLE
	SET_EV_FREE_OBJ(stat);
	ev_object_stat_handlers.dtor_obj = php_ev_stat_object_dtor;
#endif
#if EV_IDLE_ENABLE
	SET_EV_FREE_OBJ(idle);
#endif
#if EV_CHECK_ENABLE
	SET_EV_FREE_OBJ(check);
#endif
#if EV_PREPARE_ENABLE
	SET_EV_FREE_OBJ(prepare);
#endif
#if EV_EMBED_ENABLE
	SET_EV_FREE_OBJ(embed);
	ev_object_embed_handlers.dtor_obj = php_ev_embed_object_dtor;
#endif
#if EV_FORK_ENABLE
	SET_EV_FREE_OBJ(fork);
#endif

#undef SET_EV_FREE_OBJ


	zend_hash_init(&classes, 0, NULL, NULL, 1);
	php_ev_register_classes();

	/* Loop flags */
	REGISTER_EV_CLASS_CONST_LONG(FLAG_AUTO,      EVFLAG_AUTO);
	REGISTER_EV_CLASS_CONST_LONG(FLAG_NOENV,     EVFLAG_NOENV);
	REGISTER_EV_CLASS_CONST_LONG(FLAG_FORKCHECK, EVFLAG_FORKCHECK);
	REGISTER_EV_CLASS_CONST_LONG(FLAG_NOINOTIFY, EVFLAG_NOINOTIFY);
	REGISTER_EV_CLASS_CONST_LONG(FLAG_SIGNALFD,  EVFLAG_SIGNALFD);
	REGISTER_EV_CLASS_CONST_LONG(FLAG_NOSIGMASK, EVFLAG_NOSIGMASK);

	/* ev_run flags */
	REGISTER_EV_CLASS_CONST_LONG(RUN_NOWAIT, EVRUN_NOWAIT);
	REGISTER_EV_CLASS_CONST_LONG(RUN_ONCE,   EVRUN_ONCE);

	/* ev_break flags */
	REGISTER_EV_CLASS_CONST_LONG(BREAK_CANCEL, EVBREAK_CANCEL);
	REGISTER_EV_CLASS_CONST_LONG(BREAK_ONE,    EVBREAK_ONE);
	REGISTER_EV_CLASS_CONST_LONG(BREAK_ALL,    EVBREAK_ALL);

	/* Watcher priorities */
	REGISTER_EV_CLASS_CONST_LONG(MINPRI, EV_MINPRI);
	REGISTER_EV_CLASS_CONST_LONG(MAXPRI, EV_MAXPRI);


	/* Watcher events/types */
	REGISTER_EV_CLASS_CONST_LONG(READ,     EV_READ);
	REGISTER_EV_CLASS_CONST_LONG(WRITE,    EV_WRITE);
	REGISTER_EV_CLASS_CONST_LONG(TIMER,    EV_TIMER);
	REGISTER_EV_CLASS_CONST_LONG(PERIODIC, EV_PERIODIC);
	REGISTER_EV_CLASS_CONST_LONG(SIGNAL,   EV_SIGNAL);
	REGISTER_EV_CLASS_CONST_LONG(CHILD,    EV_CHILD);
	REGISTER_EV_CLASS_CONST_LONG(STAT,     EV_STAT);
	REGISTER_EV_CLASS_CONST_LONG(IDLE,     EV_IDLE);
	REGISTER_EV_CLASS_CONST_LONG(PREPARE,  EV_PREPARE);
	REGISTER_EV_CLASS_CONST_LONG(CHECK,    EV_CHECK);
	REGISTER_EV_CLASS_CONST_LONG(EMBED,    EV_EMBED);
	REGISTER_EV_CLASS_CONST_LONG(CUSTOM,   EV_CUSTOM);
	REGISTER_EV_CLASS_CONST_LONG(ERROR,    EV_ERROR);

	/* Backend types */
	REGISTER_EV_CLASS_CONST_LONG(BACKEND_SELECT,  EVBACKEND_SELECT);
	REGISTER_EV_CLASS_CONST_LONG(BACKEND_POLL,    EVBACKEND_POLL);
	REGISTER_EV_CLASS_CONST_LONG(BACKEND_EPOLL,   EVBACKEND_EPOLL);
	REGISTER_EV_CLASS_CONST_LONG(BACKEND_KQUEUE,  EVBACKEND_KQUEUE);
	REGISTER_EV_CLASS_CONST_LONG(BACKEND_DEVPOLL, EVBACKEND_DEVPOLL);
	REGISTER_EV_CLASS_CONST_LONG(BACKEND_PORT,    EVBACKEND_PORT);
	REGISTER_EV_CLASS_CONST_LONG(BACKEND_ALL,     EVBACKEND_ALL);
	REGISTER_EV_CLASS_CONST_LONG(BACKEND_MASK,    EVBACKEND_MASK);

	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MSHUTDOWN_FUNCTION */
PHP_MSHUTDOWN_FUNCTION(ev)
{
	zend_hash_destroy(&php_ev_properties);
	zend_hash_destroy(&php_ev_watcher_properties);
	zend_hash_destroy(&php_ev_io_properties);
	zend_hash_destroy(&php_ev_timer_properties);
#if EV_PERIODIC_ENABLE
	zend_hash_destroy(&php_ev_periodic_properties);
#endif
#if EV_SIGNAL_ENABLE
	zend_hash_destroy(&php_ev_signal_properties);
#endif
#if EV_CHILD_ENABLE
	zend_hash_destroy(&php_ev_child_properties);
#endif
#if EV_STAT_ENABLE
	zend_hash_destroy(&php_ev_stat_properties);
#endif
#if EV_EMBED_ENABLE
	zend_hash_destroy(&php_ev_embed_properties);
#endif

	zend_hash_destroy(&classes);

	return SUCCESS;
}
/* }}} */


/* {{{ PHP_RINIT_FUNCTION */
PHP_RINIT_FUNCTION(ev)
{
	ZVAL_UNDEF(&MyG(default_loop));
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION */
PHP_MINFO_FUNCTION(ev)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Ev support", "enabled");
#ifdef PHP_EV_DEBUG
	php_info_print_table_row(2, "Debug support", "enabled");
#else
	php_info_print_table_row(2, "Debug support", "disabled");
#endif
	php_info_print_table_row(2, "Version", PHP_EV_VERSION);
	php_info_print_table_end();
}
/* }}} */

#include "io.c"
#include "timer.c"
#if EV_PERIODIC_ENABLE
# include "periodic.c"
#endif
#if EV_SIGNAL_ENABLE
# include "signal.c"
#endif
#if EV_CHILD_ENABLE
# include "child.c"
#endif
#if EV_STAT_ENABLE
# include "stat.c"
#endif
#if EV_IDLE_ENABLE
# include "idle.c"
#endif
#if EV_CHECK_ENABLE
# include "check.c"
#endif
#if EV_PREPARE_ENABLE
# include "prepare.c"
#endif
#if EV_EMBED_ENABLE
# include "embed.c"
#endif
#if EV_FORK_ENABLE
# include "fork.c"
#endif
#include "loop.c"


/* {{{ Ev methods */

#define PHP_EV_METHOD_INT_VOID(name, method)       \
	PHP_METHOD(Ev, method)                         \
{                                                  \
	if (zend_parse_parameters_none() != FAILURE) { \
		RETURN_LONG((zend_long)ev_ ## name());    \
	}                                              \
}

#define PHP_EV_METHOD_DEFAULT_LOOP_INT_VOID(name, method)                \
	PHP_METHOD(Ev, method)                                               \
{                                                                        \
	php_ev_object *ev_obj;                                               \
	if (zend_parse_parameters_none() != FAILURE) {                       \
		ev_obj = Z_EV_OBJECT_P(php_ev_default_loop());                   \
		PHP_EV_CONSTRUCT_CHECK(ev_obj);                                  \
		RETURN_LONG(ev_ ## name(PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj))); \
	}                                                                    \
}

#define PHP_EV_METHOD_VOID_VOID(name, method)               \
	PHP_METHOD(Ev, method)                                  \
{                                                           \
	php_ev_object *ev_obj;                                  \
	if (zend_parse_parameters_none() != FAILURE) {          \
		ev_obj = Z_EV_OBJECT_P(php_ev_default_loop());      \
		PHP_EV_CONSTRUCT_CHECK(ev_obj);                     \
		ev_ ## name(PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj)); \
	}                                                       \
}

PHP_EV_METHOD_INT_VOID(supported_backends,   supportedBackends)
PHP_EV_METHOD_INT_VOID(recommended_backends, recommendedBackends)
PHP_EV_METHOD_INT_VOID(embeddable_backends,  embeddableBackends)

/* {{{ proto void Ev::feedSignal(int signum) */
PHP_METHOD(Ev, feedSignal)
{
	zend_long signum;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &signum) != FAILURE) {
		ev_feed_signal(signum);
	}
}
/* }}} */

/* {{{ proto void Ev::feedSignalEvent(int signum) */
PHP_METHOD(Ev, feedSignalEvent)
{
	zend_long      signum;
	php_ev_object *ev_obj;
	zval          *default_loop_ptr = php_ev_default_loop();

	if (!default_loop_ptr) {
		php_error_docref(NULL, E_ERROR, "The default loop is not initialized");
		return;
	}

	/* Fetch the default loop */
	ev_obj = Z_EV_OBJECT_P(default_loop_ptr);
	EV_P = PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj);

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &signum) != FAILURE) {
		ev_feed_signal_event(EV_A_ signum);
	}
}
/* }}} */

/* {{{ proto void Ev::sleep(double seconds) */
PHP_METHOD(Ev, sleep)
{
	double seconds;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "d", &seconds) != FAILURE) {
		ev_sleep(seconds);
	}
}
/* }}} */

/* {{{ proto double Ev::time(void) */
PHP_METHOD(Ev, time)
{
	if (zend_parse_parameters_none() != FAILURE) {
		RETURN_DOUBLE((double) ev_time());
	}
}
/* }}} */

/* {{{ proto void Ev::run([int flags = 0]) */
PHP_METHOD(Ev, run)
{
	php_ev_object *ev_obj;
	zend_long      flags  = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &flags) == FAILURE) {
		return;
	}

	ev_obj = Z_EV_OBJECT_P(php_ev_default_loop());
	PHP_EV_CONSTRUCT_CHECK(ev_obj);
	ev_run(PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj), flags);
}
/* }}} */

/* {{{ proto double Ev::now(void) */
PHP_METHOD(Ev, now)
{
	php_ev_object *ev_obj;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	ev_obj = Z_EV_OBJECT_P(php_ev_default_loop());
	PHP_EV_CONSTRUCT_CHECK(ev_obj);
	RETURN_DOUBLE((double)ev_now(PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj)));
}
/* }}} */

/* {{{ proto void Ev::stop([int how = 0]) */
PHP_METHOD(Ev, stop)
{
	php_ev_object *ev_obj;
	zend_long      how   = EVBREAK_ONE;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &how) == FAILURE) {
		return;
	}

	ev_obj = Z_EV_OBJECT_P(php_ev_default_loop());
	PHP_EV_CONSTRUCT_CHECK(ev_obj);
	ev_break(PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj), how);
}
/* }}} */

/* {{{ proto void Ev::verify(void) */
PHP_METHOD(Ev, verify)
{
	php_ev_object *ev_obj;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	ev_obj = Z_EV_OBJECT_P(php_ev_default_loop());
	PHP_EV_CONSTRUCT_CHECK(ev_obj);
	ev_verify(PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj));
}
/* }}} */

PHP_EV_METHOD_DEFAULT_LOOP_INT_VOID(iteration, iteration)
PHP_EV_METHOD_DEFAULT_LOOP_INT_VOID(depth,     depth)
PHP_EV_METHOD_DEFAULT_LOOP_INT_VOID(backend,   backend)

PHP_EV_METHOD_VOID_VOID(now_update, nowUpdate)
PHP_EV_METHOD_VOID_VOID(suspend,    suspend)
PHP_EV_METHOD_VOID_VOID(resume,     resume)
/* }}} */

#endif /* HAVE_EV */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
