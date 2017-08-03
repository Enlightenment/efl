/* Legacy API implementations based on internal EO calls */

#include "edje_private.h"

EAPI Eina_Bool
edje_object_part_geometry_get(const Edje_Object *obj, const char *part, int *x, int *y, int *w, int *h)
{
   Edje_Real_Part *rp;
   Edje *ed;

   // Similar to geometry_get(efl_part(obj, part), x, y, w, h) but the bool
   // return value matters here.

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (w) *w = 0;
        if (h) *h = 0;
        return EINA_FALSE;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (w) *w = 0;
        if (h) *h = 0;
        return EINA_FALSE;
     }
   if (x) *x = rp->x;
   if (y) *y = rp->y;
   if (w) *w = rp->w;
   if (h) *h = rp->h;
   return EINA_TRUE;
}

EAPI const char *
edje_object_part_state_get(const Edje_Object *obj, const char * part, double *val_ret)
{
   const char *str = "";
   efl_canvas_layout_internal_state_get(efl_part(obj, part), &str, val_ret);
   return str;
}

EAPI void
edje_object_message_signal_process(Edje_Object *obj)
{
   efl_canvas_layout_signal_process(obj, EINA_FALSE);
}

/* since 1.20 */
EAPI void
edje_object_message_signal_recursive_process(Edje_Object *obj)
{
   efl_canvas_layout_signal_process(obj, EINA_TRUE);
}

EAPI void
edje_object_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data)
{
   efl_canvas_layout_signal_callback_add(obj, emission, source, (Efl_Signal_Cb) func, data);
}

EAPI void *
edje_object_signal_callback_del_full(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data)
{
   Edje_Signal_Callback_Group *gp;
   Edje *ed = _edje_fetch(obj);
   Eina_Bool ok;

   if (!ed || ed->delete_me) return NULL;

   gp = (Edje_Signal_Callback_Group *) ed->callbacks;
   if (!gp) return NULL;

   emission = eina_stringshare_add(emission);
   source = eina_stringshare_add(source);

   ok = _edje_signal_callback_disable(gp, emission, source, func, data);

   // Legacy only
   if (!ok && !data)
     {
        for (unsigned i = 0; i < gp->matches->matches_count; ++i)
          {
             if (emission == gp->matches->matches[i].signal &&
                 source == gp->matches->matches[i].source &&
                 func == gp->matches->matches[i].func &&
                 !gp->flags[i].delete_me)
               {
                  gp->flags[i].delete_me = EINA_TRUE;
                  //return gp->custom_data[i];
                  break;
               }
          }
     }

   eina_stringshare_del(emission);
   eina_stringshare_del(source);

   // Note: This function seems to have returned NULL since ~2013, despite
   // what the documentation says.
   return NULL;
}

EAPI void *
edje_object_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func)
{
   // Legacy del_full seems to have been sloppy with NULL data, as that would
   // match the first callback found. Keeping this legacy behaviour unchanged.
   return edje_object_signal_callback_del_full(obj, emission, source, func, NULL);
}

EAPI void
edje_object_signal_emit(Evas_Object *obj, const char *emission, const char *source)
{
   efl_canvas_layout_signal_emit(obj, emission, source);
}

EAPI Eina_Bool
edje_object_part_external_param_set(Eo *obj, const char *part, const Edje_External_Param *param)
{
   Edje *ed = _edje_fetch(obj);
   return _edje_object_part_external_param_set(ed, part, param);
}

EAPI Eina_Bool
edje_object_part_external_param_get(const Eo *obj, const char *part, Edje_External_Param *param)
{
   Edje *ed = _edje_fetch(obj);
   return _edje_object_part_external_param_get(ed, part, param);
}

EAPI Edje_External_Param_Type
edje_object_part_external_param_type_get(const Eo *obj, const char *part, const char *param)
{
   Edje *ed = _edje_fetch(obj);
   return _edje_object_part_external_param_type_get(ed, part, param);
}

EAPI Evas_Object *
edje_object_part_external_object_get(const Edje_Object *obj, const char *part)
{
   return efl_content_get(efl_part(obj, part));
}

/* Legacy only. Shall we deprecate this API? */
EAPI Evas_Object *
edje_object_part_external_content_get(const Edje_Object *obj, const char *part, const char *content)
{
   Edje *ed = _edje_fetch(obj);
   return _edje_object_part_external_content_get(ed, part, content);
}

/* Efl.Ui.Base APIs */
EAPI void
edje_object_mirrored_set(Edje_Object *obj, Eina_Bool rtl)
{
   efl_ui_mirrored_set(obj, rtl);
}

EAPI Eina_Bool edje_object_mirrored_get(const Edje_Object *obj)
{
   return efl_ui_mirrored_get(obj);
}

EAPI void edje_object_language_set(Edje_Object *obj, const char *language)
{
   efl_ui_language_set(obj, language);
}

EAPI const char *edje_object_language_get(const Edje_Object *obj)
{
   return efl_ui_language_get(obj);
}

EAPI Eina_Bool edje_object_scale_set(Edje_Object *obj, double scale)
{
   efl_ui_scale_set(obj, scale);
   return EINA_TRUE;
}

EAPI double edje_object_scale_get(const Edje_Object *obj)
{
   return efl_ui_scale_get(obj);
}

/* Legacy part drag APIs */
EAPI Edje_Drag_Dir
edje_object_part_drag_dir_get(const Evas_Object *obj, const char *part)
{
   return efl_ui_drag_dir_get(efl_part(obj, part));
}

EAPI Eina_Bool
edje_object_part_drag_value_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   return efl_ui_drag_value_set(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_value_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   return efl_ui_drag_value_get(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_size_set(Evas_Object *obj, const char *part, double dw, double dh)
{
   return efl_ui_drag_size_set(efl_part(obj, part), dw, dh);
}

EAPI Eina_Bool
edje_object_part_drag_size_get(const Evas_Object *obj, const char *part, double *dw, double *dh)
{
   return efl_ui_drag_size_get(efl_part(obj, part), dw, dh);
}

EAPI Eina_Bool
edje_object_part_drag_step_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   return efl_ui_drag_step_set(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_step_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   return efl_ui_drag_step_get(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_page_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   return efl_ui_drag_page_set(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_page_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   return efl_ui_drag_page_get(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_step(Evas_Object *obj, const char *part, double dx, double dy)
{
   return efl_ui_drag_step_move(efl_part(obj, part), dx, dy);
}

EAPI Eina_Bool
edje_object_part_drag_page(Evas_Object *obj, const char *part, double dx, double dy)
{
   return efl_ui_drag_page_move(efl_part(obj, part), dx, dy);
}

EAPI void
edje_object_part_text_cursor_begin_set(Edje_Object *obj, const char *part, Edje_Cursor cur)
{
   efl_text_cursor_paragraph_first(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), (int) cur));
}

EAPI void
edje_object_part_text_cursor_end_set(Edje_Object *obj, const char *part, Edje_Cursor cur)
{
   efl_text_cursor_paragraph_last(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), (int) cur));
}

EAPI void
edje_object_part_text_cursor_pos_set(Edje_Object *obj, const char * part, Edje_Cursor cur, int pos)
{
   efl_text_cursor_position_set(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), (int) cur), pos);
}

EAPI int
edje_object_part_text_cursor_pos_get(const Edje_Object *obj, const char * part, Edje_Cursor cur)
{
   return efl_text_cursor_position_get(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), (int) cur));

}

EAPI Eina_Bool
edje_object_part_text_cursor_coord_set(Edje_Object *obj, const char *part, Edje_Cursor cur, int x, int y)
{
   efl_text_cursor_coord_set(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), (int) cur), x, y);
   return EINA_TRUE;
}

EAPI void
edje_object_part_text_cursor_line_begin_set(Edje_Object *obj, const char *part, Edje_Cursor cur)
{
   efl_text_cursor_line_char_first(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), (int) cur));
}

EAPI void
edje_object_part_text_cursor_line_end_set(Edje_Object *obj, const char *part, Edje_Cursor cur)
{
   efl_text_cursor_line_char_last(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), (int) cur));
}

EAPI Eina_Bool
edje_object_part_text_cursor_prev(Edje_Object *obj, const char *part, Edje_Cursor cur)
{
   efl_text_cursor_char_prev(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), (int) cur));
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_text_cursor_next(Edje_Object *obj, const char *part, Edje_Cursor cur)
{
   efl_text_cursor_char_next(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), (int) cur));
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_text_cursor_down(Edje_Object *obj, const char *part, Edje_Cursor cur)
{
   efl_text_cursor_line_jump_by(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), (int) cur), 1);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_object_part_text_cursor_up(Edje_Object *obj, const char *part, Edje_Cursor cur)
{
   efl_text_cursor_line_jump_by(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), (int) cur), -1);
   return EINA_TRUE;
}

EAPI void
edje_object_part_text_cursor_copy(Edje_Object *obj, const char *part, Edje_Cursor cur, Edje_Cursor dst)
{
   efl_text_cursor_copy(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), (int) dst),
         efl_text_cursor_get(efl_part(obj, part), (int) cur)
         );
}

EAPI char *
edje_object_part_text_cursor_content_get(const Edje_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje_Real_Part *rp;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_content_get(rp, cur);
     }

   return NULL;
}

EAPI void
edje_object_part_text_cursor_geometry_get(const Edje_Object *obj, const char * part, int *x, int *y, int *w, int *h)
{
   efl_text_cursor_geometry_get(efl_part(obj, part),
         efl_text_cursor_get(efl_part(obj, part), EFL_TEXT_CURSOR_GET_MAIN),
         EFL_TEXT_CURSOR_TYPE_BEFORE,
         x, y, w, h, NULL, NULL, NULL, NULL
         );
}

EAPI Eina_Bool
edje_object_part_text_hide_visible_password(Eo *obj, const char *part)
{
   Edje_Real_Part *rp;
   Eina_Bool int_ret = EINA_FALSE;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) return EINA_FALSE;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text))
     {
        return EINA_FALSE;
     }

   if (rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD)
     int_ret = _edje_entry_hide_visible_password(ed, rp);

   return int_ret;
}

EAPI Eina_Bool
edje_object_part_text_cursor_is_format_get(const Eo *obj, const char *part, Edje_Cursor cur)
{
   Edje_Real_Part *rp;
   Edje *ed;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_is_format_get(rp, cur);
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_object_part_text_cursor_is_visible_format_get(const Eo *obj, const char *part, Edje_Cursor cur)
{
   Edje_Real_Part *rp;
   Edje *ed;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_is_visible_format_get(rp, cur);
     }

   return EINA_FALSE;
}

EAPI const Eina_List *
edje_object_part_text_anchor_list_get(const Eo *obj, const char *part)
{
   Edje_Real_Part *rp;
   Edje *ed;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_anchors_list(rp);

   return NULL;
}

EAPI const Eina_List *
edje_object_part_text_anchor_geometry_get(const Eo *obj, const char *part, const char *anchor)
{
   Edje_Real_Part *rp;
   Edje *ed;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_anchor_geometry_get(rp, anchor);

   return NULL;
}

EAPI void
edje_object_part_text_style_user_push(Eo *obj EINA_UNUSED, const char *part, const char *style)
{
   Edje_Real_Part *rp;
   Evas_Textblock_Style *ts;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part) || (!style)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) return;

   ts = evas_textblock_style_new();
   evas_textblock_style_set(ts, style);
   evas_object_textblock_style_user_push(rp->object, ts);
   evas_textblock_style_free(ts);
   ed->recalc_hints = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
}

EAPI void
edje_object_part_text_style_user_pop(Eo *obj EINA_UNUSED, const char *part)
{
   Edje_Real_Part *rp;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) return;

   evas_object_textblock_style_user_pop(rp->object);
   ed->recalc_hints = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
}

EAPI const char *
edje_object_part_text_style_user_peek(const Eo *obj EINA_UNUSED, const char *part)
{
   Edje_Real_Part *rp;
   const Evas_Textblock_Style *ts;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return NULL;
   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) return NULL;

   ts = evas_object_textblock_style_user_peek(rp->object);
   if (ts)
     return evas_textblock_style_get(ts);

   return NULL;
}

EAPI const Eina_List *
edje_object_part_text_item_list_get(const Eo *obj EINA_UNUSED, const char *part)
{
   Edje_Real_Part *rp;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_items_list(rp);

   return NULL;
}

EAPI Eina_Bool
edje_object_part_text_item_geometry_get(const Eo *obj EINA_UNUSED, const char *part, const char *item, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Edje_Real_Part *rp;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_item_geometry_get(rp, item, cx, cy, cw, ch);
     }

   return EINA_FALSE;
}

EAPI void
edje_object_text_insert_filter_callback_add(Eo *obj EINA_UNUSED, const char *part, Edje_Text_Filter_Cb func, void *data)
{
   Edje_Text_Insert_Filter_Callback *cb;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return;
   cb = calloc(1, sizeof(Edje_Text_Insert_Filter_Callback));
   cb->part = eina_stringshare_add(part);
   cb->func = func;
   cb->data = (void *)data;
   ed->text_insert_filter_callbacks =
     eina_list_append(ed->text_insert_filter_callbacks, cb);
}

EAPI void *
edje_object_text_insert_filter_callback_del(Eo *obj EINA_UNUSED, const char *part, Edje_Text_Filter_Cb func)
{
   Edje_Text_Insert_Filter_Callback *cb;
   Eina_List *l;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return NULL;
   EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if ((!strcmp(cb->part, part)) && (cb->func == func))
          {
             void *data = cb->data;
             ed->text_insert_filter_callbacks =
               eina_list_remove_list(ed->text_insert_filter_callbacks, l);
             eina_stringshare_del(cb->part);
             free(cb);
             return data;
          }
     }

   return NULL;
}

EAPI void *
edje_object_text_insert_filter_callback_del_full(Eo *obj EINA_UNUSED, const char *part, Edje_Text_Filter_Cb func, void *data)
{
   Edje_Text_Insert_Filter_Callback *cb;
   Eina_List *l;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return NULL;
   EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if ((!strcmp(cb->part, part)) && (cb->func == func) &&
            (cb->data == data))
          {
             void *tmp = cb->data;
             ed->text_insert_filter_callbacks =
               eina_list_remove_list(ed->text_insert_filter_callbacks, l);
             eina_stringshare_del(cb->part);
             free(cb);
             return tmp;
          }
     }

   return NULL;
}

EAPI void
edje_object_text_markup_filter_callback_add(Eo *obj EINA_UNUSED, const char *part, Edje_Markup_Filter_Cb func, void *data)
{
   Edje_Markup_Filter_Callback *cb;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return;
   cb = calloc(1, sizeof(Edje_Markup_Filter_Callback));
   cb->part = eina_stringshare_add(part);
   cb->func = func;
   cb->data = (void *)data;
   ed->markup_filter_callbacks =
     eina_list_append(ed->markup_filter_callbacks, cb);
}

EAPI void *
edje_object_text_markup_filter_callback_del(Eo *obj EINA_UNUSED, const char *part, Edje_Markup_Filter_Cb func)
{
   Edje_Markup_Filter_Callback *cb;
   Eina_List *l;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return NULL;
   EINA_LIST_FOREACH(ed->markup_filter_callbacks, l, cb)
     {
        if ((!strcmp(cb->part, part)) && (cb->func == func))
          {
             void *data = cb->data;
             ed->markup_filter_callbacks =
               eina_list_remove_list(ed->markup_filter_callbacks, l);
             eina_stringshare_del(cb->part);
             free(cb);
             return data;
          }
     }

   return NULL;
}

EAPI void *
edje_object_text_markup_filter_callback_del_full(Eo *obj EINA_UNUSED, const char *part, Edje_Markup_Filter_Cb func, void *data)
{
   Edje_Markup_Filter_Callback *cb;
   Eina_List *l;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return NULL;
   EINA_LIST_FOREACH(ed->markup_filter_callbacks, l, cb)
     {
        if ((!strcmp(cb->part, part)) && (cb->func == func) &&
            (cb->data == data))
          {
             void *tmp = cb->data;
             ed->markup_filter_callbacks =
               eina_list_remove_list(ed->markup_filter_callbacks, l);
             eina_stringshare_del(cb->part);
             free(cb);
             return tmp;
          }
     }

   return NULL;
}

EAPI void
edje_object_part_text_user_insert(const Eo *obj, const char *part, const char *text)
{
   Edje_Real_Part *rp;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_user_insert(rp, text);
}

Eina_Bool
_edje_object_part_text_raw_append(Edje *ed, Evas_Object *obj, Edje_Real_Part *rp, const char *part, const char *text)
{
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_TRUE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_text_markup_append(rp, text);
   else if (text)
     {
        if (rp->typedata.text->text)
          {
             char *new = NULL;
             int len_added = strlen(text);
             int len_old = strlen(rp->typedata.text->text);
             new = malloc(len_old + len_added + 1);
             memcpy(new, rp->typedata.text->text, len_old);
             memcpy(new + len_old, text, len_added);
             new[len_old + len_added] = '\0';
             eina_stringshare_replace(&rp->typedata.text->text, new);
             free(new);
          }
        else
          {
             eina_stringshare_replace(&rp->typedata.text->text, text);
          }
     }
   ed->dirty = EINA_TRUE;
   ed->recalc_call = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
   if (ed->text_change.func)
     ed->text_change.func(ed->text_change.data, obj, part);
   return EINA_TRUE;
}

EAPI void
edje_object_part_text_append(Eo *obj, const char *part, const char *text)
{
   Edje_Real_Part *rp;

   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return;
   if ((rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return;
   _edje_object_part_text_raw_append(ed, obj, rp, part, text);
   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
   ed->recalc_hints = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
   if (ed->text_change.func)
     ed->text_change.func(ed->text_change.data, obj, part);
}

EAPI Eina_Bool
edje_object_part_text_escaped_set(Eo *obj, const char *part, const char *text)
{
   Edje_Real_Part *rp;
   Eina_Bool int_ret;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return EINA_FALSE;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK &&
       rp->part->type != EDJE_PART_TYPE_TEXT)
     return EINA_FALSE;
   if ((rp->part->type == EDJE_PART_TYPE_TEXT) && (text))
     {
        Eina_Strbuf *sbuf;
        char *esc_start = NULL, *esc_end = NULL;
        char *s, *p;

        sbuf = eina_strbuf_new();
        p = (char *)text;
        s = p;
        for (;; )
          {
             if ((*p == 0) || (esc_end) || (esc_start))
               {
                  if (esc_end)
                    {
                       const char *escape;

                       escape = evas_textblock_escape_string_range_get
                           (esc_start, esc_end + 1);
                       if (escape) eina_strbuf_append(sbuf, escape);
                       esc_start = esc_end = NULL;
                    }
                  else if (*p == 0)
                    {
                       if (!s) s = esc_start;  /* This would happen when there is & that isn't escaped */
                       eina_strbuf_append_length(sbuf, s, p - s);
                       s = NULL;
                    }
                  if (*p == 0)
                    break;
               }

             if (*p == '&')
               {
                  if (!s) s = esc_start;  /* This would happen when there is & that isn't escaped */
                  esc_start = p;
                  esc_end = NULL;
                  eina_strbuf_append_length(sbuf, s, p - s);
                  s = NULL;
               }
             else if (*p == ';')
               {
                  if (esc_start)
                    {
                       esc_end = p;
                       s = p + 1;
                    }
               }
             p++;
          }
        int_ret = _edje_object_part_text_raw_set(ed, obj, rp, part, eina_strbuf_string_get(sbuf));
        _edje_user_define_string(ed, part, rp->typedata.text->text, EDJE_TEXT_TYPE_ESCAPED);
        eina_strbuf_free(sbuf);
        return int_ret;
     }
   int_ret = _edje_object_part_text_raw_set(ed, obj, rp, part, text);
   _edje_user_define_string(ed, part, rp->typedata.text->text, EDJE_TEXT_TYPE_ESCAPED);

   return int_ret;
}

char *
_edje_text_escape(const char *text)
{
   Eina_Strbuf *txt;
   char *ret;
   const char *text_end;
   size_t text_len;

   if (!text) return NULL;

   txt = eina_strbuf_new();
   text_len = strlen(text);

   text_end = text + text_len;
   while (text < text_end)
     {
        int advance;
        const char *escaped = evas_textblock_string_escape_get(text, &advance);
        if (!escaped)
          {
             eina_strbuf_append_char(txt, text[0]);
             advance = 1;
          }
        else
          eina_strbuf_append(txt, escaped);

        text += advance;
     }

   ret = eina_strbuf_string_steal(txt);
   eina_strbuf_free(txt);
   return ret;
}

char *
_edje_text_unescape(const char *text)
{
   Eina_Strbuf *txt;
   char *ret;
   const char *text_end, *last, *escape_start;
   size_t text_len;

   if (!text) return NULL;

   txt = eina_strbuf_new();
   text_len = strlen(text);

   text_end = text + text_len;
   last = text;
   escape_start = NULL;
   for (; text < text_end; text++)
     {
        if (*text == '&')
          {
             size_t len;
             const char *str;

             if (last)
               {
                  len = text - last;
                  str = last;
               }
             else
               {
                  len = text - escape_start;
                  str = escape_start;
               }

             if (len > 0)
               eina_strbuf_append_n(txt, str, len);

             escape_start = text;
             last = NULL;
          }
        else if ((*text == ';') && (escape_start))
          {
             size_t len;
             const char *str = evas_textblock_escape_string_range_get(escape_start, text);

             if (str)
               len = strlen(str);
             else
               {
                  str = escape_start;
                  len = text + 1 - escape_start;
               }

             eina_strbuf_append_n(txt, str, len);

             escape_start = NULL;
             last = text + 1;
          }
     }

   if (!last && escape_start)
     last = escape_start;

   if (last && (text > last))
     {
        size_t len = text - last;
        eina_strbuf_append_n(txt, last, len);
     }

   ret = eina_strbuf_string_steal(txt);
   eina_strbuf_free(txt);
   return ret;
}

EAPI Eina_Bool
edje_object_part_text_unescaped_set(Eo *obj, const char *part, const char *text_to_escape)
{
   Edje_Real_Part *rp;
   Eina_Bool int_ret = EINA_FALSE;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return EINA_FALSE;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   if (rp->part->type == EDJE_PART_TYPE_TEXT)
     int_ret = _edje_object_part_text_raw_set(ed, obj, rp, part, text_to_escape);
   else if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
     {
        char *text = _edje_text_escape(text_to_escape);

        int_ret = _edje_object_part_text_raw_set(ed, obj, rp, part, text);
        free(text);
     }
   _edje_user_define_string(ed, part, rp->typedata.text->text, EDJE_TEXT_TYPE_UNESCAPED);

   return int_ret;
}

EAPI char *
edje_object_part_text_unescaped_get(const Eo *obj EINA_UNUSED, const char *part)
{
   Edje_Real_Part *rp;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return NULL;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, part);
   if (!rp) return NULL;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        const char *t = _edje_entry_text_get(rp);
        return _edje_text_unescape(t);
     }
   else
     {
        if (rp->part->type == EDJE_PART_TYPE_TEXT)
          {
             return strdup(rp->typedata.text->text);
          }
        if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
          {
             const char *t = evas_object_textblock_text_markup_get(rp->object);
             return _edje_text_unescape(t);
          }
     }

   return NULL;
}

void
_edje_object_part_text_insert(Edje *ed, Edje_Real_Part *rp, const char *text)
{
   if (!rp) return;
   if ((rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return;
   if (rp->part->entry_mode <= EDJE_ENTRY_EDIT_MODE_NONE) return;
   _edje_entry_text_markup_insert(rp, text);
   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
   ed->recalc_hints = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed);
}

EAPI void
edje_object_part_text_insert(Eo *obj, const char *part, const char *text)
{
   Edje_Real_Part *rp;
   Edje *ed;

   ed = efl_data_scope_get(obj, EDJE_OBJECT_CLASS);

   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(&ed, part);
   _edje_object_part_text_insert(ed, rp, text);
   if (ed->text_change.func)
     ed->text_change.func(ed->text_change.data, obj, part);
}

/* Calc interface APIs */

EAPI void
edje_object_update_hints_set(Edje_Object *obj, Eina_Bool update)
{
   efl_canvas_layout_calc_auto_update_hints_set(obj, update);
}

EAPI Eina_Bool
edje_object_update_hints_get(const Edje_Object *obj)
{
   return efl_canvas_layout_calc_auto_update_hints_get(obj);
}

EAPI void
edje_object_size_min_calc(Edje_Object *obj, int *minw, int *minh)
{
   efl_canvas_layout_calc_size_min(obj, minw, minh);
}

EAPI void
edje_object_size_min_restricted_calc(Edje_Object *obj, int *minw, int *minh, int restrictedw, int restrictedh)
{
   efl_canvas_layout_calc_size_min_restricted(obj, minw, minh, restrictedw, restrictedh);
}

EAPI Eina_Bool
edje_object_parts_extends_calc(Edje_Object *obj, int *x, int *y, int *w, int *h)
{
   return efl_canvas_layout_calc_parts_extends(obj, x, y, w, h);
}

EAPI int
edje_object_freeze(Edje_Object *obj)
{
   return efl_canvas_layout_calc_freeze(obj);
}

EAPI int
edje_object_thaw(Edje_Object *obj)
{
   return efl_canvas_layout_calc_thaw(obj);
}

EAPI void
edje_object_calc_force(Edje_Object *obj)
{
   efl_canvas_layout_calc_force(obj);
}
