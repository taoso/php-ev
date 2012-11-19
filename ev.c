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

#include "php_ev.h"

#if HAVE_EV

ZEND_DECLARE_MODULE_GLOBALS(ev)
static PHP_GINIT_FUNCTION(ev);

zend_class_entry *ev_loop_class_entry_ptr;
zend_class_entry *ev_watcher_class_entry_ptr;
zend_class_entry *ev_io_class_entry_ptr;
zend_class_entry *ev_timer_class_entry_ptr;
#if EV_PERIODIC_ENABLE
zend_class_entry *ev_periodic_class_entry_ptr;
#endif

static HashTable classes;
static HashTable php_ev_properties;
static HashTable php_ev_watcher_properties;
static HashTable php_ev_io_properties;
static HashTable php_ev_timer_properties;
#if EV_PERIODIC_ENABLE
static HashTable php_ev_periodic_properties;
#endif

static zend_object_handlers ev_object_handlers;

/* {{{ ev_module_entry */
zend_module_entry ev_module_entry = {
#if ZEND_MODULE_API_NO >= 20050922
	STANDARD_MODULE_HEADER_EX, NULL,
	NULL,
#elif ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
    "Ev",
    ev_functions,
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
static void php_ev_add_property(HashTable *h, const char *name, size_t name_len, php_ev_read_t read_func, php_ev_write_t write_func TSRMLS_DC) {
	php_ev_prop_handler p;

	p.name       = (char *) name;
	p.name_len   = name_len;
	p.read_func  = (read_func) ? read_func : php_ev_prop_read_default;
	p.write_func = (write_func) ? write_func: php_ev_prop_write_default;
	zend_hash_add(h, name, name_len + 1, &p, sizeof(php_ev_prop_handler), NULL);
}
/* }}} */

/* {{{ php_ev_read_property */
static zval *php_ev_read_property(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC)
{
	zval            tmp_member;
	zval            *retval;
	php_ev_object   *obj;
	php_ev_prop_handler *hnd;
	int             ret;

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
void php_ev_write_property(zval *object, zval *member, zval *value, const zend_literal *key TSRMLS_DC)
{
	zval             tmp_member;
	php_ev_object   *obj;
	php_ev_prop_handler *hnd;
	int              ret;

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
	    if (! PZVAL_IS_REF(value) && Z_REFCOUNT_P(value) == 0) {
	        Z_ADDREF_P(value);
	        zval_ptr_dtor(&value);
	    }
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
	php_ev_object   *obj = (php_ev_object *) zend_objects_get_address(object TSRMLS_CC);
	int              ret = 0;
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


/* {{{ php_ev_object_free_storage 
 * Common Ev object cleaner */
static void php_ev_object_free_storage(void *obj_ TSRMLS_DC)
{
	php_ev_object *obj = (php_ev_object *) obj_;

	zend_object_std_dtor(&obj->zo TSRMLS_CC);

	if (obj->ptr) {
		efree(obj->ptr);
	}

	efree(obj);
}
/* }}} */

/* {{{ php_ev_loop_free_storage */
static void php_ev_loop_free_storage(void *obj_ TSRMLS_DC)
{
	php_ev_object *obj = (php_ev_object *) obj_;

	PHP_EV_ASSERT(obj->ptr);
	php_ev_loop *ptr = (php_ev_loop *) obj->ptr;

	if (ptr->loop) {
		/* Stop all watchers associated with this loop.  But don't free their memory. 
		 * They have special automatically called handlers for this purpose */
		ev_watcher *w = ptr->w;
		while (w) {
			php_ev_stop_watcher(w TSRMLS_CC);
			w = php_ev_watcher_prev(w);
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

	PHP_EV_FREE_FCALL_INFO(ptr->fci, ptr->fcc);

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
	zval *data, *self;

	/* This is done in php_ev_loop_free_storage()
	php_ev_stop_watcher(ptr TSRMLS_CC);
	*/

	PHP_EV_FREE_FCALL_INFO(php_ev_watcher_fci(ptr), php_ev_watcher_fcc(ptr));
	
	data = php_ev_watcher_data(ptr);
	self = php_ev_watcher_self(ptr);

	if (data) {
		zval_ptr_dtor(&data);
	}
	zval_ptr_dtor(&self);
}
/* }}} */

/* {{{ php_ev_io_free_storage() */
static void php_ev_io_free_storage(void *object TSRMLS_DC)
{
	php_ev_object *obj_ptr = (php_ev_object *) object;

	PHP_EV_ASSERT(obj_ptr->ptr);
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

	PHP_EV_ASSERT(obj_ptr->ptr);
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

	PHP_EV_ASSERT(obj_ptr->ptr);
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
	} else {
	 	func_free_storage = php_ev_object_free_storage;
	}

	retval.handle = zend_objects_store_put(intern,
			(zend_objects_store_dtor_t)zend_objects_destroy_object,
			(zend_objects_free_object_storage_t)func_free_storage,
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

#if 0
	while (ce_parent) {
	    if (ce_parent == ev_watcher_class_entry_ptr
	    		|| ce_parent == ev_loop_class_entry_ptr) {
	    	break;
	    }
	    ce_parent = ce_parent->parent;
	}
#endif
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

	/* {{{ EvLoop */
	PHP_EV_REGISTER_CLASS_ENTRY("EvLoop", ev_loop_class_entry_ptr, ev_loop_class_entry_functions);
	ce = ev_loop_class_entry_ptr;
	zend_hash_init(&php_ev_properties, 0, NULL, NULL, 1);
	PHP_EV_ADD_CLASS_PROPERTIES(&php_ev_properties, ev_loop_property_entries);
	PHP_EV_DECL_CLASS_PROPERTIES(ce, ev_loop_property_entry_info);
	zend_hash_add(&classes, ce->name, ce->name_length + 1, &php_ev_properties, sizeof(php_ev_properties), NULL);
	/* }}} */

	/* {{{ EvWatcher */
	PHP_EV_REGISTER_CLASS_ENTRY("EvWatcher", ev_watcher_class_entry_ptr, ev_watcher_class_entry_functions);
	ce = ev_watcher_class_entry_ptr;
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

}
/* }}} */

/* Private functions }}} */


/* {{{ PHP_GINIT_FUNCTION */ 
static PHP_GINIT_FUNCTION(ev)
{
	ev_globals->default_loop = NULL;
}
/* }}} */


/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(ev)
{
	zend_object_handlers *std_hnd = zend_get_std_object_handlers();

	memcpy(&ev_object_handlers, std_hnd, sizeof(zend_object_handlers));

	ev_object_handlers.clone_obj            = NULL;
	ev_object_handlers.read_property        = php_ev_read_property;
	ev_object_handlers.write_property       = php_ev_write_property;
	ev_object_handlers.get_property_ptr_ptr = std_hnd->get_property_ptr_ptr;
	ev_object_handlers.has_property         = php_ev_has_property;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 3
	ev_object_handlers.get_debug_info       = php_ev_object_get_debug_info;
#endif

	zend_hash_init(&classes, 0, NULL, NULL, 1);
	php_ev_register_classes(TSRMLS_C);


	/* {{{ EvLoop flags */
	PHP_EV_REGISTER_LONG_CONSTANT(EVFLAG_AUTO);
	PHP_EV_REGISTER_LONG_CONSTANT(EVFLAG_NOENV);
	PHP_EV_REGISTER_LONG_CONSTANT(EVFLAG_FORKCHECK);
	PHP_EV_REGISTER_LONG_CONSTANT(EVFLAG_NOINOTIFY);
	PHP_EV_REGISTER_LONG_CONSTANT(EVFLAG_SIGNALFD);
	PHP_EV_REGISTER_LONG_CONSTANT(EVFLAG_NOSIGMASK);

	PHP_EV_REGISTER_LONG_CONSTANT(EVRUN_NOWAIT);
	PHP_EV_REGISTER_LONG_CONSTANT(EVRUN_ONCE);

	PHP_EV_REGISTER_LONG_CONSTANT(EVBREAK_CANCEL);
	PHP_EV_REGISTER_LONG_CONSTANT(EVBREAK_ONE);
	PHP_EV_REGISTER_LONG_CONSTANT(EVBREAK_ALL);
	/* }}} */

	PHP_EV_REGISTER_LONG_CONSTANT(EV_MINPRI);
	PHP_EV_REGISTER_LONG_CONSTANT(EV_MAXPRI);

	PHP_EV_REGISTER_LONG_CONSTANT(EV_READ);
	PHP_EV_REGISTER_LONG_CONSTANT(EV_WRITE);

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
#ifdef PHP_EV_LIBEVENT_API
	php_info_print_table_row(2, "Libevent compatibility API support", "enabled");
#else
	php_info_print_table_row(2, "Libevent compatibility API support", "disabled");
#endif
	php_info_print_table_row(2, "Version", PHP_EV_VERSION);
	php_info_print_table_end();
}
/* }}} */


#include "loop.c" 
#include "io.c"
#include "timer.c"
#if EV_PERIODIC_ENABLE
# include "periodic.c"
#endif

#endif /* HAVE_EV */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
