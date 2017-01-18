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

#ifndef __WAYLAND_IM_CONTEXT_H_
#define __WAYLAND_IM_CONTEXT_H_

#include <Ecore_IMF.h>
#include <Ecore_Wl2.h>

#include "text-input-unstable-v1-client-protocol.h"

extern Ecore_Wl2_Display *ewd;

typedef struct _WaylandIMContext WaylandIMContext;

void wayland_im_context_add                (Ecore_IMF_Context    *ctx);
void wayland_im_context_del                (Ecore_IMF_Context    *ctx);
void wayland_im_context_reset              (Ecore_IMF_Context    *ctx);
void wayland_im_context_focus_in           (Ecore_IMF_Context    *ctx);
void wayland_im_context_focus_out          (Ecore_IMF_Context    *ctx);
void wayland_im_context_preedit_string_get (Ecore_IMF_Context    *ctx,
                                            char                **str,
                                            int                  *cursor_pos);
void wayland_im_context_preedit_string_with_attributes_get(Ecore_IMF_Context  *ctx,
                                                           char              **str,
                                                           Eina_List         **attr,
                                                           int                *cursor_pos);

void wayland_im_context_cursor_position_set(Ecore_IMF_Context    *ctx,
                                            int                   cursor_pos);
void wayland_im_context_use_preedit_set    (Ecore_IMF_Context    *ctx,
                                            Eina_Bool             use_preedit);
void wayland_im_context_client_window_set  (Ecore_IMF_Context    *ctx,
                                            void                 *window);
void wayland_im_context_client_canvas_set  (Ecore_IMF_Context    *ctx,
                                            void                 *canvas);
void wayland_im_context_show               (Ecore_IMF_Context    *ctx);
void wayland_im_context_hide               (Ecore_IMF_Context    *ctx);
Eina_Bool wayland_im_context_filter_event  (Ecore_IMF_Context    *ctx,
                                            Ecore_IMF_Event_Type  type,
                                            Ecore_IMF_Event      *event);
void wayland_im_context_cursor_location_set(Ecore_IMF_Context    *ctx,
                                            int                   x,
                                            int                   y,
                                            int                   width,
                                            int                   height);

void wayland_im_context_autocapital_type_set(Ecore_IMF_Context *ctx,
                                             Ecore_IMF_Autocapital_Type autocapital_type);

void wayland_im_context_input_panel_layout_set(Ecore_IMF_Context *ctx,
                                               Ecore_IMF_Input_Panel_Layout layout);

void wayland_im_context_input_mode_set(Ecore_IMF_Context *ctx,
                                       Ecore_IMF_Input_Mode input_mode);

void wayland_im_context_input_hint_set(Ecore_IMF_Context *ctx,
                                       Ecore_IMF_Input_Hints input_hints);

void wayland_im_context_input_panel_language_set(Ecore_IMF_Context *ctx,
                                                 Ecore_IMF_Input_Panel_Lang lang);

void
wayland_im_context_input_panel_language_locale_get(Ecore_IMF_Context *ctx,
                                                   char **locale);

void
wayland_im_context_prediction_allow_set(Ecore_IMF_Context *ctx,
                                        Eina_Bool prediction);

WaylandIMContext *wayland_im_context_new        (struct zwp_text_input_manager_v1 *text_input_manager);

extern int _ecore_imf_wayland_log_dom;

#endif

/* vim:ts=8 sw=3 sts=3 expandtab cino=>5n-3f0^-2{2(0W1st0
 */
