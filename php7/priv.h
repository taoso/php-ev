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

#ifndef PHP_EV_PRIV_H
#define PHP_EV_PRIV_H

#include "types.h"
#include "macros.h"

extern const zend_function_entry ev_class_entry_functions[];
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

zval *php_ev_default_loop();

void php_ev_io_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start);
void php_ev_timer_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start);
void php_ev_periodic_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start);
void php_ev_child_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start);
void php_ev_stat_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start);
void php_ev_idle_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start);
void php_ev_check_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start);
void php_ev_prepare_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start);
void php_ev_embed_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start);
void php_ev_fork_object_ctor(INTERNAL_FUNCTION_PARAMETERS, zval *loop, zend_bool ctor, zend_bool start);

#endif /* PHP_EV_PRIV_H */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * vim600: fdm=marker
 * vim: noet sts=4 sw=4 ts=4
 */
