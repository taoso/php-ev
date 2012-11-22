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

#ifndef PHP_EV_PRIV_H
#define PHP_EV_PRIV_H

#include "types.h"
#include "macros.h"

extern const zend_function_entry ev_functions[];
extern const zend_function_entry ev_loop_class_entry_functions[];
extern const zend_function_entry ev_watcher_class_entry_functions[];
extern const zend_function_entry ev_io_class_entry_functions[];
extern const zend_function_entry ev_timer_class_entry_functions[];
extern const zend_function_entry ev_periodic_class_entry_functions[];
extern const zend_function_entry ev_signal_class_entry_functions[];
extern const zend_function_entry ev_child_class_entry_functions[];
extern const zend_function_entry ev_stat_class_entry_functions[];
extern const zend_function_entry ev_idle_class_entry_functions[];
extern const zend_function_entry ev_check_class_entry_functions[];
extern const zend_function_entry ev_prepare_class_entry_functions[];
extern const zend_function_entry ev_embed_class_entry_functions[];
extern const zend_function_entry ev_fork_class_entry_functions[];

extern const php_ev_property_entry ev_test_property_entries[];
extern const php_ev_property_entry ev_loop_property_entries[];
extern const php_ev_property_entry ev_watcher_property_entries[];
extern const php_ev_property_entry ev_io_property_entries[];
extern const php_ev_property_entry ev_timer_property_entries[];
extern const php_ev_property_entry ev_periodic_property_entries[];
extern const php_ev_property_entry ev_signal_property_entries[];
extern const php_ev_property_entry ev_child_property_entries[];
extern const php_ev_property_entry ev_stat_property_entries[];
extern const php_ev_property_entry ev_embed_property_entries[];

extern const zend_property_info ev_test_property_entry_info[];
extern const zend_property_info ev_loop_property_entry_info[];
extern const zend_property_info ev_watcher_property_entry_info[];
extern const zend_property_info ev_io_property_entry_info[];
extern const zend_property_info ev_timer_property_entry_info[];
extern const zend_property_info ev_periodic_property_entry_info[];
extern const zend_property_info ev_signal_property_entry_info[];
extern const zend_property_info ev_child_property_entry_info[];
extern const zend_property_info ev_stat_property_entry_info[];
extern const zend_property_info ev_embed_property_entry_info[];

#endif /* PHP_EV_PRIV_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
