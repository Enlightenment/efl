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
#include <config.h>
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Input.h>
#include <Ecore_Wayland.h>

#include "wayland_imcontext.h"

struct _WaylandIMContext
{
   Ecore_IMF_Context *ctx;

   struct wl_text_input_manager *text_input_manager;
   struct wl_text_input *text_input;

   Ecore_Wl_Window *window;
   Ecore_Wl_Input  *input;
   Evas            *canvas;

   char *preedit_text;
   char *preedit_commit;
   Eina_List *preedit_attrs;
   int32_t preedit_cursor;

   struct
     {
        Eina_List *attrs;
        int32_t cursor;
     } pending_preedit;

   struct
     {
        int32_t cursor;
        int32_t anchor;
        uint32_t delete_index;
        uint32_t delete_length;
     } pending_commit;

   struct
     {
        int x;
        int y;
        int width;
        int height;
     } cursor_location;

   xkb_mod_mask_t control_mask;
   xkb_mod_mask_t alt_mask;
   xkb_mod_mask_t shift_mask;

   uint32_t serial;
   uint32_t reset_serial;
};

static unsigned int
utf8_offset_to_characters(const char *str, int offset)
{
   int index = 0;
   unsigned int i = 0;

   for (; index < offset; i++)
     eina_unicode_utf8_next_get(str, &index);

   return i;
}

static void
update_state(WaylandIMContext *imcontext)
{
   char *surrounding;
   int cursor_pos;
   Ecore_Evas *ee;
   int canvas_x = 0, canvas_y = 0;

   if (!imcontext->ctx)
     return;

   /* cursor_pos is a byte index */
   if (ecore_imf_context_surrounding_get(imcontext->ctx, &surrounding, &cursor_pos))
     {
        if (imcontext->text_input)
          wl_text_input_set_surrounding_text(imcontext->text_input, surrounding, 
                                             cursor_pos, cursor_pos);
        free(surrounding);
     }

   if (imcontext->canvas)
     {
        ee = ecore_evas_ecore_evas_get(imcontext->canvas);
        if (ee)
          ecore_evas_geometry_get(ee, &canvas_x, &canvas_y, NULL, NULL);
     }

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "canvas (x: %d, y: %d)", 
                     canvas_x, canvas_y);

   if (imcontext->text_input)
     {
        wl_text_input_set_cursor_rectangle(imcontext->text_input,
                                           imcontext->cursor_location.x + canvas_x,
                                           imcontext->cursor_location.y + canvas_y,
                                           imcontext->cursor_location.width,
                                           imcontext->cursor_location.height);

        wl_text_input_commit_state(imcontext->text_input, ++imcontext->serial);
     }
}

static Eina_Bool
check_serial(WaylandIMContext *imcontext, uint32_t serial)
{
   Ecore_IMF_Preedit_Attr *attr;

   if ((imcontext->serial - serial) > 
       (imcontext->serial - imcontext->reset_serial))
     {
        EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom,
                          "outdated serial: %u, current: %u, reset: %u",
                          serial, imcontext->serial, imcontext->reset_serial);

        /* Clear pending data */
        imcontext->pending_commit.delete_index = 0;
        imcontext->pending_commit.delete_length = 0;
        imcontext->pending_commit.cursor = 0;
        imcontext->pending_commit.anchor = 0;

        imcontext->pending_preedit.cursor = 0;
        EINA_LIST_FREE(imcontext->pending_preedit.attrs, attr) free(attr);
        imcontext->pending_preedit.attrs = NULL;

        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
clear_preedit(WaylandIMContext *imcontext)
{
   Ecore_IMF_Preedit_Attr *attr;

   imcontext->preedit_cursor = 0;

   free(imcontext->preedit_text);
   imcontext->preedit_text = NULL;

   free(imcontext->preedit_commit);
   imcontext->preedit_commit = NULL;

   EINA_LIST_FREE(imcontext->preedit_attrs, attr)
     free(attr);

   imcontext->preedit_attrs = NULL;
}

static void
text_input_commit_string(void                 *data,
                         struct wl_text_input *text_input EINA_UNUSED,
                         uint32_t              serial,
                         const char           *text)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)data;
   Eina_Bool old_preedit = EINA_FALSE;
   char *surrounding;
   int cursor_pos, cursor;
   Ecore_IMF_Event_Delete_Surrounding ev;

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom,
                     "commit event (text: `%s', current pre-edit: `%s')",
                     text,
                     imcontext->preedit_text ? imcontext->preedit_text : "");

   old_preedit = 
     imcontext->preedit_text && strlen(imcontext->preedit_text) > 0;

   if (!imcontext->ctx)
     return;

   if (!check_serial(imcontext, serial))
     return;

   if (old_preedit)
     {
        ecore_imf_context_preedit_end_event_add(imcontext->ctx);
        ecore_imf_context_event_callback_call(imcontext->ctx, 
                                              ECORE_IMF_CALLBACK_PREEDIT_END,
                                              NULL);
     }

   clear_preedit(imcontext);

   if (imcontext->pending_commit.delete_length > 0)
     {
        /* cursor_pos is a byte index */
        if (ecore_imf_context_surrounding_get(imcontext->ctx, &surrounding, 
                                              &cursor_pos))
          {
             ev.ctx = imcontext->ctx;
             /* offset and n_chars are in characters */
             ev.offset = utf8_offset_to_characters(surrounding, cursor_pos + imcontext->pending_commit.delete_index);
             ev.n_chars = utf8_offset_to_characters(surrounding,
                                                    cursor_pos + imcontext->pending_commit.delete_index + imcontext->pending_commit.delete_length) - ev.offset;

             /* cursor in characters */
             cursor = utf8_offset_to_characters(surrounding, cursor_pos);

             ev.offset -= cursor;

             EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom,
                     "delete on commit (text: `%s', offset `%d', length: `%d')",
                     surrounding, ev.offset, ev.n_chars);

             free(surrounding);

             ecore_imf_context_delete_surrounding_event_add(imcontext->ctx, ev.offset, ev.n_chars);
             ecore_imf_context_event_callback_call(imcontext->ctx, ECORE_IMF_CALLBACK_DELETE_SURROUNDING, &ev);
          }
     }

   imcontext->pending_commit.delete_index = 0;
   imcontext->pending_commit.delete_length = 0;
   imcontext->pending_commit.cursor = 0;
   imcontext->pending_commit.anchor = 0;

   ecore_imf_context_commit_event_add(imcontext->ctx, text);
   ecore_imf_context_event_callback_call(imcontext->ctx, ECORE_IMF_CALLBACK_COMMIT, (void *)text);
}

static void
commit_preedit(WaylandIMContext *imcontext)
{
   if (!imcontext->preedit_commit)
     return;

   if (!imcontext->ctx)
     return;

   ecore_imf_context_commit_event_add(imcontext->ctx, 
                                      imcontext->preedit_commit);
   ecore_imf_context_event_callback_call(imcontext->ctx, 
                                         ECORE_IMF_CALLBACK_COMMIT, 
                                         (void *)imcontext->preedit_commit);
}

static void
text_input_preedit_string(void                 *data,
                          struct wl_text_input *text_input EINA_UNUSED,
                          uint32_t              serial,
                          const char           *text,
                          const char           *commit)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)data;
   Eina_Bool old_preedit = EINA_FALSE;

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom,
                     "preedit event (text: `%s', current pre-edit: `%s')",
                     text,
                     imcontext->preedit_text ? imcontext->preedit_text : "");

   if (!check_serial(imcontext, serial))
     return;

   old_preedit = 
     imcontext->preedit_text && strlen(imcontext->preedit_text) > 0;

   clear_preedit(imcontext);

   imcontext->preedit_text = strdup(text);
   imcontext->preedit_commit = strdup(commit);
   imcontext->preedit_cursor = 
     utf8_offset_to_characters(text, imcontext->pending_preedit.cursor);
   imcontext->preedit_attrs = imcontext->pending_preedit.attrs;

   imcontext->pending_preedit.attrs = NULL;

   if (!old_preedit)
     {
        ecore_imf_context_preedit_start_event_add(imcontext->ctx);
        ecore_imf_context_event_callback_call(imcontext->ctx, 
                                              ECORE_IMF_CALLBACK_PREEDIT_START, 
                                              NULL);
     }

   ecore_imf_context_preedit_changed_event_add(imcontext->ctx);
   ecore_imf_context_event_callback_call(imcontext->ctx, 
                                         ECORE_IMF_CALLBACK_PREEDIT_CHANGED, 
                                         NULL);

   if (strlen(imcontext->preedit_text) == 0)
     {
        ecore_imf_context_preedit_end_event_add(imcontext->ctx);
        ecore_imf_context_event_callback_call(imcontext->ctx, 
                                              ECORE_IMF_CALLBACK_PREEDIT_END, 
                                              NULL);
     }
}

static void
text_input_delete_surrounding_text(void                 *data,
                                   struct wl_text_input *text_input EINA_UNUSED,
                                   int32_t               index,
                                   uint32_t              length)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)data;

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom,
                     "delete surrounding text (index: %d, length: %u)",
                     index, length);

   imcontext->pending_commit.delete_index = index;
   imcontext->pending_commit.delete_length = length;
}

static void
text_input_cursor_position(void                 *data,
                           struct wl_text_input *text_input EINA_UNUSED,
                           int32_t               index,
                           int32_t               anchor)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)data;

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom,
                     "cursor_position for next commit (index: %d, anchor: %d)",
                     index, anchor);

   imcontext->pending_commit.cursor = index;
   imcontext->pending_commit.anchor = anchor;
}

static void
text_input_preedit_styling(void                 *data,
                           struct wl_text_input *text_input EINA_UNUSED,
                           uint32_t              index,
                           uint32_t              length,
                           uint32_t              style)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)data;
   Ecore_IMF_Preedit_Attr *attr = calloc(1, sizeof(*attr));

   switch (style)
     {
      case WL_TEXT_INPUT_PREEDIT_STYLE_DEFAULT:
      case WL_TEXT_INPUT_PREEDIT_STYLE_UNDERLINE:
      case WL_TEXT_INPUT_PREEDIT_STYLE_INCORRECT:
      case WL_TEXT_INPUT_PREEDIT_STYLE_HIGHLIGHT:
      case WL_TEXT_INPUT_PREEDIT_STYLE_ACTIVE:
      case WL_TEXT_INPUT_PREEDIT_STYLE_INACTIVE:
         attr->preedit_type = ECORE_IMF_PREEDIT_TYPE_SUB1;
         break;
      case WL_TEXT_INPUT_PREEDIT_STYLE_SELECTION:
         attr->preedit_type = ECORE_IMF_PREEDIT_TYPE_SUB2;
         break;
     }

   attr->start_index = index;
   attr->end_index = index + length;

   imcontext->pending_preedit.attrs = 
     eina_list_append(imcontext->pending_preedit.attrs, attr);
}

static void
text_input_preedit_cursor(void                 *data,
                          struct wl_text_input *text_input EINA_UNUSED,
                          int32_t               index)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)data;

   imcontext->pending_preedit.cursor = index;
}

static xkb_mod_index_t
modifiers_get_index(struct wl_array *modifiers_map, const char *name)
{
   xkb_mod_index_t index = 0;
   char *p = modifiers_map->data;

   while ((const char *)p < ((const char *)modifiers_map->data + modifiers_map->size))
     {
        if (strcmp(p, name) == 0)
          return index;

        index++;
        p += strlen(p) + 1;
     }

   return XKB_MOD_INVALID;
}

static xkb_mod_mask_t
modifiers_get_mask(struct wl_array *modifiers_map,
                   const char *name)
{
   xkb_mod_index_t index = modifiers_get_index(modifiers_map, name);

   if (index == XKB_MOD_INVALID)
     return XKB_MOD_INVALID;

   return 1 << index;
}
static void
text_input_modifiers_map(void                 *data,
                         struct wl_text_input *text_input EINA_UNUSED,
                         struct wl_array      *map)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)data;

   imcontext->shift_mask = modifiers_get_mask(map, "Shift");
   imcontext->control_mask = modifiers_get_mask(map, "Control");
   imcontext->alt_mask = modifiers_get_mask(map, "Mod1");
}

static void
text_input_keysym(void                 *data,
                  struct wl_text_input *text_input EINA_UNUSED,
                  uint32_t              serial EINA_UNUSED,
                  uint32_t              time,
                  uint32_t              sym,
                  uint32_t              state,
                  uint32_t              modifiers)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)data;
   char string[32], key[32], keyname[32];
   Ecore_Event_Key *e;

   memset(key, 0, sizeof(key));
   xkb_keysym_get_name(sym, key, sizeof(key));

   memset(keyname, 0, sizeof(keyname));
   xkb_keysym_get_name(sym, keyname, sizeof(keyname));
   if (keyname[0] == '\0')
     snprintf(keyname, sizeof(keyname), "Keysym-%u", sym);

   memset(string, 0, sizeof(string));
   xkb_keysym_to_utf8(sym, string, 32);

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom,
                     "key event (key: %s)",
                     keyname);

   e = malloc(sizeof(Ecore_Event_Key) + strlen(key) + strlen(keyname) + 
              strlen(string) + 3);
   if (!e) return;

   e->keyname = (char *)(e + 1);
   e->key = e->keyname + strlen(keyname) + 1;
   e->string = e->key + strlen(key) + 1;
   e->compose = e->string;

   strcpy((char *)e->keyname, keyname);
   strcpy((char *)e->key, key);
   strcpy((char *)e->string, string);

   e->window = ecore_wl_window_id_get(imcontext->window);
   e->event_window = ecore_wl_window_id_get(imcontext->window);
   e->timestamp = time;

   e->modifiers = 0;
   if (modifiers & imcontext->shift_mask)
     e->modifiers |= ECORE_EVENT_MODIFIER_SHIFT;

   if (modifiers & imcontext->control_mask)
     e->modifiers |= ECORE_EVENT_MODIFIER_CTRL;

   if (modifiers & imcontext->alt_mask)
     e->modifiers |= ECORE_EVENT_MODIFIER_ALT;

   if (state)
     ecore_event_add(ECORE_EVENT_KEY_DOWN, e, NULL, NULL);
   else
     ecore_event_add(ECORE_EVENT_KEY_UP, e, NULL, NULL);
}

static void
text_input_enter(void                 *data,
                 struct wl_text_input *text_input EINA_UNUSED,
                 struct wl_surface    *surface EINA_UNUSED)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)data;

   update_state(imcontext);

   imcontext->reset_serial = imcontext->serial;
}

static void
text_input_leave(void                 *data,
                 struct wl_text_input *text_input EINA_UNUSED)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)data;

   /* clear preedit */
   commit_preedit(imcontext);
   clear_preedit(imcontext);

   ecore_imf_context_preedit_changed_event_add(imcontext->ctx);
   ecore_imf_context_event_callback_call(imcontext->ctx, 
                                         ECORE_IMF_CALLBACK_PREEDIT_CHANGED, 
                                         NULL);

   ecore_imf_context_preedit_end_event_add(imcontext->ctx);
   ecore_imf_context_event_callback_call(imcontext->ctx, 
                                         ECORE_IMF_CALLBACK_PREEDIT_END, NULL);
}

static void
text_input_input_panel_state(void                 *data EINA_UNUSED,
                             struct wl_text_input *text_input EINA_UNUSED,
                             uint32_t              state EINA_UNUSED)
{
}

static void
text_input_language(void                 *data EINA_UNUSED,
                    struct wl_text_input *text_input EINA_UNUSED,
                    uint32_t              serial EINA_UNUSED,
                    const char           *language EINA_UNUSED)
{
}

static void
text_input_text_direction(void                 *data EINA_UNUSED,
                          struct wl_text_input *text_input EINA_UNUSED,
                          uint32_t              serial EINA_UNUSED,
                          uint32_t              direction EINA_UNUSED)
{
}

static const struct wl_text_input_listener text_input_listener =
{
   text_input_enter,
   text_input_leave,
   text_input_modifiers_map,
   text_input_input_panel_state,
   text_input_preedit_string,
   text_input_preedit_styling,
   text_input_preedit_cursor,
   text_input_commit_string,
   text_input_cursor_position,
   text_input_delete_surrounding_text,
   text_input_keysym,
   text_input_language,
   text_input_text_direction
};

EAPI void
wayland_im_context_add(Ecore_IMF_Context *ctx)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "context_add");

   imcontext->ctx = ctx;

   imcontext->text_input = 
     wl_text_input_manager_create_text_input(imcontext->text_input_manager);
   if (imcontext->text_input)
     wl_text_input_add_listener(imcontext->text_input, 
                                &text_input_listener, imcontext);
}

EAPI void
wayland_im_context_del(Ecore_IMF_Context *ctx)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "context_del");

   if (imcontext->text_input)
     wl_text_input_destroy(imcontext->text_input);

   clear_preedit(imcontext);
}

EAPI void
wayland_im_context_reset(Ecore_IMF_Context *ctx)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);

   commit_preedit(imcontext);
   clear_preedit(imcontext);

   if (imcontext->text_input)
     wl_text_input_reset(imcontext->text_input);

   update_state(imcontext);

   imcontext->reset_serial = imcontext->serial;
}

EAPI void
wayland_im_context_focus_in(Ecore_IMF_Context *ctx)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);
   Ecore_Wl_Input *input;
   struct wl_seat *seat;

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "focus-in");

   if (!imcontext->window) return;

   input = ecore_wl_window_keyboard_get(imcontext->window);
   if (!input)
     return;

   seat = ecore_wl_input_seat_get(input);
   if (!seat)
     return;

   imcontext->input = input;

   if ((imcontext->text_input) && 
       (ecore_imf_context_input_panel_enabled_get(ctx)))
     {
        wl_text_input_show_input_panel(imcontext->text_input);
        wl_text_input_activate(imcontext->text_input, seat,
                               ecore_wl_window_surface_get(imcontext->window));
     }
}

EAPI void
wayland_im_context_focus_out(Ecore_IMF_Context *ctx)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "focus-out");

   if (!imcontext->input) return;

   if (imcontext->text_input)
     wl_text_input_deactivate(imcontext->text_input,
                              ecore_wl_input_seat_get(imcontext->input));

   imcontext->input = NULL;
}

EAPI void
wayland_im_context_preedit_string_get(Ecore_IMF_Context  *ctx,
                                      char              **str,
                                      int                *cursor_pos)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom,
                     "pre-edit string requested (preedit: `%s')",
                     imcontext->preedit_text ? imcontext->preedit_text : "");

   if (str)
     *str = strdup(imcontext->preedit_text ? imcontext->preedit_text : "");

   if (cursor_pos)
     *cursor_pos = imcontext->preedit_cursor;
}

EAPI void
wayland_im_context_preedit_string_with_attributes_get(Ecore_IMF_Context  *ctx,
                                                      char              **str,
                                                      Eina_List         **attrs,
                                                      int                *cursor_pos)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom,
                     "pre-edit string with attributes requested (preedit: `%s')",
                     imcontext->preedit_text ? imcontext->preedit_text : "");

   if (str)
     *str = strdup(imcontext->preedit_text ? imcontext->preedit_text : "");

   if (attrs)
     {
        Eina_List *l;
        Ecore_IMF_Preedit_Attr *a, *attr;

        EINA_LIST_FOREACH(imcontext->preedit_attrs, l, a)
          {
             attr = malloc(sizeof(*attr));
             attr = memcpy(attr, a, sizeof(*attr));
             *attrs = eina_list_append(*attrs, attr);
          }
     }

   if (cursor_pos)
     *cursor_pos = imcontext->preedit_cursor;
}

EAPI void
wayland_im_context_cursor_position_set(Ecore_IMF_Context *ctx,
                                       int                cursor_pos)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom,
                     "set cursor position (cursor: %d)",
                     cursor_pos);

   update_state(imcontext);
}

EAPI void
wayland_im_context_use_preedit_set(Ecore_IMF_Context *ctx EINA_UNUSED,
                                   Eina_Bool          use_preedit EINA_UNUSED)
{
}

EAPI void
wayland_im_context_client_window_set(Ecore_IMF_Context *ctx,
                                     void              *window)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "client window set (window: %p)", window);

   if (window != NULL)
     imcontext->window = ecore_wl_window_find((Ecore_Window)window);
}

EAPI void
wayland_im_context_client_canvas_set(Ecore_IMF_Context *ctx,
                                     void              *canvas)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "client canvas set (canvas: %p)", canvas);

   if (canvas != NULL)
     imcontext->canvas = canvas;
}

EAPI void
wayland_im_context_show(Ecore_IMF_Context *ctx)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "context_show");

   if ((imcontext->text_input) && 
       (ecore_imf_context_input_panel_enabled_get(ctx)))
     wl_text_input_show_input_panel(imcontext->text_input);
}

EAPI void
wayland_im_context_hide(Ecore_IMF_Context *ctx)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "context_hide");

   if ((imcontext->text_input) && 
       (ecore_imf_context_input_panel_enabled_get(ctx)))
     wl_text_input_hide_input_panel(imcontext->text_input);
}

EAPI Eina_Bool
wayland_im_context_filter_event(Ecore_IMF_Context    *ctx EINA_UNUSED,
                                Ecore_IMF_Event_Type  type EINA_UNUSED,
                                Ecore_IMF_Event      *event EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI void
wayland_im_context_cursor_location_set(Ecore_IMF_Context *ctx, int x, int y, int width, int height)
{
   WaylandIMContext *imcontext = (WaylandIMContext *)ecore_imf_context_data_get(ctx);

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "cursor_location_set (x: %d, y: %d, w: %d, h: %d)", x, y, width, height);

   if ((imcontext->cursor_location.x != x) ||
       (imcontext->cursor_location.y != y) ||
       (imcontext->cursor_location.width != width) ||
       (imcontext->cursor_location.height != height))
     {
        imcontext->cursor_location.x = x;
        imcontext->cursor_location.y = y;
        imcontext->cursor_location.width = width;
        imcontext->cursor_location.height = height;

        update_state(imcontext);
     }
}


WaylandIMContext *wayland_im_context_new (struct wl_text_input_manager *text_input_manager)
{
   WaylandIMContext *context = calloc(1, sizeof(WaylandIMContext));

   EINA_LOG_DOM_INFO(_ecore_imf_wayland_log_dom, "new context created");
   context->text_input_manager = text_input_manager;

   return context;
}

/* vim:ts=8 sw=3 sts=3 expandtab cino=>5n-3f0^-2{2(0W1st0
*/
