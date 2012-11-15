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
#ifndef PHP_EV_EMBED_H
#define PHP_EV_EMBED_H

#include "common.h"
#include "types.h"

/* EV_STANDALONE isn't needed, since we use libev/libev.m4
 * #define EV_STANDALONE 1 */
#undef EV_USE_POLL
/* We compile multiple source files. So we don't need static API
 * #define EV_API_STATIC 1 */
#undef EV_API_STATIC

#define EV_COMPAT3      0
#define EV_FEATURES     8
#define EV_MULTIPLICITY 1
#define EV_USE_POLL     1
#define EV_CHILD_ENABLE 1
#define EV_ASYNC_ENABLE 1
#define EV_MINPRI       -2
#define EV_MAXPRI       2

#ifdef PHP_EV_DEBUG
# define EV_VERIFY 2
#else
# define EV_VERIFY 0
#endif

/* Thread context. With it we are getting rid of need 
 * to call the heavy TSRMLS_FETCH() */
#ifdef ZTS
# define PHP_EV_COMMON_THREAD_CTX void ***thread_ctx
#else
# define PHP_EV_COMMON_THREAD_CTX
#endif

/* Override `data` member of the watcher structs.
 * See types.h and libev/ev.h */
#define EV_COMMON                                                                    \
    zval                        *self;      /* this struct */                        \
    zval                        *data;      /* custom var attached by user */        \
    php_ev_loop                 *loop;                                               \
    zend_fcall_info             *fci;       /* fci &fcc serve $callback arg */       \
    zend_fcall_info_cache       *fcc;                                                \
    int                          type;      /* EV_ *constant from libev/ev.h */      \
    int                          e_flags;                                            \
    void                        *e_prev;    /* Linked list of ev_watcher pointers */ \
    PHP_EV_COMMON_THREAD_CTX;                                                        \

#include "libev/ev.h"

#endif /* PHP_EV_EMBED_H */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
