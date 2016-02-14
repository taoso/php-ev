#ifndef PHP_EV_EVWRAP_H
#define PHP_EV_EVWRAP_H

#ifdef _WIN32
# define EV_STANDALONE              /* keeps ev from requiring config.h */
# define EV_SELECT_IS_WINSOCKET 1   /* configure libev for windows select */
#endif

#include "libev/ev.h"

#endif /* PHP_EV_EVWRAP_H */
