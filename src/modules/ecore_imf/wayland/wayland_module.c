/*
 * Copyright © 2012, 2013 Intel Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>
#include <Ecore_IMF.h>
#include <Ecore_Wl2.h>
#include <stdio.h>

#include "wayland_imcontext.h"
#include "text-input-unstable-v1-client-protocol.h"

int _ecore_imf_wayland_log_dom = -1;
Ecore_Wl2_Display *ewd;

static const Ecore_IMF_Context_Info wayland_im_info =
{
   "wayland",
   "Wayland",
   "*",
   NULL,
   0
};

static Ecore_IMF_Context_Class wayland_imf_class =
{
   wayland_im_context_add,                    /* add */
   wayland_im_context_del,                    /* del */
   wayland_im_context_client_window_set,      /* client_window_set */
   wayland_im_context_client_canvas_set,      /* client_canvas_set */
   wayland_im_context_show,                   /* show */
   wayland_im_context_hide,                   /* hide */
   wayland_im_context_preedit_string_get,     /* get_preedit_string */
   wayland_im_context_focus_in,               /* focus_in */
   wayland_im_context_focus_out,              /* focus_out */
   wayland_im_context_reset,                  /* reset */
   wayland_im_context_cursor_position_set,    /* cursor_position_set */
   wayland_im_context_use_preedit_set,        /* use_preedit_set */
   wayland_im_context_input_mode_set,         /* input_mode_set */
   wayland_im_context_filter_event,           /* filter_event */
   wayland_im_context_preedit_string_with_attributes_get, /* preedit_string_with_attribute_get */
   wayland_im_context_prediction_allow_set,   /* prediction_allow_set */
   wayland_im_context_autocapital_type_set,   /* autocapital_type_set */
   NULL,                                      /* control panel show */
   NULL,                                      /* control panel hide */
   wayland_im_context_input_panel_layout_set, /* input_panel_layout_set */
   NULL,                                      /* input_panel_layout_get, */
   wayland_im_context_input_panel_language_set,/* input_panel_language_set, */
   NULL,                                      /* input_panel_language_get, */
   wayland_im_context_cursor_location_set,    /* cursor_location_set */
   NULL,                                      /* input_panel_imdata_set */
   NULL,                                      /* input_panel_imdata_get */
   NULL,                                      /* input_panel_return_key_type_set */
   NULL,                                      /* input_panel_return_key_disabled_set */
   NULL,                                      /* input_panel_caps_lock_mode_set */
   NULL,                                      /* input_panel_geometry_get */
   NULL,                                      /* input_panel_state_get */
   NULL,                                      /* input_panel_event_callback_add */
   NULL,                                      /* input_panel_event_callback_del */
   wayland_im_context_input_panel_language_locale_get, /* input_panel_language_locale_get */
   NULL,                                      /* candidate_window_geometry_get */
   wayland_im_context_input_hint_set,         /* input_hint_set */
   NULL,                                      /* bidi_direction_set */
   NULL,                                      /* keyboard_mode_get */
   NULL,                                      /* prediction_hint_set */
   NULL,                                      /* mime_type_accept_set */
   NULL                                       /* input_panel_position_set */
};

static struct zwp_text_input_manager_v1 *text_input_manager = NULL;

static Ecore_IMF_Context *
im_module_exit(void)
{
   return NULL;
}

static Ecore_IMF_Context *
im_module_create()
{
   Ecore_IMF_Context *ctx = NULL;
   WaylandIMContext *ctxd = NULL;

   if (!text_input_manager)
     {
        Eina_Iterator *itr;

        itr = ecore_wl2_display_globals_get(ewd);
        if (itr)
          {
             Ecore_Wl2_Global *global;
             struct wl_registry *registry;

             registry = ecore_wl2_display_registry_get(ewd);
             EINA_ITERATOR_FOREACH(itr, global)
               {
                  if (!strcmp(global->interface, "zwp_text_input_manager_v1"))
                    {
                       text_input_manager =
                         wl_registry_bind(registry, global->id,
                                          &zwp_text_input_manager_v1_interface, 1);
                       EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom,
                                         "bound wl_text_input_manager interface");
                       break;
                    }
               }
             eina_iterator_free(itr);
          }

        if (!text_input_manager)
          return NULL;
     }

   ctxd = wayland_im_context_new(text_input_manager);
   if (!ctxd) return NULL;

   ctx = ecore_imf_context_new(&wayland_imf_class);
   if (!ctx)
     {
        free(ctxd);
        return NULL;
     }

   ecore_imf_context_data_set(ctx, ctxd);

   return ctx;
}

static Eina_Bool
im_module_init(void)
{
   const char *s;

   _ecore_imf_wayland_log_dom = 
     eina_log_domain_register("ecore_imf_wayland", EINA_COLOR_YELLOW);

   if (!getenv("WAYLAND_DISPLAY")) return EINA_FALSE;
   if ((s = getenv("ELM_DISPLAY")))
     {
        if (strcmp(s, "wl")) return EINA_FALSE;
     }

   if (!ecore_wl2_init())
     return EINA_FALSE;

   ewd = ecore_wl2_display_connect(NULL);
   if (!ewd) goto err;

   ecore_imf_module_register(&wayland_im_info, im_module_create, 
                             im_module_exit);
   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "im module initialized");

   return EINA_TRUE;

err:
   ecore_wl2_shutdown();
   return EINA_FALSE;
}

static void
im_module_shutdown(void)
{
   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "im module shutdown");
   ecore_wl2_display_disconnect(ewd);
   ecore_wl2_shutdown();
}

EINA_MODULE_INIT(im_module_init);
EINA_MODULE_SHUTDOWN(im_module_shutdown);

/* vim:ts=8 sw=3 sts=3 expandtab cino=>5n-3f0^-2{2(0W1st0
*/
