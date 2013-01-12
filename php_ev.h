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

#ifndef PHP_EV_H
#define PHP_EV_H

#include "embed.h"
#include "priv.h"
#include "watcher.h"

PHP_MINIT_FUNCTION(ev);
PHP_MSHUTDOWN_FUNCTION(ev);
PHP_RINIT_FUNCTION(ev);
PHP_RSHUTDOWN_FUNCTION(ev);
PHP_MINFO_FUNCTION(ev);

/* Max. signum supported */
#ifndef EV_NSIG
# define EV_NSIG 32
#endif

ZEND_BEGIN_MODULE_GLOBALS(ev)
	zval *default_loop;
	/* Helps to prevent binding of different `signum's to a loop */
	struct ev_loop *signal_loops[EV_NSIG - 1];
ZEND_END_MODULE_GLOBALS(ev)
ZEND_EXTERN_MODULE_GLOBALS(ev)

extern zend_module_entry ev_module_entry;
#define phpext_ev_ptr &ev_module_entry

#define PHP_EV_VERSION "0.2.3"

#endif /* PHP_EV_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
