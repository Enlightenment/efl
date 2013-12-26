/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler,
 *                         Jorge Luis Zapata Muga,
 *                         Cedric Bail,
 *                         Gustavo Sverzut Barbieri
 *                         Tom Hacohen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.

 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_unicode.h"
#include "eina_log.h"
#include "eina_share_common.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_binshare.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_eina_share_binshare_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_share_binshare_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_share_binshare_log_dom, __VA_ARGS__)

static int _eina_share_binshare_log_dom = -1;

/* The actual share */
static Eina_Share *binshare_share;
static const char EINA_MAGIC_BINSHARE_NODE_STR[] = "Eina Binshare Node";

/**
 * @endcond
 */

/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/**
 * @internal
 * @brief Initialize the share_common module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the share_common module of Eina. It is called by
 * eina_init().
 *
 * @see eina_init()
 */
EAPI Eina_Bool
eina_binshare_init(void)
{
   Eina_Bool ret;

   if (_eina_share_binshare_log_dom < 0)
     {
        _eina_share_binshare_log_dom = eina_log_domain_register
          ("eina_binshare", EINA_LOG_COLOR_DEFAULT);

        if (_eina_share_binshare_log_dom < 0)
          {
             EINA_LOG_ERR("Could not register log domain: eina_binshare");
             return EINA_FALSE;
          }
     }

   ret = eina_share_common_init(&binshare_share,
                                EINA_MAGIC_BINSHARE_NODE,
                                EINA_MAGIC_BINSHARE_NODE_STR);

   if (!ret)
     {
        eina_log_domain_unregister(_eina_share_binshare_log_dom);
        _eina_share_binshare_log_dom = -1;
     }

   return ret;
}

/**
 * @internal
 * @brief Shut down the share_common module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the share_common module set up by
 * eina_share_common_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
EAPI Eina_Bool
eina_binshare_shutdown(void)
{
   Eina_Bool ret;
   ret = eina_share_common_shutdown(&binshare_share);

   if (_eina_share_binshare_log_dom > 0)
     {
        eina_log_domain_unregister(_eina_share_binshare_log_dom);
        _eina_share_binshare_log_dom = -1;
     }

   return ret;
}


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI void
eina_binshare_del(const void *obj)
{
   if (!obj)
      return;

   if (!eina_share_common_del(binshare_share, obj))
     CRI("EEEK trying to del non-shared binshare %p", obj);
}

EAPI const void *
eina_binshare_add_length(const void *obj, unsigned int olen)
{
   return eina_share_common_add_length(binshare_share,
                                                     obj,
                                                     (olen) * sizeof(char),
                                                     0);
}

EAPI const void *
eina_binshare_ref(const void *obj)
{
   return eina_share_common_ref(binshare_share, obj);
}

EAPI int
eina_binshare_length(const void *obj)
{
   return eina_share_common_length(binshare_share, obj);
}

EAPI void
eina_binshare_dump(void)
{
   eina_share_common_dump(binshare_share, NULL, 0);
}
