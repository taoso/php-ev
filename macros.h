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

#ifndef PHP_EV_MACROS_H
# define PHP_EV_MACROS_H

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

#ifdef PHP_EV_DEBUG
# define PHP_EV_ASSERT(x) assert(x)
#else
# define PHP_EV_ASSERT(x)
#endif

#ifdef ZTS
# define MyG(v)                     TSRMG(ev_globals_id, zend_ev_globals *, v)
# define TSRMLS_FETCH_FROM_CTX(ctx) void ***tsrm_ls = (void ***) ctx
# define TSRMLS_SET_CTX(ctx)        ctx = (void ***) tsrm_ls
#else
# define MyG(v)                     (ev_globals.v)
# define TSRMLS_FETCH_FROM_CTX(ctx)
# define TSRMLS_SET_CTX(ctx)
#endif


#define PHP_EV_REGISTER_LONG_CONSTANT(name) \
	REGISTER_LONG_CONSTANT(#name, name, CONST_CS | CONST_PERSISTENT)

#define PHP_EV_REGISTER_CLASS_LONG_CONSTANT(name, zconst)     \
    zconst = pemalloc(sizeof(zval), 1);                       \
    INIT_PZVAL(zconst);                                       \
    ZVAL_LONG(zconst, name);                                  \
    zend_hash_add(&ce->constants_table, #name, sizeof(#name), \
            (void*) &zconst, sizeof(zval*), NULL);

#define PHP_EV_REGISTER_CLASS_ENTRY(name, ce, ce_functions) \
{                                                           \
    zend_class_entry tmp_ce;                                \
    INIT_CLASS_ENTRY(tmp_ce, name, ce_functions);           \
    ce = zend_register_internal_class(&tmp_ce TSRMLS_CC);   \
    ce->create_object = php_ev_object_create;               \
}

#define PHP_EV_REGISTER_CLASS_ENTRY_EX(name, ce, ce_functions, parent_ce)       \
{                                                                               \
    zend_class_entry tmp_ce;                                                    \
    INIT_CLASS_ENTRY_EX(tmp_ce, name, sizeof(name) - 1, ce_functions);          \
    ce = zend_register_internal_class_ex(&tmp_ce, parent_ce, NULL TSRMLS_CC);   \
    ce->create_object = parent_ce->create_object; /*php_ev_object_create; */    \
}

#define PHP_EV_INIT_CLASS_OBJECT(pz, pce) \
        Z_TYPE_P(pz) = IS_OBJECT;         \
        object_init_ex(pz, pce);          \
        Z_SET_REFCOUNT_P(pz, 1);          \
        Z_SET_ISREF_P(pz)

#define PHP_EV_ADD_CLASS_PROPERTIES(a, b)                                                      \
{                                                                                              \
    int i = 0;                                                                                 \
    while (b[i].name != NULL) {                                                                \
        php_ev_add_property((a), (b)[i].name, (b)[i].name_length,                              \
                (php_ev_read_t)(b)[i].read_func, (php_ev_write_t)(b)[i].write_func TSRMLS_CC); \
        i++;                                                                                   \
    }                                                                                          \
}

#define PHP_EV_DECL_CLASS_PROPERTIES(a, b)                                                           \
{                                                                                                    \
    int i = 0;                                                                                       \
    while (b[i].name != NULL) {                                                                      \
        zend_declare_property_null((a), (b)[i].name, (b)[i].name_length, ZEND_ACC_PUBLIC TSRMLS_CC); \
        i++;                                                                                         \
    }                                                                                                \
}

#define PHP_EV_CONSTRUCT_CHECK(ev_obj)                                        \
    if (!ev_obj->ptr) {                                                       \
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Loop is not initialized"); \
        return;                                                               \
    }

#if PHP_VERSION_ID >= 50300
# define PHP_EV_FCI_ADDREF(pfci)          \
{                                         \
	Z_ADDREF_P(pfci->function_name);      \
    if (pfci->object_ptr) {               \
        Z_ADDREF_P(pfci->object_ptr);     \
    }                                     \
}
# define PHP_EV_FCI_DELREF(pfci)          \
{                                         \
	zval_ptr_dtor(&pfci->function_name);  \
    if (pfci->object_ptr) {               \
        zval_ptr_dtor(&pfci->object_ptr); \
    }                                     \
}
#else
# define PHP_EV_FCI_ADDREF(pfci) Z_ADDREF_P(pfci_dst->function_name)
# define PHP_EV_FCI_DELREF(pfci) zval_ptr_dtor(&pfci->function_name)
#endif

#define PHP_EV_COPY_FCALL_INFO(pfci_dst, pfcc_dst, pfci, pfcc)                                  \
    if (ZEND_FCI_INITIALIZED(*pfci)) {                                                          \
        pfci_dst = (zend_fcall_info *) safe_emalloc(1, sizeof(zend_fcall_info), 0);             \
        pfcc_dst = (zend_fcall_info_cache *) safe_emalloc(1, sizeof(zend_fcall_info_cache), 0); \
                                                                                                \
        memcpy(pfci_dst, pfci, sizeof(zend_fcall_info));                                        \
        memcpy(pfcc_dst, pfcc, sizeof(zend_fcall_info_cache));                                  \
                                                                                                \
        PHP_EV_FCI_ADDREF(pfci_dst);                                                            \
    } else {                                                                                    \
        pfci_dst = NULL;                                                                        \
        pfcc_dst = NULL;                                                                        \
    }                                                                                           \

#define PHP_EV_FREE_FCALL_INFO(pfci, pfcc)       \
    if (pfci && pfcc) {                          \
        efree(pfcc);                             \
                                                 \
        if (ZEND_FCI_INITIALIZED(*pfci)) {       \
            PHP_EV_FCI_DELREF(pfci);             \
        }                                        \
        efree(pfci);                             \
    }                                            \

#define PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj) (obj ? (php_ev_loop *) obj->ptr : NULL)
#define PHP_EV_WATCHER_FETCH_FROM_OBJECT(o)       ((ev_watcher *) o->ptr)
#define PHP_EV_WATCHER_FETCH_FROM_THIS()          \
	(PHP_EV_WATCHER_FETCH_FROM_OBJECT(((php_ev_object *) zend_object_store_get_object(getThis() TSRMLS_CC))))

#define PHP_EV_LOOP_FETCH_FROM_OBJECT(obj) (obj ? PHP_EV_LOOP_OBJECT_FETCH_FROM_OBJECT(obj)->loop : NULL)
#define PHP_EV_LOOP_FETCH_FROM_THIS                                                             \
    php_ev_object *ev_obj = (php_ev_object *)zend_object_store_get_object(getThis() TSRMLS_CC); \
    PHP_EV_CONSTRUCT_CHECK(ev_obj);                                                             \
    EV_P = PHP_EV_LOOP_FETCH_FROM_OBJECT(ev_obj) /* no ';' */

#define PHP_EV_CHECK_PENDING_WATCHER(w)              \
    if (ev_is_pending(w)) {                          \
        php_error_docref(NULL TSRMLS_CC, E_ERROR,    \
                "Failed modifying pending watcher"); \
        return;                                      \
    }                                                \

#define PHP_EV_EXIT_LOOP(__loop) ev_break((__loop), EVBREAK_ALL)


#define PHP_EV_PROP_ZVAL_READ(data)          \
    do {                                     \
        if (!data) {                         \
            ALLOC_INIT_ZVAL(*retval);        \
            return SUCCESS;                  \
        }                                    \
                                             \
        MAKE_STD_ZVAL(*retval);              \
        REPLACE_ZVAL_VALUE(retval, data, 1); \
    } while (0)

#define PHP_EV_PROP_ZVAL_WRITE(ppz)                                     \
    do {                                                                \
        /* Make a copy of the zval, avoid direct binding to the address \
         * of value, since it breaks refcount in php_ev_read_property() \
         * causing further leaks and memory access violations */        \
        if (!*ppz) {                                                    \
            MAKE_STD_ZVAL(*ppz);                                        \
        }                                                               \
        REPLACE_ZVAL_VALUE(ppz, value, 1);                              \
    } while (0)

#define PHP_EV_CHECK_REPEAT(repeat)                                                 \
    if (repeat < 0.) {                                                              \
        php_error_docref(NULL TSRMLS_CC, E_ERROR, # repeat " value must be >= 0."); \
        return;                                                                     \
    }                                                                               \

#define PHP_EV_CHECK_REPEAT_RET(repeat, ret)                                        \
    if (repeat < 0.) {                                                              \
        php_error_docref(NULL TSRMLS_CC, E_ERROR, # repeat " value must be >= 0."); \
        return (ret);                                                               \
    }                                                                               \

#define PHP_EV_CHECK_SIGNUM(num)                                     \
    if ((num) < 0) {                                                 \
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "invalid signum"); \
        return;                                                      \
    }                                                                \


#endif /* PHP_EV_MACROS_H*/

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
