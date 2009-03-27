/* vim: set sw=4 ts=4 sts=4 et: */
#ifndef EFREET_BASE_H
#define EFREET_BASE_H

/**
 * @file efreet_base.h
 * @brief Contains the methods used to support the FDO base directory
 * specification.
 * @addtogroup Efreet_Base Efreet_Base: The XDG Base Directory Specification
 * functions
 *
 * @{
 */

#include <Eina.h>

EAPI const char *efreet_data_home_get(void);
EAPI Eina_List *efreet_data_dirs_get(void);

EAPI const char *efreet_config_home_get(void);
EAPI Eina_List *efreet_config_dirs_get(void);

EAPI const char *efreet_cache_home_get(void);

/**
 * @}
 */

#endif
