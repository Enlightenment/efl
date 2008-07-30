#ifndef EINA_STRINGSHARE_H_
#define EINA_STRINGSHARE_H_

#include "eina_types.h"

/**
 * @defgroup Stringshare_Group Shared strings.
 * @{
 */

EAPI int eina_stringshare_init(void);
EAPI void eina_stringshare_shutdown(void);
EAPI const char *eina_stringshare_add(const char *str);
EAPI void eina_stringshare_del(const char *str);

/** @} */

#endif /* EINA_STRINGSHARE_H_ */
