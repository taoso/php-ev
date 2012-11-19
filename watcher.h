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
#define php_ev_watcher_prev(w)       (((ev_watcher *) w)->e_prev)

#define php_ev_watcher_loop_ptr(w)   (php_ev_watcher_loop(w)->loop)

#define PHP_EV_WATCHER_FLAG_KEEP_ALIVE 1
#define PHP_EV_WATCHER_FLAG_UNREFED    2

#define PHP_EV_WATCHER_UNREF(w)                                                 \
    if (!(php_ev_watcher_flags(w) &                                             \
                (PHP_EV_WATCHER_FLAG_KEEP_ALIVE | PHP_EV_WATCHER_FLAG_UNREFED)) \
            && ev_is_active(w)) {                                               \
        ev_unref(php_ev_watcher_loop(w)->loop);                                 \
        php_ev_watcher_flags(w) |= PHP_EV_WATCHER_FLAG_UNREFED;                 \
    }

#define PHP_EV_WATCHER_REF(w)                                                   \
    if (php_ev_watcher_flags(w) & PHP_EV_WATCHER_FLAG_UNREFED) {                \
        php_ev_watcher_flags(w) &= ~PHP_EV_WATCHER_FLAG_UNREFED;                \
        ev_ref(php_ev_watcher_loop(w)->loop);                                   \
    }

#define PHP_EV_WATCHER_STOP(t, w)                         \
    do {                                                  \
        PHP_EV_WATCHER_REF(w);                            \
        t ## _stop(php_ev_watcher_loop_ptr(w), (t *) w);  \
    } while(0)

#define PHP_EV_WATCHER_START(t, w)                        \
    do {                                                  \
        t ## _start(php_ev_watcher_loop_ptr(w), (t *) w); \
        PHP_EV_WATCHER_UNREF(w);                          \
    } while(0)

/* Stop, ev_*_set() and start a watcher. Call it when need
 * to modify probably active watcher.
 * args - list of args for ev_*_set() with brackets */
#define PHP_EV_WATCHER_RESET(t, w, args)                  \
    do {                                                  \
        int is_active = ev_is_active(w);                  \
                                                          \
        if (is_active) {                                  \
            PHP_EV_WATCHER_STOP(t, w);                    \
        }                                                 \
                                                          \
        t ## _set args;                                   \
                                                          \
        if (is_active) {                                  \
            PHP_EV_WATCHER_START(t, w);                   \
        }                                                 \
    } while(0)


#define PHP_EV_CHECK_SIGNAL_CAN_START(w)                                           \
    do {                                                                           \
        /* Pulled this part from EV module of Perl */                              \
        if (signals [(w)->signum - 1].loop                                         \
                && signals [(w)->signum - 1].loop != php_ev_watcher_loop_ptr(w)) { \
            php_error_docref(NULL TSRMLS_CC, E_WARNING,                            \
                    "Can't start signal watcher, signal %d already "               \
                    "registered in another loop", w->signum);                      \
        }                                                                          \
    } while(0)

#define PHP_EV_SIGNAL_START(w)              \
    do {                                    \
        PHP_EV_CHECK_SIGNAL_CAN_START(w);   \
        PHP_EV_WATCHER_START(ev_signal, w); \
    } while(0)

#define PHP_EV_SIGNAL_RESET(w, seta)             \
    do {                                         \
        int active = ev_is_active(w);            \
        if (active)                              \
            PHP_EV_WATCHER_STOP(ev_signal, w);   \
        ev_ ## signal ## _set seta;              \
        if (active)                              \
            PHP_EV_WATCHER_START(ev_signal, w);  \
    } while(0)


void php_ev_watcher_callback(EV_P_ ev_watcher *watcher, int revents);
void php_ev_set_watcher(ev_watcher *w, size_t size, zval *self, php_ev_loop *loop,
		const zend_fcall_info *pfci, const zend_fcall_info_cache *pfcc, zval *data, int priority TSRMLS_DC);
void *php_ev_new_watcher(size_t size, zval *self, php_ev_loop *loop,
		const zend_fcall_info *pfci, const zend_fcall_info_cache *pfcc, zval *data, int priority TSRMLS_DC);
void php_ev_stop_watcher(ev_watcher *watcher TSRMLS_DC);

#endif /* PHP_EV_WATCHER_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
