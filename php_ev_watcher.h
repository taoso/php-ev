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

#define php_ev_watcher_fci(w)        (((ev_watcher *)w)->fci)
#define php_ev_watcher_fcc(w)        (((ev_watcher *)w)->fcc)
#define php_ev_watcher_self(w)       (((ev_watcher *)w)->self)
#define php_ev_watcher_data(w)       (((ev_watcher *)w)->data)
#define php_ev_watcher_loop(w)       (((ev_watcher *)w)->loop)
#define php_ev_watcher_thread_ctx(w) (((ev_watcher *)w)->thread_ctx)
#define php_ev_watcher_type(w)       (((ev_watcher *)w)->type)


#define PHP_EV_WATCHER_STOP(t, w)                       \
    do {                                                \
        t ## _stop(php_ev_watcher_loop_ptr(w TSRMLS_CC), (t *)w); \
    } while(0)

#define PHP_EV_WATCHER_START(t, w)                       \
    do {                                                 \
        t ## _start(php_ev_watcher_loop_ptr(w TSRMLS_CC), (t *)w); \
    } while(0)


inline php_ev_object *php_ev_watcher_loop_obj(const ev_watcher *w TSRMLS_DC);
inline struct ev_loop *php_ev_watcher_loop_ptr(const ev_watcher *w TSRMLS_DC);
inline void php_ev_set_watcher_priority(ev_watcher *watcher, long priority TSRMLS_DC);
void php_ev_watcher_callback(EV_P_ ev_watcher *watcher, int revents);
void *php_ev_new_watcher(size_t size, zval *self, zval *loop,
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
