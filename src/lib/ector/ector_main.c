/* ECTOR - EFL retained mode drawing library
 * Copyright (C) 2014 Cedric Bail
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

#include <Ector.h>
#include "ector_private.h"

int _ector_log_dom_global = 0;

static int _ector_main_count = 0;

EAPI int
ector_init(void)
{
   if (EINA_LIKELY(_ector_main_count > 0))
     return ++_ector_main_count;

   eina_init();
   eo_init();

   _ector_log_dom_global = eina_log_domain_register("ector", ECTOR_DEFAULT_LOG_COLOR);
   if (_ector_log_dom_global < 0)
     {
        EINA_LOG_ERR("Could not register log domain: ector");
        goto on_error;
     }

   _ector_main_count = 1;

   eina_log_timing(_ector_log_dom_global, EINA_LOG_STATE_STOP, EINA_LOG_STATE_INIT);

   return _ector_main_count;

 on_error:
   eo_shutdown();
   eina_shutdown();

   return 0;
}

EAPI int
ector_shutdown(void)
{
   if (_ector_main_count <= 0)
     {
        EINA_LOG_ERR("Init count not greater than 0 in shutdown of ector.");
        return 0;
     }

   _ector_main_count--;
   if (EINA_LIKELY(_ector_main_count > 0))
     return _ector_main_count;

   eina_log_timing(_ector_log_dom_global,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   eo_shutdown();

   eina_log_domain_unregister(_ector_log_dom_global);

   eina_shutdown();
   return _ector_main_count;
}
