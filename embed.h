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

#define EV_MULTIPLICITY 1
#define EV_COMPAT3      0
#define EV_MINPRI       -2
#define EV_MAXPRI       2

#undef EV_FEATURES
/* We compile multiple source files.So we don't need static API */
#undef EV_API_STATIC

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


/* 
 * TODO: consider refactoring of embed.h and types.h.
 * We can't declare this above #include "libev/ev.h", since we'll get
 * `field 'periodic' has incomplete type' compilation error.
 *
 * php_ev_periodic is special type for periodic watcher.
 * I.e. we don't want to embed extra members into EV_COMMON
 * Extends ev_watcher
 */

typedef struct php_ev_periodic {
	struct ev_periodic     periodic;   /* Contains common watcher vars embedded         */
	zend_fcall_info       *fci;        /* fci/fcc store specific "rescheduler" callback */
	zend_fcall_info_cache *fcc;
} php_ev_periodic;

typedef struct php_ev_stat {
	struct ev_stat  stat;   /* Extending ev_stat */
	char           *path;
} php_ev_stat;

typedef struct php_ev_embed {
	struct ev_embed  embed;   /* Extending ev_embed */
	zval            *other;   /* Loop to embed      */
} php_ev_embed;

#endif /* PHP_EV_EMBED_H */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
