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

#include "php_ev.h"
#include "util.h"

/* {{{ php_ev_zval_to_fd 
 * Get numeric file descriptor from PHP stream or Socket resource */
php_socket_t php_ev_zval_to_fd(zval **ppfd TSRMLS_DC)
{
	php_socket_t  file_desc = -1;
	php_stream   *stream;
#ifdef PHP_EV_USE_SOCKETS
	php_socket   *php_sock;
#endif

	if (Z_TYPE_PP(ppfd) == IS_RESOURCE) {
		/* PHP stream or PHP socket resource  */
		if (ZEND_FETCH_RESOURCE_NO_RETURN(stream, php_stream *, ppfd, -1, NULL, php_file_le_stream())) {
			php_stream_from_zval_no_verify(stream, ppfd);

			if (stream == NULL) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed obtaining fd");
				return -1;
			}

			/* PHP stream */
			if (php_stream_can_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL) == SUCCESS) {
				if (php_stream_cast(stream, PHP_STREAM_AS_FD_FOR_SELECT | PHP_STREAM_CAST_INTERNAL,
							(void*) &file_desc, 1) != SUCCESS || file_desc < 0) {
					return -1;
				}
			} else if (php_stream_can_cast(stream, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL) == SUCCESS) {
				if (php_stream_cast(stream, PHP_STREAM_AS_FD | PHP_STREAM_CAST_INTERNAL,
							(void*) &file_desc, 1) != SUCCESS || file_desc < 0) {
					return -1;
				}
			} else if (php_stream_can_cast(stream, PHP_STREAM_AS_STDIO | PHP_STREAM_CAST_INTERNAL) == SUCCESS) {
				if (php_stream_cast(stream, PHP_STREAM_AS_STDIO | PHP_STREAM_CAST_INTERNAL,
							(void*) &file_desc, 1) != SUCCESS || file_desc < 0) {
					return -1;
				}
			} else { /* STDIN, STDOUT, STDERR etc. */
				file_desc = Z_LVAL_P(*ppfd);
			}
		} else {
			/* PHP socket resource */
#ifdef PHP_EV_USE_SOCKETS
			if (ZEND_FETCH_RESOURCE_NO_RETURN(php_sock, php_socket *,ppfd, -1, NULL, php_sockets_le_socket())) {
				if (php_sock->error) {
					if (!php_sock->blocking && php_sock->error == EINPROGRESS) {
#ifdef PHP_EV_DEBUG
						php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Operation in progress");
#endif
					} else
						return -1;
				}

				return php_sock->bsd_socket;
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"either valid PHP stream or valid PHP socket resource expected");
			}
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"valid PHP stream resource expected");
#endif
			return -1;
		}
	} else if (Z_TYPE_PP(ppfd) == IS_LONG) {
		/* Numeric fd */
		file_desc = Z_LVAL_PP(ppfd);
		if (file_desc < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid file descriptor passed");
			return -1;
		}
	} else {
		/* Invalid fd */
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid file descriptor passed");
		return -1;
	}

	return file_desc;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 sts=4 fdm=marker
 * vim<600: noet sw=4 ts=4 sts=4
 */
