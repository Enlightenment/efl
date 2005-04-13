/*
 * Copyright 1998-2003, ITB CompuPhase, The Netherlands.
 * info@compuphase.com.
 */

#ifndef EMBRYO_CC_OSDEFS_H
# define EMBRYO_CC_OSDEFS_H

# ifdef HAVE_STDINT_H
#  include <stdint.h>
# endif

/* _MAX_PATH is sometimes called differently and it may be in limits.h instead
 * stdio.h.
 */
# if !defined _MAX_PATH
/* not defined, perhaps stdio.h was not included */
#  include <stdio.h>
#  if !defined _MAX_PATH
/* still undefined, try a common alternative name */
#   if defined MAX_PATH
#    define _MAX_PATH    MAX_PATH
#   else
/* no _MAX_PATH and no MAX_PATH, perhaps it is in limits.h */
#    include <limits.h>
#    if defined PATH_MAX
#     define _MAX_PATH  PATH_MAX
#    elif defined _POSIX_PATH_MAX
#     define _MAX_PATH  _POSIX_PATH_MAX
#    else
/* everything failed, actually we have a problem here... */
#     define _MAX_PATH  4096
#    endif
#   endif
#  endif
# endif

#endif
