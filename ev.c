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
#include "util.h"

#if !defined(_WIN32) && !defined(_MINIX)
# include <pthread.h>
#endif

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
    NULL,                         /* PHP_RINIT(ev)     */
    NULL,                         /* PHP_RSHUTDOWN(ev) */
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
ZEND_GET_MODULE(ev)
#endif

/* {{{ Private functions */

/* {{{ php_ev_default_fork */
static void php_ev_default_fork(void)
{
	ev_loop_fork(EV_DEFAULT);
}
/* }}} */

/* {{{ php_ev_default_loop */
zval *php_ev_default_loop(TSRMLS_D)
{
	zval **default_loop_ptr_ptr = &MyG(default_loop);

	if (*default_loop_ptr_ptr) {
		return *default_loop_ptr_ptr;
	}

	php_ev_object  *ev_obj;
	struct ev_loop *loop   = ev_default_loop(EVFLAG_AUTO);

	if (!loop) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed to instanciate default loop, "
				"bad $LIBEV_FLAGS in environment?");
		return 0;
	}

	MAKE_STD_ZVAL(*default_loop_ptr_ptr);
	PHP_EV_INIT_CLASS_OBJECT(*default_loop_ptr_ptr, ev_loop_class_entry_ptr);

	ev_obj = (php_ev_object *) zend_object_store_get_object(*default_loop_ptr_ptr TSRMLS_CC);

	php_ev_loop *ptr = (php_ev_loop *) emalloc(sizeof(php_ev_loop));
	memset(ptr, 0, sizeof(php_ev_loop));
	ptr->loop = loop;

	ev_obj->ptr = (void *) ptr;

	ev_set_userdata(loop, (void *) *default_loop_ptr_ptr);

	return *default_loop_ptr_ptr;
}
/* }}} */

/* {{{ php_ev_prop_read_default */
static int php_ev_prop_read_default(php_ev_object *obj, zval **retval TSRMLS_DC)
{
	*retval = NULL;
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot read property");
	return FAILURE;
}
/* }}} */

/* {{{ php_ev_prop_write_default */
static int php_ev_prop_write_default(php_ev_object *obj, zval *newval TSRMLS_DC)
{
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot write property");
	return FAILURE;
}
/* }}} */

/* {{{ php_ev_add_property */
static void php_ev_add_property(HashTable *h, const char *name, size_t name_len, php_ev_read_t read_func, php_ev_write_t write_func, php_ev_get_prop_ptr_ptr_t get_ptr_ptr_func TSRMLS_DC) {
	php_ev_prop_handler p;

	p.name             = (char *) name;
	p.name_len         = name_len;
	p.read_func        = (read_func) ? read_func : php_ev_prop_read_default;
	p.write_func       = (write_func) ? write_func: php_ev_prop_write_default;
	p.get_ptr_ptr_func = get_ptr_ptr_func;
	zend_hash_add(h, name, name_len + 1, &p, sizeof(php_ev_prop_handler), NULL);
}
/* }}} */

/* {{{ php_ev_read_property */
static zval *php_ev_read_property(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC)
{
	zval                 tmp_member;
	zval                *retval;
	php_ev_object       *obj;
	php_ev_prop_handler *hnd;
	int                  ret;

	ret = FAILURE;
	obj = (php_ev_object *) zend_objects_get_address(object TSRMLS_CC);

	if (member->type != IS_STRING) {
	    tmp_member = *member;
	    zval_copy_ctor(&tmp_member);
	    convert_to_string(&tmp_member);
	    member = &tmp_member;
	}

	if (obj->prop_handler != NULL) {
	    ret = zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
	}

	if (ret == SUCCESS) {
	    ret = hnd->read_func(obj, &retval TSRMLS_CC);
	    if (ret == SUCCESS) {
	        /* ensure we're creating a temporary variable */
	        Z_SET_REFCOUNT_P(retval, 0);
	    } else {
	        retval = EG(uninitialized_zval_ptr);
	    }
	} else {
	    zend_object_handlers * std_hnd = zend_get_std_object_handlers();
	    retval = std_hnd->read_property(object, member, type, key TSRMLS_CC);
	}

	if (member == &tmp_member) {
	    zval_dtor(member);
	}
	return(retval);
}
/* }}} */

/* {{{ php_ev_write_property */
static void php_ev_write_property(zval *object, zval *member, zval *value, const zend_literal *key TSRMLS_DC)
{
	zval                 tmp_member;
	php_ev_object       *obj;
	php_ev_prop_handler *hnd;
	int                  ret;

	if (member->type != IS_STRING) {
	    tmp_member = *member;
	    zval_copy_ctor(&tmp_member);
	    convert_to_string(&tmp_member);
	    member = &tmp_member;
	}

	ret = FAILURE;
	obj = (php_ev_object *) zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
	    ret = zend_hash_find((HashTable *) obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void **) &hnd);
	}
	if (ret == SUCCESS) {
	    hnd->write_func(obj, value TSRMLS_CC);
	} else {
	    zend_object_handlers * std_hnd = zend_get_std_object_handlers();
	    std_hnd->write_property(object, member, value, key TSRMLS_CC);
	}

	if (member == &tmp_member) {
	    zval_dtor(member);
	}
}
/* }}} */

/* {{{ php_ev_has_property */
static int php_ev_has_property(zval *object, zval *member, int has_set_exists, const zend_literal *key TSRMLS_DC)
{
	php_ev_object	*obj = (php_ev_object *) zend_objects_get_address(object TSRMLS_CC);
	int				 ret = 0;
	php_ev_prop_handler  p;

	if (zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member) + 1, (void **) &p) == SUCCESS) {
		switch (has_set_exists) {
			case 2:
				ret = 1;
				break;
			case 1: {
						zval *value = php_ev_read_property(object, member, BP_VAR_IS, key TSRMLS_CC);
						if (value != EG(uninitialized_zval_ptr)) {
							convert_to_boolean(value);
							ret = Z_BVAL_P(value)? 1:0;
							/* refcount is 0 */
							Z_ADDREF_P(value);
							zval_ptr_dtor(&value);
						}
						break;
					}
			case 0:{
					   zval *value = php_ev_read_property(object, member, BP_VAR_IS, key TSRMLS_CC);
					   if (value != EG(uninitialized_zval_ptr)) {
						   ret = Z_TYPE_P(value) != IS_NULL? 1:0;
						   /* refcount is 0 */
						   Z_ADDREF_P(value);
						   zval_ptr_dtor(&value);
					   }
					   break;
				   }
			default:
				   php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid value for has_set_exists");
		}
	} else {
		zend_object_handlers *std_hnd = zend_get_std_object_handlers();
		ret = std_hnd->has_property(object, member, has_set_exists, key TSRMLS_CC);
	}
	return ret;
} /* }}} */

/* {{{ php_ev_object_get_debug_info */
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
static HashTable *php_ev_object_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
	php_ev_object *obj = (php_ev_object *) zend_objects_get_address(object TSRMLS_CC); 
	HashTable *retval, *props = obj->prop_handler;
	HashPosition pos;
	php_ev_prop_handler *entry;

	ALLOC_HASHTABLE(retval);
	ZEND_INIT_SYMTABLE_EX(retval, zend_hash_num_elements(props) + 1, 0);

	zend_hash_internal_pointer_reset_ex(props, &pos);
	while (zend_hash_get_current_data_ex(props, (void **) &entry, &pos) == SUCCESS) {
	    zval member;
	    zval *value;

	    INIT_ZVAL(member);
	    ZVAL_STRINGL(&member, entry->name, entry->name_len, 0);

	    value = php_ev_read_property(object, &member, BP_VAR_IS, 0 TSRMLS_CC);
	    if (value != EG(uninitialized_zval_ptr)) {
	        Z_ADDREF_P(value);
	        zend_hash_add(retval, entry->name, entry->name_len + 1, &value, sizeof(zval *) , NULL);
	    }       

	    zend_hash_move_forward_ex(props, &pos);
	}               

	*is_temp = 1;   

	return retval;
}               
#endif    
/* }}} */

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 4
/* {{{ get_properties
   Returns all object properties. */
static HashTable *get_properties(zval *object TSRMLS_DC)
{
	php_ev_object       *obj;
	php_ev_prop_handler *hnd;
	HashTable           *props;
	zval                *val;
	char                *key;
	uint                 key_len;
	HashPosition         pos;
	ulong                num_key;

	obj = (php_ev_object *) zend_objects_get_address(object TSRMLS_CC);
	props = zend_std_get_properties(object TSRMLS_CC);

	if (obj->prop_handler) {
		zend_hash_internal_pointer_reset_ex(obj->prop_handler, &pos);

		while (zend_hash_get_current_data_ex(obj->prop_handler,
					(void **) &hnd, &pos) == SUCCESS) {
			zend_hash_get_current_key_ex(obj->prop_handler,
					&key, &key_len, &num_key, 0, &pos);
			if (!hnd->read_func || hnd->read_func(obj, &val TSRMLS_CC) != SUCCESS) {
				val = EG(uninitialized_zval_ptr);
				Z_ADDREF_P(val);
			}
			zend_hash_update(props, key, key_len, (void *) &val, sizeof(zval *), NULL);
			zend_hash_move_forward_ex(obj->prop_handler, &pos);
		}
	}

	return obj->zo.properties;
}
/* }}} */
#endif



/* {{{ php_ev_get_property_ptr_ptr */
#if PHP_VERSION_ID >= 50500
static zval **php_ev_get_property_ptr_ptr(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC)
#else
static zval **php_ev_get_property_ptr_ptr(zval *object, zval *member, const zend_literal *key TSRMLS_DC)
#endif
{
	php_ev_object        *obj;
	zval                  tmp_member;
	zval                **retval     = NULL;
	php_ev_prop_handler  *hnd;
	int                   ret        = FAILURE;

	if (member->type != IS_STRING) {
		tmp_member = *member;
		zval_copy_ctor(&tmp_member);
		convert_to_string(&tmp_member);
		member = &tmp_member;
	}

	obj = (php_ev_object *) zend_objects_get_address(object TSRMLS_CC);

	if (obj->prop_handler != NULL) {
		ret = zend_hash_find(obj->prop_handler, Z_STRVAL_P(member), Z_STRLEN_P(member) + 1, (void **) &hnd);
	}

	if (ret == FAILURE) {
#if PHP_VERSION_ID >= 50500
		retval = zend_get_std_object_handlers()->get_property_ptr_ptr(object, member, type, key TSRMLS_CC);
#else
		retval = zend_get_std_object_handlers()->get_property_ptr_ptr(object, member, key TSRMLS_CC);
#endif

	} else if (hnd->get_ptr_ptr_func) {
		retval = hnd->get_ptr_ptr_func(obj TSRMLS_CC);
	}

	if (member == &tmp_member) {
		zval_dtor(member);
	}

	return retval;
}
/* }}} */

/* {{{ php_ev_object_free_storage 
 * Common Ev object cleaner */
static void php_ev_object_free_storage(void *obj_ TSRMLS_DC)
{
	php_ev_object *obj = (php_ev_object *) obj_;

	zend_object_std_dtor(&obj->zo TSRMLS_CC);

	if (obj->ptr) {
		efree(obj->ptr);
		obj->ptr = NULL;
	}

	efree(obj);
}
/* }}} */

/* {{{ php_ev_loop_free_storage */
static void php_ev_loop_free_storage(void *obj_ TSRMLS_DC)
{
	php_ev_object *obj = (php_ev_object *) obj_;

	if (UNEXPECTED(!obj->ptr)) return;
	php_ev_loop *ptr = (php_ev_loop *) obj->ptr;

	if (ptr->loop) {
		/* Don't free memory of watchers here.
		 * They have special GC handlers for this purpose */
		ev_watcher *w = ptr->w;
		while (w) {
			/* Watcher is stopped in it's cleanup handler
			 * php_ev_stop_watcher(w TSRMLS_CC);*/
			php_ev_watcher_loop(w) = NULL;
			w = php_ev_watcher_next(w);
		}

		if (ev_is_default_loop(ptr->loop)) {
			zval **default_loop_ptr_ptr = &MyG(default_loop);
			if (*default_loop_ptr_ptr) {
				zval_ptr_dtor(default_loop_ptr_ptr);
				*default_loop_ptr_ptr = NULL;
			}
		}

		ev_loop_destroy(ptr->loop);
		ptr->loop = NULL;
	}

	if (ptr->data) {
		zval_ptr_dtor(&ptr->data);
		ptr->data = NULL;
	}

	php_ev_object_free_storage(obj TSRMLS_CC);
}
/* }}} */

/* {{{ php_ev_watcher_free_storage() 
 * There must *not* be EvWatcher instances created by user.
 * This is a helper for derived watcher class objects. */
static void php_ev_watcher_free_storage(ev_watcher *ptr TSRMLS_DC)
{
	zval        *data, *self;
	ev_watcher  *w_next  , *w_prev;
	php_ev_loop *o_loop;

	/* What if we do it in php_ev_loop_free_storage()?*/
	php_ev_stop_watcher(ptr TSRMLS_CC);

	/* Re-link the list of watchers */

	w_next = php_ev_watcher_next(ptr);
	w_prev = php_ev_watcher_prev(ptr);

	if (w_prev) {
		php_ev_watcher_next(w_prev) = w_next;
	}

	if (w_next) {
		php_ev_watcher_prev(w_next) = w_prev;
	}

	o_loop = php_ev_watcher_loop(ptr);

	if (o_loop && o_loop->w == ptr) {
		o_loop->w = NULL;
	}

	php_ev_watcher_next(ptr) = php_ev_watcher_prev(ptr) = NULL;

	PHP_EV_FREE_FCALL_INFO(php_ev_watcher_fci(ptr), php_ev_watcher_fcc(ptr));
	
	data = php_ev_watcher_data(ptr);
	if (data) {
		zval_ptr_dtor(&data);
		php_ev_watcher_data(ptr) = NULL;
	}

	if (php_ev_watcher_self(ptr) && Z_REFCOUNT_P(php_ev_watcher_self(ptr)) > 1) {
		Z_DELREF_P(php_ev_watcher_self(ptr));
	}
}
/* }}} */

/* {{{ php_ev_io_free_storage() */
static void php_ev_io_free_storage(void *object TSRMLS_DC)
{
	php_ev_object *obj_ptr = (php_ev_object *) object;

	if (UNEXPECTED(!obj_ptr->ptr)) return;
	ev_io *ptr = (ev_io *) obj_ptr->ptr;

	/* Free base class members */
	php_ev_watcher_free_storage((ev_watcher *) ptr TSRMLS_CC);

	/* Free common Ev object members and the object itself */
	php_ev_object_free_storage(object TSRMLS_CC);
}
/* }}} */

/* {{{ php_ev_timer_free_storage() */
static void php_ev_timer_free_storage(void *object TSRMLS_DC)
{
	php_ev_object *obj_ptr = (php_ev_object *) object;

	if (UNEXPECTED(!obj_ptr->ptr)) return;
	ev_timer *ptr = (ev_timer *) obj_ptr->ptr;

	/* Free base class members */
	php_ev_watcher_free_storage((ev_watcher *) ptr TSRMLS_CC);

	/* Free common Ev object members and the object itself */
	php_ev_object_free_storage(object TSRMLS_CC);
}
/* }}} */

#if EV_PERIODIC_ENABLE
/* {{{ php_ev_periodic_free_storage() */
static void php_ev_periodic_free_storage(void *object TSRMLS_DC)
{
	php_ev_object *obj_ptr = (php_ev_object *) object;

	if (UNEXPECTED(!obj_ptr->ptr)) return;
	ev_periodic *ptr              = (ev_periodic *) obj_ptr->ptr;
	php_ev_periodic *periodic_ptr = (php_ev_periodic *) obj_ptr->ptr;

	PHP_EV_FREE_FCALL_INFO(periodic_ptr->fci, periodic_ptr->fcc);

	/* Free base class members */
	php_ev_watcher_free_storage((ev_watcher *) ptr TSRMLS_CC);

	/* Free common Ev object members and the object itself */
	php_ev_object_free_storage(object TSRMLS_CC);
}
/* }}} */
#endif

#if EV_SIGNAL_ENABLE
/* {{{ php_ev_signal_free_storage() */
static void php_ev_signal_free_storage(void *object TSRMLS_DC)
{
	php_ev_object *obj_ptr = (php_ev_object *) object;

	if (UNEXPECTED(!obj_ptr->ptr)) return;
	ev_signal *ptr = (ev_signal *) obj_ptr->ptr;

	/* Free base class members */
	php_ev_watcher_free_storage((ev_watcher *) ptr TSRMLS_CC);

	/* Free common Ev object members and the object itself */
	php_ev_object_free_storage(object TSRMLS_CC);
}
/* }}} */
#endif

#if EV_CHILD_ENABLE
/* {{{ php_ev_child_free_storage() */
static void php_ev_child_free_storage(void *object TSRMLS_DC)
{
	php_ev_object *obj_ptr = (php_ev_object *) object;

	if (UNEXPECTED(!obj_ptr->ptr)) return;
	ev_child *ptr = (ev_child *) obj_ptr->ptr;

	/* Free base class members */
	php_ev_watcher_free_storage((ev_watcher *) ptr TSRMLS_CC);

	/* Free common Ev object members and the object itself */
	php_ev_object_free_storage(object TSRMLS_CC);
}
/* }}} */
#endif

#if EV_STAT_ENABLE
/* {{{ php_ev_stat_free_storage() */
static void php_ev_stat_free_storage(void *object TSRMLS_DC)
{
	php_ev_object *obj_ptr = (php_ev_object *) object;

	if (UNEXPECTED(!obj_ptr->ptr)) return;
	ev_stat *ptr          = (ev_stat *) obj_ptr->ptr;
	php_ev_stat *stat_ptr = (php_ev_stat *) obj_ptr->ptr;

	efree(stat_ptr->path);

	/* Free base class members */
	php_ev_watcher_free_storage((ev_watcher *) ptr TSRMLS_CC);

	/* Free common Ev object members and the object itself */
	php_ev_object_free_storage(object TSRMLS_CC);
}
/* }}} */
#endif

#if EV_IDLE_ENABLE
/* {{{ php_ev_idle_free_storage() */
static void php_ev_idle_free_storage(void *object TSRMLS_DC)
{
	php_ev_object *obj_ptr = (php_ev_object *) object;

	if (UNEXPECTED(!obj_ptr->ptr)) return;
	ev_idle *ptr = (ev_idle *) obj_ptr->ptr;

	/* Free base class members */
	php_ev_watcher_free_storage((ev_watcher *) ptr TSRMLS_CC);

	/* Free common Ev object members and the object itself */
	php_ev_object_free_storage(object TSRMLS_CC);
}
/* }}} */
#endif

#if EV_CHECK_ENABLE
/* {{{ php_ev_check_free_storage() */
static void php_ev_check_free_storage(void *object TSRMLS_DC)
{
	php_ev_object *obj_ptr = (php_ev_object *) object;

	if (UNEXPECTED(!obj_ptr->ptr)) return;
	ev_check *ptr = (ev_check *) obj_ptr->ptr;

	/* Free base class members */
	php_ev_watcher_free_storage((ev_watcher *) ptr TSRMLS_CC);

	/* Free common Ev object members and the object itself */
	php_ev_object_free_storage(object TSRMLS_CC);
}
/* }}} */
#endif

#if EV_PREPARE_ENABLE
/* {{{ php_ev_prepare_free_storage() */
static void php_ev_prepare_free_storage(void *object TSRMLS_DC)
{
	php_ev_object *obj_ptr = (php_ev_object *) object;

	if (UNEXPECTED(!obj_ptr->ptr)) return;
	ev_prepare *ptr = (ev_prepare *) obj_ptr->ptr;

	/* Free base class members */
	php_ev_watcher_free_storage((ev_watcher *) ptr TSRMLS_CC);

	/* Free common Ev object members and the object itself */
	php_ev_object_free_storage(object TSRMLS_CC);
}
/* }}} */
#endif

#if EV_EMBED_ENABLE
/* {{{ php_ev_embed_free_storage() */
static void php_ev_embed_free_storage(void *object TSRMLS_DC)
{
	php_ev_object *obj_ptr = (php_ev_object *) object;

	if (UNEXPECTED(!obj_ptr->ptr)) return;

	ev_embed *ptr           = (ev_embed *) obj_ptr->ptr;
	php_ev_embed *embed_ptr = (php_ev_embed *) obj_ptr->ptr;

	if (embed_ptr->other) {
		zval_ptr_dtor(&embed_ptr->other);
	}

	/* Free base class members */
	php_ev_watcher_free_storage((ev_watcher *) ptr TSRMLS_CC);

	/* Free common Ev object members and the object itself */
	php_ev_object_free_storage(object TSRMLS_CC);
}
/* }}} */
#endif

#if EV_FORK_ENABLE
/* {{{ php_ev_fork_free_storage() */
static void php_ev_fork_free_storage(void *object TSRMLS_DC)
{
	php_ev_object *obj_ptr = (php_ev_object *) object;

	if (UNEXPECTED(!obj_ptr->ptr)) return;
	ev_fork *ptr = (ev_fork *) obj_ptr->ptr;

	/* Free base class members */
	php_ev_watcher_free_storage((ev_watcher *) ptr TSRMLS_CC);

	/* Free common Ev object members and the object itself */
	php_ev_object_free_storage(object TSRMLS_CC);
}
/* }}} */
#endif

/* {{{ php_ev_register_object 
 * Is called AFTER php_ev_object_new() */
zend_object_value php_ev_register_object(zend_class_entry *ce, php_ev_object *intern TSRMLS_DC)
{
	zend_object_value                  retval;
	zend_objects_free_object_storage_t func_free_storage;

	if (instanceof_function(ce, ev_loop_class_entry_ptr TSRMLS_CC)) {
		/* EvLoop */
	 	func_free_storage = php_ev_loop_free_storage;
	} else if (instanceof_function(ce, ev_io_class_entry_ptr TSRMLS_CC)) {
		/* EvIo */
	 	func_free_storage = php_ev_io_free_storage;
	} else if (instanceof_function(ce, ev_timer_class_entry_ptr TSRMLS_CC)) {
		/* EvTimer */
	 	func_free_storage = php_ev_timer_free_storage;
#if EV_PERIODIC_ENABLE
	} else if (instanceof_function(ce, ev_periodic_class_entry_ptr TSRMLS_CC)) {
		/* EvPeriodic */
	 	func_free_storage = php_ev_periodic_free_storage;
#endif
#if EV_SIGNAL_ENABLE
	} else if (instanceof_function(ce, ev_signal_class_entry_ptr TSRMLS_CC)) {
		/* EvSignal*/
	 	func_free_storage = php_ev_signal_free_storage;
#endif
#if EV_CHILD_ENABLE
	} else if (instanceof_function(ce, ev_child_class_entry_ptr TSRMLS_CC)) {
		/* EvChild */
	 	func_free_storage = php_ev_child_free_storage;
#endif
#if EV_STAT_ENABLE
	} else if (instanceof_function(ce, ev_stat_class_entry_ptr TSRMLS_CC)) {
		/* EvStat */
	 	func_free_storage = php_ev_stat_free_storage;
#endif
#if EV_IDLE_ENABLE
	} else if (instanceof_function(ce, ev_idle_class_entry_ptr TSRMLS_CC)) {
		/* EvIdle */
	 	func_free_storage = php_ev_idle_free_storage;
#endif
#if EV_CHECK_ENABLE
	} else if (instanceof_function(ce, ev_check_class_entry_ptr TSRMLS_CC)) {
		/* EvCheck */
	 	func_free_storage = php_ev_check_free_storage;
#endif
#if EV_PREPARE_ENABLE
	} else if (instanceof_function(ce, ev_prepare_class_entry_ptr TSRMLS_CC)) {
		/* EvPrepare */
	 	func_free_storage = php_ev_prepare_free_storage;
#endif
#if EV_EMBED_ENABLE
	} else if (instanceof_function(ce, ev_embed_class_entry_ptr TSRMLS_CC)) {
		/* EvEmbed */
	 	func_free_storage = php_ev_embed_free_storage;
#endif
#if EV_FORK_ENABLE
	} else if (instanceof_function(ce, ev_fork_class_entry_ptr TSRMLS_CC)) {
		/* EvCheck */
	 	func_free_storage = php_ev_fork_free_storage;
#endif
	} else {
	 	func_free_storage = php_ev_object_free_storage;
	}

	retval.handle = zend_objects_store_put(intern,
			(zend_objects_store_dtor_t) zend_objects_destroy_object,
			(zend_objects_free_object_storage_t) func_free_storage,
			NULL TSRMLS_CC);
	retval.handlers = &ev_object_handlers;

	return retval;
}
/* }}} */

/* {{{ php_ev_object_new
 * Custom Ev object. php_ev_register_object() must be called AFTER */
php_ev_object *php_ev_object_new(zend_class_entry *ce TSRMLS_DC)
{
	php_ev_object    *intern;
	zend_class_entry *ce_parent = ce;

	intern               = ecalloc(1, sizeof(php_ev_object));
	intern->ptr          = NULL;
	intern->prop_handler = NULL;

	while (ce_parent->type != ZEND_INTERNAL_CLASS && ce_parent->parent != NULL) {
		ce_parent = ce_parent->parent;
	}
	zend_hash_find(&classes, ce_parent->name, ce_parent->name_length + 1,
	        (void **) &intern->prop_handler);

	zend_object_std_init(&intern->zo, ce TSRMLS_CC);
	object_properties_init(&intern->zo, ce);

	return intern;
}
/* }}} */

/* {{{ php_ev_object_create
 Custom Ev object(class instance) creator */
zend_object_value php_ev_object_create(zend_class_entry *ce TSRMLS_DC)
{
	php_ev_object *intern;

	intern = php_ev_object_new(ce TSRMLS_CC);
	return php_ev_register_object(ce, intern TSRMLS_CC);
}
/* }}} */

/* {{{ php_ev_register_classes
 * Registers all Ev classes. Should be called in MINIT */
static inline void php_ev_register_classes(TSRMLS_D)
{
	zend_class_entry *ce;

	/* {{{ Ev */
	PHP_EV_REGISTER_CLASS_ENTRY("Ev", ev_class_entry_ptr, ev_class_entry_functions);
	ce = ev_class_entry_ptr;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
	/* }}} */

	/* {{{ EvLoop */
	PHP_EV_REGISTER_CLASS_ENTRY("EvLoop", ev_loop_class_entry_ptr, ev_loop_class_entry_functions);
	ce = ev_loop_class_entry_ptr;
	ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
	zend_hash_init(&php_ev_properties, 0, NULL, NULL, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_properties, ev_loop_property_entries);
	PHP_EV_DECL_CLASS_PROPERTIES(ce, ev_loop_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_properties, sizeof(php_ev_properties), NULL);
	/* }}} */

	/* {{{ EvWatcher */
	PHP_EV_REGISTER_CLASS_ENTRY("EvWatcher", ev_watcher_class_entry_ptr, ev_watcher_class_entry_functions);
	ce = ev_watcher_class_entry_ptr;
	ce->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	zend_hash_init(&php_ev_watcher_properties, 0, NULL, NULL, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_watcher_properties, ev_watcher_property_entries);
	PHP_EV_DECL_CLASS_PROPERTIES(ce, ev_watcher_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_watcher_properties, sizeof(php_ev_watcher_properties), NULL);
	/* }}} */

	/* {{{ EvIo */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvIo", ev_io_class_entry_ptr, ev_io_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_io_class_entry_ptr;
	zend_hash_init(&php_ev_io_properties, 0, NULL, NULL, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_io_properties, ev_io_property_entries);
	zend_hash_merge(&php_ev_io_properties, &php_ev_watcher_properties, NULL, NULL, sizeof(php_ev_prop_handler), 0);
	PHP_EV_DECL_CLASS_PROPERTIES(ce, ev_io_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_io_properties, sizeof(php_ev_io_properties), NULL);
	/* }}} */

	/* {{{ EvTimer */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvTimer", ev_timer_class_entry_ptr, ev_timer_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_timer_class_entry_ptr;
	zend_hash_init(&php_ev_timer_properties, 0, NULL, NULL, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_timer_properties, ev_timer_property_entries);
	zend_hash_merge(&php_ev_timer_properties, &php_ev_watcher_properties, NULL, NULL, sizeof(php_ev_prop_handler), 0);
	PHP_EV_DECL_CLASS_PROPERTIES(ce, ev_timer_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_timer_properties, sizeof(php_ev_timer_properties), NULL);
	/* }}} */

#if EV_PERIODIC_ENABLE
	/* {{{ EvPeriodic */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvPeriodic", ev_periodic_class_entry_ptr, ev_periodic_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_periodic_class_entry_ptr;
	zend_hash_init(&php_ev_periodic_properties, 0, NULL, NULL, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_periodic_properties, ev_periodic_property_entries);
	zend_hash_merge(&php_ev_periodic_properties, &php_ev_watcher_properties, NULL, NULL, sizeof(php_ev_prop_handler), 0);
	PHP_EV_DECL_CLASS_PROPERTIES(ce, ev_periodic_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_periodic_properties, sizeof(php_ev_periodic_properties), NULL);
	/* }}} */
#endif

#if EV_SIGNAL_ENABLE
	/* {{{ EvSignal*/
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvSignal", ev_signal_class_entry_ptr, ev_signal_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_signal_class_entry_ptr;
	zend_hash_init(&php_ev_signal_properties, 0, NULL, NULL, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_signal_properties, ev_signal_property_entries);
	zend_hash_merge(&php_ev_signal_properties, &php_ev_watcher_properties, NULL, NULL, sizeof(php_ev_prop_handler), 0);
	PHP_EV_DECL_CLASS_PROPERTIES(ce, ev_signal_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_signal_properties, sizeof(php_ev_signal_properties), NULL);
	/* }}} */
#endif

#if EV_CHILD_ENABLE
	/* {{{ EvChild*/
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvChild", ev_child_class_entry_ptr, ev_child_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_child_class_entry_ptr;
	zend_hash_init(&php_ev_child_properties, 0, NULL, NULL, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_child_properties, ev_child_property_entries);
	zend_hash_merge(&php_ev_child_properties, &php_ev_watcher_properties, NULL, NULL, sizeof(php_ev_prop_handler), 0);
	PHP_EV_DECL_CLASS_PROPERTIES(ce, ev_child_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_child_properties, sizeof(php_ev_child_properties), NULL);
	/* }}} */
#endif

#if EV_STAT_ENABLE
	/* {{{ EvStat */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvStat", ev_stat_class_entry_ptr, ev_stat_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_stat_class_entry_ptr;
	zend_hash_init(&php_ev_stat_properties, 0, NULL, NULL, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_stat_properties, ev_stat_property_entries);
	zend_hash_merge(&php_ev_stat_properties, &php_ev_watcher_properties, NULL, NULL, sizeof(php_ev_prop_handler), 0);
	PHP_EV_DECL_CLASS_PROPERTIES(ce, ev_stat_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_stat_properties, sizeof(php_ev_stat_properties), NULL);
	/* }}} */
#endif

#if EV_IDLE_ENABLE
	/* {{{ EvIdle */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvIdle", ev_idle_class_entry_ptr, ev_idle_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_idle_class_entry_ptr;
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_watcher_properties, sizeof(php_ev_watcher_properties), NULL);
	/* }}} */
#endif

#if EV_CHECK_ENABLE
	/* {{{ EvCheck */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvCheck", ev_check_class_entry_ptr, ev_check_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_check_class_entry_ptr;
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_watcher_properties, sizeof(php_ev_watcher_properties), NULL);
	/* }}} */
#endif

#if EV_PREPARE_ENABLE
	/* {{{ EvPrepare */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvPrepare", ev_prepare_class_entry_ptr, ev_prepare_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_prepare_class_entry_ptr;
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_watcher_properties, sizeof(php_ev_watcher_properties), NULL);
	/* }}} */
#endif

#if EV_EMBED_ENABLE
	/* {{{ EvEmbed */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvEmbed", ev_embed_class_entry_ptr, ev_embed_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_embed_class_entry_ptr;
	zend_hash_init(&php_ev_embed_properties, 0, NULL, NULL, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_embed_properties, ev_embed_property_entries);
	zend_hash_merge(&php_ev_embed_properties, &php_ev_watcher_properties, NULL, NULL, sizeof(php_ev_prop_handler), 0);
	PHP_EV_DECL_CLASS_PROPERTIES(ce, ev_embed_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_embed_properties, sizeof(php_ev_embed_properties), NULL);
	/* }}} */
#endif

#if EV_CHECK_ENABLE
	/* {{{ EvFork */
	PHP_EV_REGISTER_CLASS_ENTRY_EX("EvFork", ev_fork_class_entry_ptr, ev_fork_class_entry_functions, ev_watcher_class_entry_ptr);
	ce = ev_fork_class_entry_ptr;
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_watcher_properties, sizeof(php_ev_watcher_properties), NULL);
	/* }}} */
#endif
}
/* }}} */


/* Private functions }}} */


/* {{{ PHP_GINIT_FUNCTION */ 
static PHP_GINIT_FUNCTION(ev)
{
	ev_globals->default_loop = NULL;
	memset(ev_globals->signal_loops, 0, sizeof(ev_globals->signal_loops));
}
/* }}} */


/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(ev)
{
	zend_object_handlers *std_hnd = zend_get_std_object_handlers();

	memcpy(&ev_object_handlers, std_hnd, sizeof(zend_object_handlers));

	ev_object_handlers.clone_obj            = NULL; /* TODO: add __clone() handler */
	ev_object_handlers.read_property        = php_ev_read_property;
	ev_object_handlers.write_property       = php_ev_write_property;
	ev_object_handlers.get_property_ptr_ptr = php_ev_get_property_ptr_ptr; /* std_hnd->get_property_ptr_ptr */;
	ev_object_handlers.has_property         = php_ev_has_property;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	ev_object_handlers.get_debug_info       = php_ev_object_get_debug_info;
#endif
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 4
	ev_object_handlers.get_properties       = get_properties;
#endif

	zend_hash_init(&classes, 0, NULL, NULL, 1);
	php_ev_register_classes(TSRMLS_C);

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

#if !defined(_WIN32) && !defined(_MINIX)
	pthread_atfork(0, 0, php_ev_default_fork);
#endif

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
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_RSHUTDOWN_FUNCTION */
PHP_RSHUTDOWN_FUNCTION(ev)
{
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

#define PHP_EV_METHOD_INT_VOID(name, method)                                      \
    PHP_METHOD(Ev, method)                                                        \
    {                                                                             \
        if (zend_parse_parameters_none() == FAILURE) {                            \
            return;                                                               \
        }                                                                         \
                                                                                  \
        RETURN_LONG((long) ev_ ## name());                                        \
    }

#define PHP_EV_METHOD_DEFAULT_LOOP_INT_VOID(name, method)                         \
    PHP_METHOD(Ev, method)                                                        \
    {                                                                             \
        zval          *zloop;                                                     \
        php_ev_object *ev_obj;                                                    \
                                                                                  \
        if (zend_parse_parameters_none() == FAILURE) {                            \
            return;                                                               \
        }                                                                         \
                                                                                  \
        zloop  = php_ev_default_loop(TSRMLS_C);                                   \
        ev_obj = (php_ev_object *) zend_object_store_get_object(zloop TSRMLS_CC); \
                                                                                  \
        PHP_EV_CONSTRUCT_CHECK(ev_obj);                                           \
                                                                                  \
        RETURN_LONG(ev_ ## name(PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj)));          \
    }

#define PHP_EV_METHOD_VOID_VOID(name, method)                                     \
    PHP_METHOD(Ev, method)                                                        \
    {                                                                             \
        zval          *zloop;                                                     \
        php_ev_object *ev_obj;                                                    \
                                                                                  \
        if (zend_parse_parameters_none() == FAILURE) {                            \
            return;                                                               \
        }                                                                         \
                                                                                  \
        zloop  = php_ev_default_loop(TSRMLS_C);                                   \
        ev_obj = (php_ev_object *) zend_object_store_get_object(zloop TSRMLS_CC); \
                                                                                  \
        PHP_EV_CONSTRUCT_CHECK(ev_obj);                                           \
                                                                                  \
        ev_ ## name(PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj));                       \
    }

PHP_EV_METHOD_INT_VOID(supported_backends,   supportedBackends)
PHP_EV_METHOD_INT_VOID(recommended_backends, recommendedBackends)
PHP_EV_METHOD_INT_VOID(embeddable_backends,  embeddableBackends)

/* {{{ proto void Ev::feedSignal(int signum) */
PHP_METHOD(Ev, feedSignal)
{
	long signum;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &signum) == FAILURE) {
		return;
	}

	ev_feed_signal(signum);
}
/* }}} */


/* {{{ proto void Ev::feedSignalEvent(int signum) */
PHP_METHOD(Ev, feedSignalEvent)
{
	long			signum;
	php_ev_object	*ev_obj;
	zval			**default_loop_ptr_ptr = &MyG(default_loop);
	
	if (!*default_loop_ptr_ptr) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"The default loop is not initialized");
		return;
	}

	/* Fetch the default loop */
	ev_obj = (php_ev_object *) zend_object_store_get_object(*default_loop_ptr_ptr TSRMLS_CC);
	EV_P = PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &signum) == FAILURE) {
		return;
	}

	ev_feed_signal_event(EV_A_ signum);
}
/* }}} */

/* {{{ proto void Ev::sleep(double seconds) */
PHP_METHOD(Ev, sleep)
{
	double seconds;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &seconds) == FAILURE) {
		return;
	}

	ev_sleep(seconds);
}
/* }}} */

/* {{{ proto double Ev::time(void) */
PHP_METHOD(Ev, time)
{
    if (zend_parse_parameters_none() == FAILURE) {
        return;
    }

    RETURN_DOUBLE((double) ev_time());
}
/* }}} */

/* {{{ proto void Ev::run([int flags = 0]) */
PHP_METHOD(Ev, run)
{
	long           flags  = 0;
	zval          *zloop;
	php_ev_object *ev_obj;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &flags) == FAILURE) {
		return;
	}

	zloop  = php_ev_default_loop(TSRMLS_C);
	ev_obj = (php_ev_object *) zend_object_store_get_object(zloop TSRMLS_CC);

	PHP_EV_CONSTRUCT_CHECK(ev_obj);

	ev_run(PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj), flags);
}
/* }}} */

/* {{{ proto double Ev::now(void) */
PHP_METHOD(Ev, now)
{
	zval          *zloop;
	php_ev_object *ev_obj;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	zloop  = php_ev_default_loop(TSRMLS_C);
	ev_obj = (php_ev_object *) zend_object_store_get_object(zloop TSRMLS_CC);

	PHP_EV_CONSTRUCT_CHECK(ev_obj);

	RETURN_DOUBLE((double) ev_now(PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj)));
}
/* }}} */

/* {{{ proto void Ev::stop([int how = 0]) */
PHP_METHOD(Ev, stop)
{
	long  how   = EVBREAK_ONE;
	zval *zloop;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &how) == FAILURE) {
		return;
	}

	zloop = php_ev_default_loop(TSRMLS_C);

	php_ev_object *ev_obj = (php_ev_object *) zend_object_store_get_object(zloop TSRMLS_CC);
	PHP_EV_CONSTRUCT_CHECK(ev_obj);

	ev_break(PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj), how);
}
/* }}} */

/* {{{ proto void Ev::verify(void) */
PHP_METHOD(Ev, verify)
{
	zval *zloop;
	php_ev_object *ev_obj;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	zloop  = php_ev_default_loop(TSRMLS_C);
	ev_obj = (php_ev_object *) zend_object_store_get_object(zloop TSRMLS_CC);

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
