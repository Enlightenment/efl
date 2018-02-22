#ifndef EINA_VPATH_H
#define EINA_VPATH_H

#include "eina_prefix.h"

/*
 * Eina_vpath
 * eina vpath is a path that can be prefixed with a virtual path.
 *
 * A virutla path can either start with (:XXXXXXXX:) that indicates a virtual path, OR normal with / or ./ or ../ or ~
 * The char sequence in between (: and :) are used as key to lookup the real value.
 * The key has to be set by a interface before, otherwise you will get a error.
 *
 * The symbol ~ is interpretated as the home directory of the running user, and will be replaced.
 * Additional infos: https://phab.enlightenment.org/w/eina_vpath/
 */

/**
 * This datatype is a vpath, this means you can use the syntax described above.
 */
typedef const char* Eina_Vpath;

/*
 * Translate a virtual path into a normal path.
 *
 * @return a string that is not virtual anymore
 *
 */
EAPI char* eina_vpath_resolve(Eina_Vpath path);

#endif
