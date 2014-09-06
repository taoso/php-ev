/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2014 The PHP Group                                |
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
#ifndef PHP_EV_WATCHER_H
#define PHP_EV_WATCHER_H

#define php_ev_watcher_loop(w)       (((ev_watcher *) w)->loop)
#define php_ev_watcher_fci(w)        (((ev_watcher *) w)->fci)
#define php_ev_watcher_fcc(w)        (((ev_watcher *) w)->fcc)
#define php_ev_watcher_self(w)       (((ev_watcher *) w)->self)
#define php_ev_watcher_data(w)       (((ev_watcher *) w)->data)
#define php_ev_watcher_thread_ctx(w) (((ev_watcher *) w)->thread_ctx)
#define php_ev_watcher_type(w)       (((ev_watcher *) w)->type)
#define php_ev_watcher_flags(w)      (((ev_watcher *) w)->e_flags)
#define php_ev_watcher_next(w)       (((ev_watcher *) w)->e_next)
#define php_ev_watcher_prev(w)       (((ev_watcher *) w)->e_prev)

#define php_ev_watcher_loop_ptr(w)   (php_ev_watcher_loop(w)->loop)

#define PHP_EV_WATCHER_FLAG_KEEP_ALIVE   (1<<0)
#define PHP_EV_WATCHER_FLAG_UNREFED      (1<<1)
#define PHP_EV_WATCHER_FLAG_SELF_UNREFED (1<<2)

#if 0
#define PHP_EV_WATCHER_SELF_UNREF(w)                                             \
	do {                                                                         \
		if (EXPECTED(php_ev_watcher_self(w) != NULL)                             \
				&& !(php_ev_watcher_flags(w) & PHP_EV_WATCHER_FLAG_SELF_UNREFED) \
				&&  Z_REFCOUNT_P(php_ev_watcher_self(w)) > 2) {                  \
			Z_DELREF_P(php_ev_watcher_self(w));                                  \
        	php_ev_watcher_flags(w) |= PHP_EV_WATCHER_FLAG_SELF_UNREFED;         \
		}                                                                        \
	} while (0)

#define PHP_EV_WATCHER_SELF_REF(w)                                        \
	do {                                                                  \
		if ((php_ev_watcher_flags(w) & PHP_EV_WATCHER_FLAG_SELF_UNREFED)  \
				&& Z_REFCOUNT_P(php_ev_watcher_self(w)) < 2) {            \
    		Z_ADDREF_P(php_ev_watcher_self(w));                           \
        	php_ev_watcher_flags(w) &= ~PHP_EV_WATCHER_FLAG_SELF_UNREFED; \
    	}                                                                 \
	} while (0)
#endif
#define PHP_EV_WATCHER_SELF_UNREF(w)
#define PHP_EV_WATCHER_SELF_REF(w)


#define PHP_EV_WATCHER_UNREF(w)                                                     \
	do {                                                                            \
		if (!(php_ev_watcher_flags(w) &                                             \
					(PHP_EV_WATCHER_FLAG_KEEP_ALIVE | PHP_EV_WATCHER_FLAG_UNREFED)) \
		) {                                               \
			ev_unref(php_ev_watcher_loop(w)->loop);                                 \
			php_ev_watcher_flags(w) |= PHP_EV_WATCHER_FLAG_UNREFED;                 \
		}                                                                           \
	} while (0)

#define PHP_EV_WATCHER_REF(w)                                        \
	do {                                                             \
    	if (php_ev_watcher_flags(w) & PHP_EV_WATCHER_FLAG_UNREFED) { \
        	php_ev_watcher_flags(w) &= ~PHP_EV_WATCHER_FLAG_UNREFED; \
        	ev_ref(php_ev_watcher_loop(w)->loop);                    \
    	}                                                            \
    } while (0)


#define PHP_EV_WATCHER_STOP(t, w)                            \
    do {                                                     \
        if (EXPECTED(php_ev_watcher_loop(w) != NULL)) {      \
            PHP_EV_WATCHER_REF(w);                           \
            t ## _stop(php_ev_watcher_loop_ptr(w), (t *) w); \
        }                                                    \
		PHP_EV_WATCHER_SELF_UNREF(w);                        \
    } while (0)

#define PHP_EV_WATCHER_START(t, w)                            \
    do {                                                      \
        if (php_ev_watcher_loop(w) != NULL) {                         \
            t ## _start(php_ev_watcher_loop_ptr(w), (t *) w); \
            PHP_EV_WATCHER_UNREF(w);                          \
        }                                                     \
		PHP_EV_WATCHER_SELF_REF(w);                       \
    } while (0)

/* Stop, ev_*_set() and start a watcher. Call it when need
 * to modify probably active watcher.
 * args - list of args for ev_*_set() with brackets */
#define PHP_EV_WATCHER_RESET(t, w, args) \
    do {                                 \
        int is_active = ev_is_active(w); \
                                         \
        if (is_active) {                 \
            PHP_EV_WATCHER_STOP(t, w);   \
        }                                \
                                         \
        t ## _set args;                  \
                                         \
        if (is_active) {                 \
            PHP_EV_WATCHER_START(t, w);  \
        }                                \
    } while (0)


#define PHP_EV_CHECK_SIGNAL_CAN_START(w)                                                   \
    do {                                                                                   \
        struct ev_loop *tmp_loop = MyG(signal_loops[(w)->signum - 1]);                     \
        if (tmp_loop && tmp_loop != php_ev_watcher_loop_ptr(w)) {                          \
            php_error_docref(NULL TSRMLS_CC, E_WARNING,                                    \
                    "Can't start signal watcher, signal %d already "                       \
                    "registered for another loop", (w)->signum);                           \
            return;                                                                        \
        }                                                                                  \
    } while (0)

#define PHP_EV_SIGNAL_START(w)                                                             \
    do {                                                                                   \
        PHP_EV_CHECK_SIGNAL_CAN_START(((ev_signal *) (w)));                                \
        MyG(signal_loops[(w)->signum - 1]) = php_ev_watcher_loop_ptr((w));                 \
        PHP_EV_WATCHER_START(ev_signal, (w));                                              \
    } while (0)

#define PHP_EV_SIGNAL_STOP(w)                                                              \
    do {                                                                                   \
        MyG(signal_loops[(w)->signum - 1]) = 0;                                            \
        PHP_EV_WATCHER_STOP(ev_signal, (w));                                               \
    } while (0)

#define PHP_EV_SIGNAL_RESET(w, seta)                                                       \
    do {                                                                                   \
        int active = ev_is_active(w);                                                      \
        if (active)                                                                        \
            PHP_EV_WATCHER_STOP(ev_signal, w);                                             \
        ev_ ## signal ## _set seta;                                                        \
        if (active)                                                                        \
            PHP_EV_WATCHER_START(ev_signal, w);                                            \
    } while (0)

#define PHP_EV_WATCHER_CTOR(type, zloop)                                                   \
    php_ev_ ## type ## _object_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, zloop, TRUE, TRUE)
#define PHP_EV_WATCHER_FACTORY(type, zloop)                                                \
    php_ev_ ## type ## _object_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, zloop, FALSE, TRUE)
#define PHP_EV_WATCHER_FACTORY_NS(type, zloop)                                             \
    php_ev_ ## type ## _object_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, zloop, FALSE, FALSE)

#define php_ev_set_watcher_priority(watcher, priority)   \
    do {                                                 \
        if (ev_is_pending(watcher)) {                    \
            php_error_docref(NULL TSRMLS_CC, E_WARNING,  \
                    "Failed modifying pending watcher"); \
            break;                                       \
        }                                                \
        ev_set_priority(watcher, priority);              \
    } while (0);

void php_ev_watcher_callback(EV_P_ ev_watcher *watcher, int revents);
void php_ev_set_watcher(ev_watcher *w, size_t size, zval *self, php_ev_loop *loop,
		const zend_fcall_info *pfci, const zend_fcall_info_cache *pfcc, zval *data, int priority TSRMLS_DC);
void *php_ev_new_watcher(size_t size, zval *self, php_ev_loop *loop,
		const zend_fcall_info *pfci, const zend_fcall_info_cache *pfcc, zval *data, int priority TSRMLS_DC);
void php_ev_stop_watcher(ev_watcher *watcher TSRMLS_DC);
void php_ev_start_watcher(ev_watcher *watcher TSRMLS_DC);

#endif /* PHP_EV_WATCHER_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
