#ifndef PHP_EV_EVWRAP_H
#define PHP_EV_EVWRAP_H

#ifdef _WIN32
# define EV_STANDALONE              /* keeps ev from requiring config.h */
# define EV_SELECT_IS_WINSOCKET 1   /* configure libev for windows select */
#endif

#include <errno.h>
#include "common.h"
#include "types.h"

/* We compile multiple source files.So we don't need static API */
#undef EV_API_STATIC

#ifdef PHP_EV_DEBUG
# define EV_VERIFY 2
#else
# define EV_VERIFY 0
#endif

#define EV_MULTIPLICITY 1
#define EV_COMPAT3      0
#define EV_MINPRI       -2
#define EV_MAXPRI       2

#define EV_PERIODIC_ENABLE 1
#define EV_IDLE_ENABLE     1
#define EV_EMBED_ENABLE    1
#define EV_STAT_ENABLE     1
#define EV_PREPARE_ENABLE  1
#define EV_CHECK_ENABLE    1
#define EV_FORK_ENABLE     1
#define EV_SIGNAL_ENABLE   1
#define EV_ASYNC_ENABLE    1
#define EV_CHILD_ENABLE    1

#ifdef _WIN32
# define EV_STANDALONE          1 /* keeps ev from requiring config.h(no autoconf) */
# define EV_USE_SELECT          1
# define EV_SELECT_IS_WINSOCKET 1 /* configure libev for windows select            */
#endif /* _WIN32 */

#define EV_H "embed.h"

#include "libev/ev.h"

#endif /* PHP_EV_EVWRAP_H */
