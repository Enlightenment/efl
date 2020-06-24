/* Portions of this code have been derived from Weston
 *
 * Copyright © 2008-2012 Kristian Høgsberg
 * Copyright © 2010-2012 Intel Corporation
 * Copyright © 2010-2011 Benjamin Franzke
 * Copyright © 2011-2012 Collabora, Ltd.
 * Copyright © 2010 Red Hat <mjg@redhat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "ecore_drm_private.h"

/* local variables */
static int _ecore_drm_init_count = 0;

/* external variables */
int _ecore_drm_log_dom = -1;

EAPI int ECORE_DRM_EVENT_ACTIVATE = 0;

static void
_ecore_drm_event_activate_free(void *data EINA_UNUSED, void *event)
{
   free(event);
}

void
_ecore_drm_event_activate_send(Eina_Bool active)
{
   Ecore_Drm_Event_Activate *e;

   if (!(e = calloc(1, sizeof(Ecore_Drm_Event_Activate)))) return;

   e->active = active;
   ecore_event_add(ECORE_DRM_EVENT_ACTIVATE, e,
                   _ecore_drm_event_activate_free, NULL);
}

/**
 * @defgroup Ecore_Drm_Init_Group Drm Library Init and Shutdown Functions
 *
 * Functions that start and shutdown the Ecore_Drm Library.
 */

/**
 * Initialize the Ecore_Drm library
 *
 * @return  The number of times the library has been initialized without
 *          being shut down. 0 is returned if an error occurs.
 *
 * @ingroup Ecore_Drm_Init_Group
 */
EAPI int
ecore_drm_init(void)
{
   /* if we have already initialized, return the count */
   if (++_ecore_drm_init_count != 1) return _ecore_drm_init_count;

   /* try to init eina */
   if (!eina_init()) return --_ecore_drm_init_count;

   /* try to init ecore */
   if (!ecore_init())
     {
        eina_shutdown();
        return --_ecore_drm_init_count;
     }

   /* try to init ecore_event */
   if (!ecore_event_init())
     {
        ecore_shutdown();
        eina_shutdown();
        return --_ecore_drm_init_count;
     }

   /* set logging level */
   /* eina_log_level_set(EINA_LOG_LEVEL_DBG); */

   /* try to create logging domain */
   _ecore_drm_log_dom =
     eina_log_domain_register("ecore_drm", ECORE_DRM_DEFAULT_LOG_COLOR);
   if (!_ecore_drm_log_dom)
     {
        EINA_LOG_ERR("Could not create log domain for Ecore_Drm");
        goto log_err;
     }

   /* try to init eeze */
   if (!eeze_init()) goto eeze_err;

   _ecore_drm_inputs_init();

   ECORE_DRM_EVENT_ACTIVATE = ecore_event_type_new();
   ECORE_DRM_EVENT_OUTPUT = ecore_event_type_new();
   ECORE_DRM_EVENT_SEAT_ADD = ecore_event_type_new();

   /* return init count */
   return _ecore_drm_init_count;

eeze_err:
   eina_log_domain_unregister(_ecore_drm_log_dom);
   _ecore_drm_log_dom = -1;
log_err:
   ecore_event_shutdown();
   ecore_shutdown();
   eina_shutdown();
   return --_ecore_drm_init_count;
}

/**
 * Shutdown the Ecore_Drm library.
 *
 * @return  The number of times the library has been initialized without
 *          being shutdown. 0 is returned if an error occurs.
 *
 * @ingroup Ecore_Drm_Init_Group
 */
EAPI int
ecore_drm_shutdown(void)
{
   Eina_List *lists;
   Ecore_Drm_Device *dev;

   /* _ecore_drm_init_count should not go below zero. */
   if (_ecore_drm_init_count < 1)
     {
        ERR("Ecore_Drm Shutdown called without Ecore_Drm Init");
        return 0;
     }

   /* if we are still in use, decrement init count and get out */
   if (--_ecore_drm_init_count != 0) return _ecore_drm_init_count;

   ecore_event_type_flush(ECORE_DRM_EVENT_ACTIVATE,
                          ECORE_DRM_EVENT_OUTPUT,
                          ECORE_DRM_EVENT_SEAT_ADD);

   /* free the list of devices */
   lists = eina_list_clone(ecore_drm_devices_get());
   EINA_LIST_FREE(lists, dev)
     {
        ecore_drm_device_free(dev);
     }

   _ecore_drm_inputs_shutdown();

   /* close eeze */
   eeze_shutdown();

   /* shutdown ecore_event */
   ecore_event_shutdown();

   /* shutdown ecore */
   ecore_shutdown();

   /* unregsiter log domain */
   eina_log_domain_unregister(_ecore_drm_log_dom);
   _ecore_drm_log_dom = -1;

   /* shutdown eina */
   eina_shutdown();

   /* return init count */
   return _ecore_drm_init_count;
}
