#include <Eo.h>

#include <Efl_Ui.h>
#include <Elementary.h>

#include "efl2_text_raw_editable.eo.h"

#define MY_CLASS EFL2_TEXT_RAW_EDITABLE_CLASS

#define _PARAGRAPH_SEPARATOR_UTF8 "\xE2\x80\xA9"


typedef struct
{
   Efl2_Text_Cursor             *main_cursor;
   Efl2_Text_Cursor             *sel_start, *sel_end;
   Efl2_Text_Cursor             *preedit_start, *preedit_end;
   Efl2_Text_Cursor             *cur_tmp_begin, *cur_tmp_end;
   Ecore_Timer                  *pw_timer;
   Eina_List                    *seq;
   char                         *selection;
   Efl2_Input_Text_Panel_Lang    input_panel_lang;
   Efl2_Text_Cursor_Type         cursor_mode;
   Eina_Bool                     composing : 1;
   Eina_Bool                     selecting : 1;
   Eina_Bool                     have_selection : 1;
   Eina_Bool                     select_allow : 1;
   Eina_Bool                     editable : 1;
   Eina_Bool                     select_mod_start : 1;
   Eina_Bool                     select_mod_end : 1;
   Eina_Bool                     had_sel : 1;
   Eina_Bool                     input_panel_enable : 1;
   Eina_Bool                     prediction_allow : 1;
   Eina_Bool                     anchors_updated : 1;
   Eina_Bool                     auto_return_key : 1;

#ifdef HAVE_ECORE_IMF
   Eina_Bool                     have_preedit : 1;
   Eina_Bool                     commit_cancel : 1; // For skipping useless commit
   Ecore_IMF_Context            *imf_context;
#endif
} Efl2_Text_Raw_Editable_Data;

static void _sel_range_del_emit(Eo *obj, Efl2_Text_Raw_Editable_Data *en);
static void _sel_init(Efl2_Text_Cursor *c, Eo *o, Efl2_Text_Raw_Editable_Data *en);
static void _sel_enable(Efl2_Text_Cursor *c EINA_UNUSED, Eo *o EINA_UNUSED, Efl2_Text_Raw_Editable_Data *en);
static void _sel_extend(Efl2_Text_Cursor *c, Eo *o, Efl2_Text_Raw_Editable_Data *en);
static void _sel_clear(Eo *o EINA_UNUSED, Efl2_Text_Raw_Editable_Data *en);
static void _entry_imf_cursor_info_set(Efl2_Text_Raw_Editable_Data *en);

#warning NEW FUNCTIONS
Eina_Bool _edje_entry_hide_visible_password(Eo *obj, Efl2_Text_Raw_Editable_Data *en);
static void _entry_cursor_line_geometry_get(Efl2_Canvas_Text *obj, const Efl2_Text_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);


#warning GLOBAL VARS
double _edje_password_show_last_timeout = 0;
Eina_Bool _edje_password_show_last = EINA_FALSE;

static void
_text_filter_format_prepend(Efl2_Canvas_Text *obj, Efl2_Text_Raw_Editable_Data *en,
                            Efl2_Text_Cursor *c, const char *text);

static Efl2_Text_Change_Info *
_text_filter_text_prepend(Efl2_Canvas_Text *obj, Efl2_Text_Raw_Editable_Data *en,
                          Efl2_Text_Cursor *c,
                          const char *text,
                          const char *fmtpre, const char *fmtpost,
                          Eina_Bool clearsel, Eina_Bool changeinfo);

static Efl2_Text_Change_Info *
_text_filter_markup_prepend_internal(Efl2_Canvas_Text *obj, Efl2_Text_Raw_Editable_Data *en,
                                     Efl2_Text_Cursor *c,
                                     char *text,
                                     const char *fmtpre, const char *fmtpost,
                                     Eina_Bool clearsel, Eina_Bool changeinfo);

static Eina_Bool
_password_timer_cb(void *data)
{
   Eo *obj = (Eo *)data;
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   _edje_entry_hide_visible_password(obj, en);
   en->pw_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

#ifdef HAVE_ECORE_IMF
static void
_preedit_clear(Efl2_Text_Raw_Editable_Data *en)
{
   if (en->preedit_start)
     {
        efl_del(en->preedit_start);
        en->preedit_start = NULL;
     }

   if (en->preedit_end)
     {
        efl_del(en->preedit_end);
        en->preedit_end = NULL;
     }

   en->have_preedit = EINA_FALSE;
}

static void
_preedit_del(Efl2_Text_Raw_Editable_Data *en)
{
   if (!en || !en->have_preedit) return;
   if (!en->preedit_start || !en->preedit_end) return;
   if (efl2_text_cursor_equal(en->preedit_start, en->preedit_end)) return;

   /* delete the preedit characters */
   efl2_text_cursor_range_delete(en->preedit_start, en->preedit_end);
}

static Eina_Bool
_entry_imf_retrieve_surrounding_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, char **text, int *cursor_pos)
{
   Efl2_Canvas_Text *obj = data;
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   const char *str;
   char *plain_text;

   if (text)
     {
        str = efl_text_get(obj);;
        if (str)
          {
             plain_text = evas_textblock_text_markup_to_utf8(NULL, str);

             if (plain_text)
               {
                  if (ecore_imf_context_input_hint_get(ctx) & ECORE_IMF_INPUT_HINT_SENSITIVE_DATA)
                    {
                       char *itr = NULL;
                       for (itr = plain_text; itr && *itr; ++itr)
                         *itr = '*';
                    }

                  *text = strdup(plain_text);
                  free(plain_text);
                  plain_text = NULL;
               }
             else
               {
                  *text = strdup("");
               }
          }
        else
          {
             *text = strdup("");
          }
     }

   if (cursor_pos)
     {
        if (en->have_selection && en->sel_start)
          *cursor_pos = efl2_text_cursor_position_get(en->sel_start);
        else if (efl2_text_raw_editable_main_cursor_get(obj))
          *cursor_pos = efl2_text_cursor_position_get(efl2_text_raw_editable_main_cursor_get(obj));
        else
          *cursor_pos = 0;
     }

   return EINA_TRUE;
}


static Efl2_Text_Change_Info *
_text_filter_markup_prepend(Efl2_Canvas_Text *obj, Efl2_Text_Raw_Editable_Data *en,
                            Efl2_Text_Cursor *c,
                            const char *text,
                            const char *fmtpre, const char *fmtpost,
                            Eina_Bool clearsel, Eina_Bool changeinfo)
{
   char *text2;
   //Edje_Text_Insert_Filter_Callback *cb;
   //Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(text, NULL);

   if ((clearsel) && (en->have_selection))
     {
        _sel_range_del_emit(obj, en);
     }

   text2 = strdup(text);
   /*EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if (!strcmp(cb->part, en->rp->part->name))
          {
             cb->func(cb->data, ed->obj, cb->part, EDJE_TEXT_FILTER_MARKUP, &text2);
             if (!text2) break;
          }
     }*/
   if (text2)
     {
        Efl2_Text_Change_Info *info;

        info = _text_filter_markup_prepend_internal(obj, en, c, text2,
                                                    fmtpre, fmtpost,
                                                    clearsel, changeinfo);
        return info;
     }
   return NULL;
}

static void
_entry_imf_event_commit_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info)
{
   Efl2_Canvas_Text *obj = data;
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   char *commit_str = event_info;
   Efl2_Text_Change_Info *info = NULL;

   if (en->have_selection)
     {
        if (strcmp(commit_str, ""))
          {
             /* delete selected characters */
             _sel_range_del_emit(obj, en);
             _sel_clear(obj, en);
          }
     }

   /* delete preedit characters */
   _preedit_del(en);
   _preedit_clear(en);

   // Skipping commit process when it is useless
   if (en->commit_cancel)
     {
        en->commit_cancel = EINA_FALSE;
        return;
     }

   if ((efl2_text_raw_editable_password_mode_get(obj) == EINA_TRUE) &&
       _edje_password_show_last)
     _edje_entry_hide_visible_password(obj, en);
   if ((efl2_text_raw_editable_password_mode_get(obj) == EINA_TRUE) &&
       _edje_password_show_last && (!en->preedit_start))
     {
        info = _text_filter_text_prepend(obj, en, efl2_text_raw_editable_main_cursor_get(obj), commit_str,
                                         "+ password=off", "- password",
                                         EINA_TRUE, EINA_TRUE);
        if (info)
          {
             if (en->pw_timer)
               {
                  ecore_timer_del(en->pw_timer);
                  en->pw_timer = NULL;
               }
             if (_edje_password_show_last_timeout >= 0)
               en->pw_timer = ecore_timer_add
                   (_edje_password_show_last_timeout,
                   _password_timer_cb, obj);
          }
     }
   else
     {
        info = _text_filter_text_prepend(obj, en, efl2_text_raw_editable_main_cursor_get(obj), commit_str,
                                         NULL, NULL,
                                         EINA_TRUE, EINA_TRUE);
     }

   _entry_imf_cursor_info_set(en);
   #warning ANCHORS
   //_anchors_get(efl2_text_raw_editable_main_cursor_get(obj), obj, en);
   if (info)
     {
         efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER, info);
         #warning EFL_UI_TEXT_EVENT_CHANGED
         //efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED, NULL);
         eina_stringshare_del(info->content);
         free(info);
         info = NULL;
     }
   _entry_imf_cursor_info_set(en);
}

static void
_entry_imf_event_preedit_changed_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl2_Canvas_Text *obj = data;
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl2_Text_Change_Info *info = NULL;
   int cursor_pos;
   int preedit_start_pos, preedit_end_pos;
   char *preedit_string;
   char *markup_txt = NULL;
   char *tagname[] = {
      NULL, "preedit",
      // XXX: FIXME: EFL2 - make these 2 preedit_sel's different for efl
      // 2.0 and beyond. maybe use "preedit_sel", "preedit_hilight",
      // See https://phab.enlightenment.org/D2980 for this issue
      "preedit_sel", "preedit_sel",
      "preedit_sub1", "preedit_sub2", "preedit_sub3", "preedit_sub4"
   };
   int i;
   size_t preedit_type_size = sizeof(tagname) / sizeof(tagname[0]);
   Eina_Bool preedit_end_state = EINA_FALSE;
   Eina_List *attrs = NULL, *l = NULL;
   Ecore_IMF_Preedit_Attr *attr;
   Eina_Strbuf *buf;
   Eina_Strbuf *preedit_attr_str;

   if (!en->imf_context) return;

   ecore_imf_context_preedit_string_with_attributes_get(en->imf_context,
                                                        &preedit_string,
                                                        &attrs, &cursor_pos);
   if (!preedit_string) return;

   if (!strcmp(preedit_string, ""))
     preedit_end_state = EINA_TRUE;

   if (en->have_selection && !preedit_end_state)
     _sel_range_del_emit(obj, en);

   /* delete preedit characters */
   _preedit_del(en);

   preedit_start_pos = efl2_text_cursor_position_get(efl2_text_raw_editable_main_cursor_get(obj));

   /* insert preedit character(s) */
   if (strlen(preedit_string) > 0)
     {
        buf = eina_strbuf_new();
        if (attrs)
          {
             EINA_LIST_FOREACH(attrs, l, attr)
               {
                  if (attr->preedit_type < preedit_type_size &&
                      tagname[attr->preedit_type])
                    {
                       preedit_attr_str = eina_strbuf_new();
                       if (preedit_attr_str)
                         {
                            eina_strbuf_append_n(preedit_attr_str, preedit_string + attr->start_index, attr->end_index - attr->start_index);
                            markup_txt = evas_textblock_text_utf8_to_markup(NULL, eina_strbuf_string_get(preedit_attr_str));

                            if (markup_txt)
                              {
                                 eina_strbuf_append_printf(buf, "<%s>%s</%s>", tagname[attr->preedit_type], markup_txt, tagname[attr->preedit_type]);
                                 free(markup_txt);
                              }
                            eina_strbuf_free(preedit_attr_str);
                         }
                    }
                  else
                    eina_strbuf_append(buf, preedit_string);
               }
          }
        else
          {
             eina_strbuf_append(buf, preedit_string);
          }

        // For skipping useless commit
        if (!preedit_end_state)
          en->have_preedit = EINA_TRUE;

        if ((efl2_text_raw_editable_password_mode_get(obj) == EINA_TRUE) &&
            _edje_password_show_last)
          {
             _edje_entry_hide_visible_password(obj, en);
             info = _text_filter_markup_prepend(obj, en, efl2_text_raw_editable_main_cursor_get(obj),
                                              eina_strbuf_string_get(buf),
                                              "+ password=off",
                                              "- password",
                                              EINA_TRUE, EINA_TRUE);
             if (info)
               {
                  if (en->pw_timer)
                    {
                       ecore_timer_del(en->pw_timer);
                       en->pw_timer = NULL;
                    }
                  if (_edje_password_show_last_timeout >= 0)
                    en->pw_timer = ecore_timer_add
                        (_edje_password_show_last_timeout,
                        _password_timer_cb, obj);
               }
          }
        else
          info = _text_filter_markup_prepend(obj, en, efl2_text_raw_editable_main_cursor_get(obj),
                                      eina_strbuf_string_get(buf),
                                      NULL, NULL,
                                      EINA_TRUE, EINA_TRUE);
        eina_strbuf_free(buf);
     }

   if (!preedit_end_state)
     {
        /* set preedit start cursor */
        if (!en->preedit_start)
          en->preedit_start = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
        efl2_text_cursor_copy(efl2_text_raw_editable_main_cursor_get(obj), en->preedit_start);

        /* set preedit end cursor */
        if (!en->preedit_end)
          en->preedit_end = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
        efl2_text_cursor_copy(efl2_text_raw_editable_main_cursor_get(obj), en->preedit_end);

        preedit_end_pos = efl2_text_cursor_position_get(efl2_text_raw_editable_main_cursor_get(obj));

        for (i = 0; i < (preedit_end_pos - preedit_start_pos); i++)
          {
             efl2_text_cursor_char_prev(en->preedit_start);
          }

        en->have_preedit = EINA_TRUE;

        /* set cursor position */
        efl2_text_cursor_position_set(efl2_text_raw_editable_main_cursor_get(obj), preedit_start_pos + cursor_pos);
     }

   _entry_imf_cursor_info_set(obj, en);
   #warning ANCHORS
   //_anchors_get(efl2_text_raw_editable_main_cursor_get(obj), obj, en);
   efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_PREEDIT_CHANGED, info);
   eina_stringshare_del(info->content);
   free(info);

   /* delete attribute list */
   if (attrs)
     {
        EINA_LIST_FREE(attrs, attr)
          free(attr);
     }

   free(preedit_string);
}

static void
_entry_imf_event_delete_surrounding_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info)
{
   Efl2_Canvas_Text *obj = data;
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Ecore_IMF_Event_Delete_Surrounding *ev = event_info;
   Efl2_Text_Cursor *del_start, *del_end;
   Efl2_Text_Change_Info info = { NULL, 0, 0, 0, 0 };
   int cursor_pos;
   int start, end;
   char *tmp;

   cursor_pos = efl2_text_cursor_position_get(efl2_text_raw_editable_main_cursor_get(obj));

   del_start = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
   efl2_text_cursor_position_set(del_start, cursor_pos + ev->offset);

   del_end = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
   efl2_text_cursor_position_set(del_end, cursor_pos + ev->offset + ev->n_chars);

   start = efl2_text_cursor_position_get(del_start);
   end = efl2_text_cursor_position_get(del_end);
   if (start == end) goto end;

   info.insert = EINA_FALSE;
   info.position = start;
   info.length = end - start;

   tmp = efl2_text_cursor_range_text_get(del_start, del_end);
   info.content = eina_stringshare_add(tmp);
   if (tmp) free(tmp);

   efl2_text_cursor_range_delete(del_start, del_end);
   #warning ANCHORS
   //_anchors_get(efl2_text_raw_editable_main_cursor_get(obj), obj, en);
   //_anchors_update_check(ed, rp);

   efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER, &info);
   #warning EFL_UI_TEXT_EVENT_CHANGED
   //efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED, NULL);
   efl_event_callback_call(efl2_text_raw_editable_main_cursor_get(obj), EFL2_TEXT_CURSOR_EVENT_CHANGED_USER, NULL);

   _entry_imf_cursor_info_set(obj, en);

end:
   efl_del(del_start);
   efl_del(del_end);
}

static void
_entry_imf_event_selection_set_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info)
{
   Efl2_Canvas_Text *obj = data;
   Efl2_Text_Cursor *cur = efl2_text_raw_editable_main_cursor_get(obj);
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Ecore_IMF_Event_Selection *ev = event_info;

   if (ev->start == ev->end)
     {
        efl2_text_cursor_position_set(cur, ev->start);
     }
   else
     {
        _sel_clear(obj, en);
        efl2_text_cursor_position_set(cur, ev->start);
        _sel_enable(cur, obj, en);
        _sel_init(cur, obj, en);
        efl2_text_cursor_position_set(cur, ev->end);
        _sel_extend(cur, obj, en);
     }
}

static const char *
_entry_selection_get(Efl2_Text_Raw_Editable_Data *en)
{
   if ((!en->selection) && (en->have_selection))
     en->selection = efl2_text_cursor_range_text_get(en->sel_start, en->sel_end);
   return en->selection;
}

static Eina_Bool
_entry_imf_retrieve_selection_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, char **text)
{
   Efl2_Canvas_Text *obj = data;
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   const char *selection_text = NULL;

   if (en->have_selection)
     {
        selection_text = _entry_selection_get(obj, en);

        if (text)
          *text = selection_text ? strdup(selection_text) : NULL;

        return selection_text ? EINA_TRUE : EINA_FALSE;
     }
   else
     return EINA_FALSE;
}

#endif

#ifdef HAVE_ECORE_IMF
static void
_entry_imf_cursor_location_set(Efl2_Text_Cursor *cur, Efl2_Text_Raw_Editable_Data *en)
{
#ifdef HAVE_ECORE_IMF
   Evas_Coord cx = 0, cy = 0, cw = 0, ch = 0;
   if (!en->imf_context) return;

   efl2_text_cursor_geometry_get(cur, EFL_TEXT_CURSOR_TYPE_BEFORE, &cx, &cy, &cw, &ch, NULL, NULL, NULL, NULL);
   ecore_imf_context_cursor_location_set(en->imf_context, cx, cy, cw, ch);
   // FIXME: ecore_imf_context_bidi_direction_set(en->imf_context, (Ecore_IMF_BiDi_Direction)dir);
#else
   (void)cur;
   (void)en;
#endif
}
#endif

static void
_entry_imf_cursor_info_set(Efl2_Text_Raw_Editable_Data *en)
{
#ifdef HAVE_ECORE_IMF
   int cursor_pos;
   if (!en->imf_context) return;

   if (en->have_selection)
     {
        if (efl2_text_cursor_compare(en->sel_start, en->sel_end) < 0)
          cursor_pos = efl2_text_cursor_position_get(en->sel_start);
        else
          cursor_pos = efl2_text_cursor_position_get(en->sel_end);
     }
   else
     cursor_pos = efl2_text_cursor_position_get(en->main_cursor);

   ecore_imf_context_cursor_position_set(en->imf_context, cursor_pos);

   _entry_imf_cursor_location_set(cur, en);
#else
   (void)en;
#endif
}

static void
_focus_in_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Eo *obj, void *event_info EINA_UNUSED)
{
#ifdef HAVE_ECORE_IMF
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl2_Text_Cursor *cur;

   if (!en->imf_context) return;

   cur = efl2_text_raw_editable_main_cursor_get(obj);
   ecore_imf_context_focus_in(en->imf_context);
   _entry_imf_cursor_info_set(en);
#endif
   (void)obj;
}

static void
_entry_imf_context_reset(Efl2_Text_Raw_Editable_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_reset(en->imf_context);
   if (en->commit_cancel)
     en->commit_cancel = EINA_FALSE;
#else
   (void)en;
#endif
}

static void
_focus_out_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Eo *obj, void *event_info EINA_UNUSED)
{
#ifdef HAVE_ECORE_IMF
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   if (!en->imf_context) return;

   ecore_imf_context_reset(en->imf_context);
   ecore_imf_context_focus_out(en->imf_context);
#endif
   (void)obj;
}

static void
_sel_cursor_changed(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
//   Eo *obj = data;
}

static void
_sel_init(Efl2_Text_Cursor *c, Eo *o EINA_UNUSED, Efl2_Text_Raw_Editable_Data *en)
{
   if (en->have_selection)
      return;

   efl2_text_cursor_copy(c, en->sel_start);
   efl2_text_cursor_copy(c, en->sel_end);

   en->have_selection = EINA_FALSE;
   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }
}

static void
_sel_enable(Efl2_Text_Cursor *c EINA_UNUSED,
            Eo *o EINA_UNUSED, Efl2_Text_Raw_Editable_Data *en)
{
   if (en->have_selection) return;
   en->have_selection = EINA_TRUE;
   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }

   _entry_imf_context_reset(en);
   efl_event_callback_call(o, EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_START, NULL);
}

static void
_emit_sel_state( Eo *o, Efl2_Text_Raw_Editable_Data *en)
{
   if (!efl2_text_cursor_compare(en->sel_start, en->sel_end))
     {
        efl_event_callback_call(o, EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CLEARED, NULL);
     }
   else
     {
        efl_event_callback_call(o, EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CHANGED, NULL);
     }
}

static void
_sel_extend(Efl2_Text_Cursor *c, Eo *o, Efl2_Text_Raw_Editable_Data *en)
{
   if (!en->sel_end) return;
   _sel_enable(c, o, en);
   if (efl2_text_cursor_equal(c, en->sel_end)) return;

   efl2_text_cursor_copy(c, en->sel_end);

   _entry_imf_cursor_info_set(en);

   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }

   _emit_sel_state(o, en);
}

static void
_sel_clear(Eo *o EINA_UNUSED, Efl2_Text_Raw_Editable_Data *en)
{
   en->had_sel = EINA_FALSE;
   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }
   if (en->have_selection)
     {
        en->have_selection = EINA_FALSE;
        efl2_text_cursor_copy(en->sel_start, en->sel_end);
        efl_event_callback_call(o, EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CLEARED, NULL);
     }
}

static void
_range_del_emit(Eo *obj, Efl2_Text_Cursor *cur1, Efl2_Text_Cursor *cur2)
{
   size_t start, end;
   char *tmp;
   Efl2_Text_Change_Info info = { NULL, 0, 0, 0, 0 };

   start = efl2_text_cursor_position_get(cur1);
   end = efl2_text_cursor_position_get(cur2);
   if (start == end)
      return;

   info.insert = EINA_FALSE;
   info.position = start;
   info.length = end - start;

   tmp = efl2_text_cursor_range_text_get(cur1, cur2);
   info.content = tmp;

   efl2_text_cursor_range_delete(cur1, cur2);

   efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER, &info);
   #warning EFL_UI_TEXT_EVENT_CHANGED
   //efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED, NULL);
   if (tmp) free(tmp);
}

static void
_sel_range_del_emit(Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
   _range_del_emit(obj, en->sel_start, en->sel_end);
   _sel_clear(obj, en);
}

static void
_delete_emit(Eo *obj, Efl2_Text_Cursor *c, Efl2_Text_Raw_Editable_Data *en EINA_UNUSED, size_t pos,
             Eina_Bool backspace)
{
    if (backspace)
     {
        if (!efl2_text_cursor_char_prev(c))
          {
             return;
          }
        efl2_text_cursor_char_next(c);
     }
   else
     {
        if (!efl2_text_cursor_char_next(c))
          {
             return;
          }
        efl2_text_cursor_char_prev(c);
     }

   Efl2_Text_Change_Info info = { NULL, 0, 0, 0, 0 };
   char *tmp = NULL;
   info.insert = EINA_FALSE;

   if (backspace)
     {

        Efl2_Text_Cursor *cc = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
        efl2_text_cursor_copy(c, cc);
        /* FIXME have same behaviour as elm_entry
        Eina_Bool remove_cluster = evas_textblock_cursor_at_cluster_as_single_glyph(cc,EINA_FALSE);
        if (remove_cluster)
          {
             efl2_text_cursor_cluster_start(cc);
             efl2_text_cursor_char_prev(cc);
             efl2_text_cursor_cluster_start(cc);
          }
        else*/
          {
             efl2_text_cursor_char_prev(cc);
          }

        info.position = efl2_text_cursor_position_get(cc);
        info.length = pos -info.position;

        tmp = efl2_text_cursor_range_text_get(c, cc);
        efl2_text_cursor_range_delete(c, cc);
        efl_del(cc);
     }
   else
     {
        Efl2_Text_Cursor *cc = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
        efl2_text_cursor_copy(c, cc);

        /* FIXME have same behaviour as elm_entry
        Eina_Bool remove_cluster = evas_textblock_cursor_at_cluster_as_single_glyph(cc,EINA_TRUE);
        if (remove_cluster)*/
          {
             efl2_text_cursor_cluster_end(cc);
             efl2_text_cursor_char_next(cc);
          }
        /*else
          {
             efl2_text_cursor_char_next(cc);
          }*/

        info.position = efl2_text_cursor_position_get(cc);
        info.length = pos - info.position;

        tmp = efl2_text_cursor_range_text_get(c, cc);
        efl2_text_cursor_range_delete(c, cc);
        efl_del(cc);
     }

   info.content = eina_stringshare_add(tmp);
   if (tmp) free(tmp);
   efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER, &info);
   #warning EFL_UI_TEXT_EVENT_CHANGED
   //efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED, NULL);
   eina_stringshare_del(info.content);
}

Eina_Bool
_edje_entry_hide_visible_password(Eo *obj, Efl2_Text_Raw_Editable_Data *en EINA_UNUSED)
{
   #warning IMPLEMENTATION
   (void)obj;
   return EINA_FALSE;
   /*
   Eina_Bool int_ret = EINA_FALSE;
   const Evas_Object_Textblock_Node_Format *node;
   node = evas_textblock_node_format_first_get(obj);
   for (; node; node = evas_textblock_node_format_next_get(node))
     {
        const char *text = evas_textblock_node_format_text_get(node);
        if (text)
          {
             if (!strcmp(text, "+ password=off"))
               {
                  evas_textblock_node_format_remove_pair(obj,
                                                         (Evas_Object_Textblock_Node_Format *)node);
                  int_ret = EINA_TRUE;
               }
          }
     }
   
   return int_ret;
   */
}

static Eina_Bool
_is_modifier(const char *key)
{
   if ((!strncmp(key, "Shift", 5)) ||
       (!strncmp(key, "Control", 7)) ||
       (!strncmp(key, "Alt", 3)) ||
       (!strncmp(key, "Meta", 4)) ||
       (!strncmp(key, "Super", 5)) ||
       (!strncmp(key, "Hyper", 5)) ||
       (!strcmp(key, "Scroll_Lock")) ||
       (!strcmp(key, "Num_Lock")) ||
       (!strcmp(key, "Caps_Lock")))
     return EINA_TRUE;
   return EINA_FALSE;
}

static void
_compose_seq_reset(Efl2_Text_Raw_Editable_Data *en)
{
   char *str;

   EINA_LIST_FREE(en->seq, str)
     eina_stringshare_del(str);
   en->composing = EINA_FALSE;
}

/*
 * shift: if shift is pressed.
 * movement_forward: if the movement we are going to do is forward (towards the end of the textblock)
 */
static void
_key_down_sel_pre(Eo *obj, Efl2_Text_Cursor *cur, Efl2_Text_Raw_Editable_Data *en, Eina_Bool shift, Eina_Bool movement_forward)
{
   if (en->select_allow)
     {
        if (shift)
          {
             _sel_init(cur, obj, en);
          }
        else if (en->have_selection)
          {
             Eina_Bool sel_forward = efl2_text_cursor_compare(en->sel_start, en->sel_end);
             if ((sel_forward && movement_forward) || (!sel_forward && !movement_forward))
                efl2_text_cursor_copy(en->sel_end, cur);
             else
                efl2_text_cursor_copy(en->sel_start, cur);
             _sel_clear(obj, en);
          }
     }
}

static void
_key_down_sel_post(Eo *obj, Efl2_Text_Cursor *cur, Efl2_Text_Raw_Editable_Data *en, Eina_Bool shift)
{
   if (en->select_allow)
     {
        if (shift) _sel_extend(cur, obj, en);
        else _sel_clear(obj, en);
     }
}

static void
_key_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Eo *obj, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Efl2_Text_Cursor *cur;
   Eina_Bool control, alt, shift;
#if defined(__APPLE__) && defined(__MACH__)
   Eina_Bool super, altgr;
#endif
   Eina_Bool multiline;
   int old_cur_pos;
   char *string = (char *)ev->string;
   Eina_Bool free_string = EINA_FALSE;
   Eina_Bool changed_user = EINA_FALSE;
   Efl2_Text_Change_Info *info = NULL;

   if (!ev->key) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);

   /* FIXME: Maybe allow selctions to happen even when not editable. */
   if (!en->editable) return;

#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        Ecore_IMF_Event_Key_Down ecore_ev;
        //FIXME
        //ecore_imf_evas_event_key_down_wrap(ev, &ecore_ev);
        if (!en->composing)
          {
             if (ecore_imf_context_filter_event(en->imf_context,
                                                ECORE_IMF_EVENT_KEY_DOWN,
                                                (Ecore_IMF_Event *)&ecore_ev))
               {
                  ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                  return;
               }
          }
     }
#endif

   cur = efl2_text_raw_editable_main_cursor_get(obj);
   old_cur_pos = efl2_text_cursor_position_get(cur);
   if (old_cur_pos < 0) return;

   control = evas_key_modifier_is_set(ev->modifiers, "Control");
   alt = evas_key_modifier_is_set(ev->modifiers, "Alt");
   shift = evas_key_modifier_is_set(ev->modifiers, "Shift");
#if defined(__APPLE__) && defined(__MACH__)
   super = evas_key_modifier_is_set(ev->modifiers, "Super");
   altgr = evas_key_modifier_is_set(ev->modifiers, "AltGr");
#endif
   multiline = efl2_canvas_text_multiline_get(obj);

   /* Translate some keys to strings. */
   if (!strcmp(ev->key, "Tab"))
     {
        if (multiline)
          {
             string = "\t";
          }
     }
   else if ((!strcmp(ev->key, "Return")) || (!strcmp(ev->key, "KP_Enter")))
     {
        if (multiline)
          {
             if (shift || efl2_canvas_text_legacy_newline_get(obj))
               {
                  string = "\n";
               }
             else
               {
                  string = _PARAGRAPH_SEPARATOR_UTF8;
               }
          }
     }


   /* Key handling */
   if (!strcmp(ev->key, "Escape"))
     {
        _compose_seq_reset(en);
        // dead keys here. Escape for now (should emit these)
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Up") ||
            (!strcmp(ev->key, "KP_Up") && !ev->string))
     {
        _compose_seq_reset(en);
        if (multiline)
          {
             _key_down_sel_pre(obj, cur, en, shift, EINA_FALSE);

             efl2_text_cursor_line_jump_by(cur, -1);
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

             _key_down_sel_post(obj, cur, en, shift);
          }
        efl_event_callback_call(efl2_text_raw_editable_main_cursor_get(obj), EFL2_TEXT_CURSOR_EVENT_CHANGED_USER, NULL);
     }
   else if (!strcmp(ev->key, "Down") ||
            (!strcmp(ev->key, "KP_Down") && !ev->string))
     {
        _compose_seq_reset(en);
        if (multiline)
          {
             _key_down_sel_pre(obj, cur, en, shift, EINA_TRUE);

             efl2_text_cursor_line_jump_by(cur, 1);
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

             _key_down_sel_post(obj, cur, en, shift);
          }
        efl_event_callback_call(efl2_text_raw_editable_main_cursor_get(obj), EFL2_TEXT_CURSOR_EVENT_CHANGED_USER, NULL);
     }
   else if (!strcmp(ev->key, "Left") ||
            (!strcmp(ev->key, "KP_Left") && !ev->string))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_FALSE);

        efl2_text_cursor_cluster_start(cur);
        efl2_text_cursor_char_prev(cur);
        efl2_text_cursor_cluster_start(cur);
#if defined(__APPLE__) && defined(__MACH__)
        if (altgr) efl2_text_cursor_word_start(cur);
#else
        /* If control is pressed, go to the start of the word */
        if (control) efl2_text_cursor_word_start(cur);
#endif
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        _key_down_sel_post(obj, cur, en, shift);
        efl_event_callback_call(efl2_text_raw_editable_main_cursor_get(obj), EFL2_TEXT_CURSOR_EVENT_CHANGED_USER, NULL);
     }
   else if (!strcmp(ev->key, "Right") ||
            (!strcmp(ev->key, "KP_Right") && !ev->string))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_TRUE);

#if defined(__APPLE__) && defined(__MACH__)
        if (altgr) efl2_text_cursor_word_end(cur);
#else
        /* If control is pressed, go to the end of the word */
        if (control) efl2_text_cursor_word_end(cur);
#endif
        efl2_text_cursor_cluster_end(cur);
        efl2_text_cursor_char_next(cur);

        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        _key_down_sel_post(obj, cur, en, shift);
        efl_event_callback_call(efl2_text_raw_editable_main_cursor_get(obj), EFL2_TEXT_CURSOR_EVENT_CHANGED_USER, NULL);
     }
   else if (!strcmp(ev->key, "BackSpace"))
     {
        _compose_seq_reset(en);
        if (control && !en->have_selection)
          {
             // del to start of previous word
             Efl2_Text_Cursor *tc = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));

             efl2_text_cursor_copy(tc, cur);
             efl2_text_cursor_cluster_start(cur);
             efl2_text_cursor_char_prev(cur);
             efl2_text_cursor_cluster_start(cur);
             efl2_text_cursor_word_start(cur);

             _range_del_emit(obj, cur, tc);

             efl_del(tc);
          }
        else if ((alt) && (shift))
          {
             // undo last action
          }
        else
          {
             if (en->have_selection)
               {
                  _sel_range_del_emit(obj, en);
               }
             else
               {
                  //if (efl2_text_cursor_char_prev(cur))
                    {
                       _delete_emit(obj, cur, en, old_cur_pos, EINA_TRUE);
                    }
               }
          }
        _sel_clear(obj, en);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Delete") ||
            (!strcmp(ev->key, "KP_Delete") && !ev->string))
     {
        _compose_seq_reset(en);
        if (control)
          {
             // del to end of next word
             Efl2_Text_Cursor *tc = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));

             efl2_text_cursor_copy(tc, cur);
             efl2_text_cursor_word_end(cur);
             efl2_text_cursor_cluster_end(cur);
             efl2_text_cursor_char_next(cur);

             _range_del_emit(obj, cur, tc);

             efl_del(tc);
          }
        else if (shift)
          {
             // cut
          }
        else
          {
             if (en->have_selection)
               {
                  _sel_range_del_emit(obj, en);
               }
             else
               {
                  _delete_emit(obj, cur, en, old_cur_pos, EINA_FALSE);
               }
          }
        _sel_clear(obj, en);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((!alt) &&
            (!strcmp(ev->key, "Home") ||
             ((!strcmp(ev->key, "KP_Home")) && !ev->string)))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_FALSE);

        if ((control) && (multiline))
           efl2_text_cursor_paragraph_first(cur);
        else
           efl2_text_cursor_line_start(cur);

        _key_down_sel_post(obj, cur, en, shift);
        efl_event_callback_call(efl2_text_raw_editable_main_cursor_get(obj), EFL2_TEXT_CURSOR_EVENT_CHANGED_USER, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((!alt) &&
            (!strcmp(ev->key, "End") ||
             ((!strcmp(ev->key, "KP_End")) && !ev->string)))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_TRUE);

        if ((control) && (multiline))
           efl2_text_cursor_paragraph_last(cur);
        else
           efl2_text_cursor_line_end(cur);

        _key_down_sel_post(obj, cur, en, shift);
        efl_event_callback_call(efl2_text_raw_editable_main_cursor_get(obj), EFL2_TEXT_CURSOR_EVENT_CHANGED_USER, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
#if defined(__APPLE__) && defined(__MACH__)
   else if ((super) && (!shift) && (!strcmp(ev->keyname, "v")))
#else
   else if ((control) && (!shift) && (!strcmp(ev->keyname, "v")))
#endif
     {
        _compose_seq_reset(en);
        //FIXME
        //efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_TEXT_PASTE, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
#if defined(__APPLE__) && defined(__MACH__)
   else if ((super) && (!strcmp(ev->keyname, "a")))
#else
   else if ((control) && (!strcmp(ev->keyname, "a")))
#endif
     {
        _compose_seq_reset(en);
        if (shift)
          {
             efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CLEARED, NULL);
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
          }
        else
          {
             efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_SELECTION_CHANGED, NULL);
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
          }
     }
#if defined(__APPLE__) && defined(__MACH__)
   else if ((super) && (((!shift) && !strcmp(ev->keyname, "c")) || !strcmp(ev->key, "Insert")))
#else
   else if ((control) && (((!shift) && !strcmp(ev->keyname, "c")) || !strcmp(ev->key, "Insert")))
#endif
     {
        _compose_seq_reset(en);
        //FIXME
        //efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_TEXT_COPY, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
#if defined(__APPLE__) && defined(__MACH__)
   else if ((super) && (!shift) && ((!strcmp(ev->keyname, "x") || (!strcmp(ev->keyname, "m")))))
#else
   else if ((control) && (!shift) && ((!strcmp(ev->keyname, "x") || (!strcmp(ev->keyname, "m")))))
#endif
     {
        _compose_seq_reset(en);
        //FIXME
        //efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_TEXT_CUT, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
#if defined(__APPLE__) && defined(__MACH__)
   else if ((super) && (!strcmp(ev->keyname, "z")))
#else
   else if ((control) && (!strcmp(ev->keyname, "z")))
#endif
     {
        _compose_seq_reset(en);
        if (shift)
          {
             // redo
             efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_REDO_REQUEST, NULL);
          }
        else
          {
             // undo
             efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_UNDO_REQUEST, NULL);
          }
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
#if defined(__APPLE__) && defined(__MACH__)
   else if ((super) && (!shift) && (!strcmp(ev->keyname, "y")))
#else
   else if ((control) && (!shift) && (!strcmp(ev->keyname, "y")))
#endif
     {
        _compose_seq_reset(en);
        // redo
        efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_REDO_REQUEST, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (shift && !strcmp(ev->key, "Tab"))
     {
        _compose_seq_reset(en);
        if (multiline)
          {
             // remove a tab
          }
        else
         {
            info = calloc(1, sizeof(*info));
            if (!info)
               {
                  //FIXME
                  //ERR("Running very low on memory");
               }
            else
               {
                  info->insert = EINA_TRUE;
                  info->length = 1;

                  if (en->have_selection)
                     {
                        _sel_range_del_emit(obj, en);
                        info->merge = EINA_TRUE;
                     }
                  info->position =
                     efl2_text_cursor_position_get(efl2_text_raw_editable_main_cursor_get(obj));
                  info->content = eina_stringshare_add("\t");
                  _text_filter_format_prepend(obj, en, efl2_text_raw_editable_main_cursor_get(obj), "tab");
                  #warning ANCHORS
                  //_anchors_get(efl2_text_raw_editable_main_cursor_get(obj), obj, en);

                  efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER, info);
                  /* FIXME: this is kinda gross */
                  #warning EFL_UI_TEXT_EVENT_CHANGED
                  //efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED, NULL);
               }
         }
       ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((!strcmp(ev->key, "ISO_Left_Tab")) && (multiline))
     {
        _compose_seq_reset(en);
        // remove a tab
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Prior") ||
            (!strcmp(ev->key, "KP_Prior") && !ev->string))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_FALSE);

        efl2_text_cursor_line_jump_by(cur, -10);

        _key_down_sel_post(obj, cur, en, shift);
        efl_event_callback_call(efl2_text_raw_editable_main_cursor_get(obj), EFL2_TEXT_CURSOR_EVENT_CHANGED_USER, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Next") ||
            (!strcmp(ev->key, "KP_Next") && !ev->string))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_TRUE);

        efl2_text_cursor_line_jump_by(cur, 10);

        _key_down_sel_post(obj, cur, en, shift);
        efl_event_callback_call(efl2_text_raw_editable_main_cursor_get(obj), EFL2_TEXT_CURSOR_EVENT_CHANGED_USER, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((!strcmp(ev->key, "Return")) || (!strcmp(ev->key, "KP_Enter")))
     {
        _compose_seq_reset(en);
        if (multiline)
          {
             info = calloc(1, sizeof(*info));
             if (!info)
               {
                  //FIXME
                  //ERR("Running very low on memory");
               }
             else
               {
                  info->insert = EINA_TRUE;
                  info->length = 1;
                  if (en->have_selection)
                     {
                        _sel_range_del_emit(obj, en);
                        info->merge = EINA_TRUE;
                     }

                  info->position =
                     efl2_text_cursor_position_get(efl2_text_raw_editable_main_cursor_get(obj));
                  if (shift ||
                        efl2_canvas_text_legacy_newline_get(obj))
                     {
                        _text_filter_format_prepend(obj, en, efl2_text_raw_editable_main_cursor_get(obj), "br");
                        info->content = eina_stringshare_add("\n");
                     }
                  else
                     {
                        _text_filter_format_prepend(obj, en, efl2_text_raw_editable_main_cursor_get(obj), "ps");
                        info->content = eina_stringshare_add(EFL_TEXT_PARAGRAPH_SEPARATOR_UTF8);
                     }
                  #warning ANCHORS
                  //_anchors_get(efl2_text_raw_editable_main_cursor_get(obj), obj, en);
                  efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER, info);
                  /* FIXME: this is kinda gross */
                  #warning EFL_UI_TEXT_EVENT_CHANGED
                  //efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED, NULL);
                  //cursor_changed = EINA_TRUE;
               }
          }
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else
     {
        char *compres = NULL;
        Ecore_Compose_State state;

        if (control) goto end;
        if (!en->composing)
          {
             _compose_seq_reset(en);
             en->seq = eina_list_append(en->seq, eina_stringshare_add(ev->key));
             state = ecore_compose_get(en->seq, &compres);
             if (state == ECORE_COMPOSE_MIDDLE) en->composing = EINA_TRUE;
             else en->composing = EINA_FALSE;
             if (!en->composing)
               {
                  free(compres);
                  compres = NULL;
                  _compose_seq_reset(en);
#if defined(__APPLE__) && defined(__MACH__)
                  if (super ||
                      (string && (!string[1]) &&
                       (string[0] != 0xa) && (string[0] != 0x9) &&
                       ((string[0] < 0x20) || (string[0] == 0x7f))))
#else
                  if (string && (!string[1]) &&
                      (string[0] != 0xa) && (string[0] != 0x9) &&
                      ((string[0] < 0x20) || (string[0] == 0x7f)))
#endif
                    goto end;
               }
             else
               {
                  free(compres);
                  compres = NULL;
                  goto end;
               }
          }
        else
          {
             if (_is_modifier(ev->key)) goto end;
             en->seq = eina_list_append(en->seq, eina_stringshare_add(ev->key));
             state = ecore_compose_get(en->seq, &compres);
             if (state == ECORE_COMPOSE_NONE)
               {
                  _compose_seq_reset(en);
                  free(compres);
                  compres = NULL;
               }
             else if (state == ECORE_COMPOSE_DONE)
               {
                  _compose_seq_reset(en);
                  if (compres)
                    {
                       string = compres;
                       free_string = EINA_TRUE;
                    }
                  compres = NULL;
               }
             else
               {
                  free(compres);
                  compres = NULL;
                  goto end;
               }
          }
        if (string)
          {
             if ((efl2_text_raw_editable_password_mode_get(obj) == EINA_TRUE) &&
                 _edje_password_show_last)
               {
                  _edje_entry_hide_visible_password(obj, en);
                  info = _text_filter_text_prepend(obj, en, efl2_text_raw_editable_main_cursor_get(obj), string,
                                                   "+ password=off",
                                                   "- password",
                                                   EINA_TRUE, EINA_TRUE);
                  if (info)
                    {
                       if (en->pw_timer)
                         {
                            ecore_timer_del(en->pw_timer);
                            en->pw_timer = NULL;
                         }
                       if (_edje_password_show_last_timeout >= 0)
                         en->pw_timer = ecore_timer_add
                             (_edje_password_show_last_timeout,
                             _password_timer_cb, obj);
                    }
               }
             else
               {
                  info = _text_filter_text_prepend(obj, en, efl2_text_raw_editable_main_cursor_get(obj), string,
                                                NULL, NULL,
                                                EINA_TRUE, EINA_TRUE);
                  #warning ANCHORS
                  //_anchors_get(efl2_text_raw_editable_main_cursor_get(obj), obj, en);
                  if (info)
                    {
                       changed_user = EINA_TRUE;
                       //cursor_changed = EINA_TRUE;
                       ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                    }
                  if (free_string) free(string);
               }
          }
     }
end:
   if (changed_user)
     {
        efl_event_callback_call(obj, EFL2_TEXT_RAW_EDITABLE_EVENT_CHANGED_USER, info);
        /* FIXME: this is kinda gross */
        #warning EFL_UI_TEXT_EVENT_CHANGED
        //efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED, NULL);
     }
   if (info)
     {
        eina_stringshare_del(info->content);
        free(info);
        info = NULL;
     }
    _entry_imf_cursor_info_set(en);
}

static void
_key_up_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Eo *obj, void *event_info)
{
#ifdef HAVE_ECORE_IMF
   Evas_Event_Key_Up *ev = event_info;
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   if (en->imf_context)
     {
        Ecore_IMF_Event_Key_Up ecore_ev;
        //FIXME
        //ecore_imf_evas_event_key_up_wrap(ev, &ecore_ev);
        if (ecore_imf_context_filter_event(en->imf_context,
                                           ECORE_IMF_EVENT_KEY_UP,
                                           (Ecore_IMF_Event *)&ecore_ev))
          return;
     }
#else
   (void)obj;
   (void)event_info;
#endif
}

static void _entry_cursor_line_geometry_get(Efl2_Canvas_Text *obj, const Efl2_Text_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   if (!en->cur_tmp_begin) en->cur_tmp_begin = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
   if (!en->cur_tmp_end) en->cur_tmp_end = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
   efl2_text_cursor_copy(cur, en->cur_tmp_begin);
   efl2_text_cursor_copy(cur, en->cur_tmp_end);
   efl2_text_cursor_line_start(en->cur_tmp_begin);
   efl2_text_cursor_line_end(en->cur_tmp_end);
   Eina_Iterator *itr = efl2_text_cursor_range_geometry_get(en->cur_tmp_begin, en->cur_tmp_end);
   if (itr)
     {
        void* data = NULL;
        Eina_Bool ret = eina_iterator_next(itr, &data);
        if (ret && data)
          {
             Eina_Rect* r = (Eina_Rect*) data;
             if (cx) *cx = r->x;
             if (cy) *cy = r->y;
             if (cw) *cw = r->w;
             if (ch) *ch = r->h;
          }
        eina_iterator_free(itr);
     }
}

static Efl2_Text_Cursor *
_cursor_cluster_coord_set(Efl2_Canvas_Text *obj, Efl2_Text_Cursor *cur, Evas_Coord canvasx, Evas_Coord canvasy, Evas_Coord *cx, Evas_Coord *cy)
{
   Evas_Coord x, y, lh = 0, cly = 0;
   Efl2_Text_Cursor *line_cur;
   Efl2_Text_Cursor *tc;

   tc = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
   efl2_text_cursor_copy(cur, tc);
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   *cx = canvasx - x;
   *cy = canvasy - y;

   line_cur = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
   efl2_text_cursor_paragraph_last(line_cur);
   _entry_cursor_line_geometry_get(obj, line_cur, NULL, &cly, NULL, &lh);
   /* Consider a threshold of half the line height */
   if (*cy > (cly + lh) && *cy < (cly + lh + lh / 2))
     {
        *cy = cly + lh - 1; // Make it inside Textblock
     }
   efl2_text_cursor_paragraph_first(line_cur);
   _entry_cursor_line_geometry_get(obj, line_cur, NULL, &cly, NULL, NULL);

   if (*cy < cly && *cy > (cly - lh / 2))
     {
        *cy = cly;
     }
   efl_del(line_cur);
   /* No need to check return value if not able to set the char coord Textblock
    * will take care */
   Eina_Position2D position = {
        .x = *cx,
        .y = *cy,
   };
   efl2_text_cursor_coord_set(cur, position);

   return tc;
}

#define FLOAT_T double
#define DIV(a, b) ((a) / (b))
#define FROM_INT(a) (double)(a)
#define ZERO 0.0
#define EQ(a, b) EINA_DBL_EQ(a, b)

static void
_mouse_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Eo *obj EINA_UNUSED, void *event_info)
{
   Evas_Coord cx, cy;
   Evas_Event_Mouse_Down *ev = event_info;
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl2_Text_Cursor *cur = efl2_text_raw_editable_main_cursor_get(obj);
   Efl2_Text_Cursor *tc = NULL;
   Eina_Bool dosel = EINA_FALSE;
   Eina_Bool shift;

   if ((ev->button != 1) && (ev->button != 2)) return;

#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        Ecore_IMF_Event_Mouse_Down ecore_ev;
        ecore_imf_evas_event_mouse_down_wrap(ev, &ecore_ev);
        if (ecore_imf_context_filter_event(en->imf_context,
                                           ECORE_IMF_EVENT_MOUSE_DOWN,
                                           (Ecore_IMF_Event *)&ecore_ev))
          return;
     }
#endif

   _entry_imf_context_reset(en);

   shift = evas_key_modifier_is_set(ev->modifiers, "Shift");
   en->select_mod_start = EINA_FALSE;
   en->select_mod_end = EINA_FALSE;

   if (en->select_allow && ev->button != 2) dosel = EINA_TRUE;
   if (dosel)
     {
        if (ev->flags & EVAS_BUTTON_TRIPLE_CLICK)
          {
             if (shift)
               {
                  tc = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
                  efl2_text_cursor_copy(cur, tc);
                  if (efl2_text_cursor_compare(cur, en->sel_start) < 0)
                    efl2_text_cursor_line_start(cur);
                  else
                    efl2_text_cursor_line_end(cur);
                  _sel_extend(cur, obj, en);
               }
             else
               {
                  en->have_selection = EINA_FALSE;
                  en->selecting = EINA_FALSE;
                  _sel_clear(obj, en);
                  tc = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
                  efl2_text_cursor_copy(cur, tc);
                  efl2_text_cursor_line_start(cur);
                  _sel_init(cur, obj, en);
                  efl2_text_cursor_line_end(cur);
                  _sel_extend(cur, obj, en);
               }
             goto end;
          }
        else if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
          {
             if (shift)
               {
                  tc = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
                  efl2_text_cursor_copy(cur, tc);
                  if (efl2_text_cursor_compare(cur, en->sel_start) < 0)
                    efl2_text_cursor_word_start(cur);
                  else
                    {
                       efl2_text_cursor_word_end(cur);
                       efl2_text_cursor_cluster_end(cur);
                       efl2_text_cursor_char_next(cur);
                    }
                  _sel_extend(cur, obj, en);
               }
             else
               {
                  en->have_selection = EINA_FALSE;
                  en->selecting = EINA_FALSE;
                  _sel_clear(obj, en);
                  tc = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
                  efl2_text_cursor_copy(cur, tc);
                  efl2_text_cursor_word_start(cur);
                  _sel_init(cur, obj, en);
                  efl2_text_cursor_word_end(cur);
                  efl2_text_cursor_cluster_end(cur);
                  efl2_text_cursor_char_next(cur);
                  _sel_extend(cur, obj, en);
               }
             goto end;
          }
     }
   tc = _cursor_cluster_coord_set(obj, cur, ev->canvas.x, ev->canvas.y, &cx, &cy);

   if (dosel)
     {
        //FIXME
        /*if ((en->have_selection) &&
            (rp->part->select_mode == EDJE_ENTRY_SELECTION_MODE_EXPLICIT))
          {
             if (shift)
               _sel_extend(cur, obj, en);
             else
               {
                  Eina_List *first, *last;
                  FLOAT_T sc;

                  first = en->sel;
                  last = eina_list_last(en->sel);
                  if (first && last)
                    {
                       Evas_Textblock_Rectangle *r1, *r2;
                       Evas_Coord d, d1, d2;

                       r1 = first->data;
                       r2 = last->data;
                       d = r1->x - cx;
                       d1 = d * d;
                       d = (r1->y + (r1->h / 2)) - cy;
                       d1 += d * d;
                       d = r2->x + r2->w - 1 - cx;
                       d2 = d * d;
                       d = (r2->y + (r2->h / 2)) - cy;
                       d2 += d * d;
                       sc = DIV(en->ed->scale, en->ed->file->base_scale);
                       if (EQ(sc, ZERO)) sc = DIV(_edje_scale, en->ed->file->base_scale);
                       d = (Evas_Coord)MUL(FROM_INT(20), sc); // FIXME: maxing number!
                       d = d * d;
                       if (d1 < d2)
                         {
                            if (d1 <= d)
                              {
                                 en->select_mod_start = EINA_TRUE;
                                 en->selecting = EINA_TRUE;
                              }
                         }
                       else
                         {
                            if (d2 <= d)
                              {
                                 en->select_mod_end = EINA_TRUE;
                                 en->selecting = EINA_TRUE;
                              }
                         }
                    }
               }
          }
        else*/
          {
             if (shift)
               {
                  _sel_extend(cur, obj, en);
               }
             else
               {
                  en->selecting = EINA_TRUE;
                  _sel_clear(obj, en);
                  _sel_init(cur, obj, en);
               }
          }
     }

end:
   if (efl2_text_cursor_compare(tc, efl2_text_raw_editable_main_cursor_get(obj)))
     {
        efl_event_callback_call(efl2_text_raw_editable_main_cursor_get(obj), EFL2_TEXT_CURSOR_EVENT_CHANGED_USER, NULL);
     }
   efl_del(tc);

   if (ev->button == 2)
     {
        //FIXME
        //efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_TEXT_PASTE, NULL);
     }
}

static void
_mouse_up_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Eo *obj, void *event_info)
{
   Evas_Coord cx, cy;
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl2_Text_Cursor *cur = efl2_text_raw_editable_main_cursor_get(obj);
   Evas_Event_Mouse_Up *ev = event_info;
   Efl2_Text_Cursor *tc;

   if ((!ev) || (ev->button != 1)) return;

   /* We don't check for ON_HOLD because we'd like to end selection anyway when
    * mouse is up, even if it's held. */

#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        Ecore_IMF_Event_Mouse_Up ecore_ev;
        ecore_imf_evas_event_mouse_up_wrap(ev, &ecore_ev);
        if (ecore_imf_context_filter_event(en->imf_context,
                                           ECORE_IMF_EVENT_MOUSE_UP,
                                           (Ecore_IMF_Event *)&ecore_ev))
          return;
     }
#endif

   /* cx cy are unused but needed in mouse down, please bear with it */
   tc = _cursor_cluster_coord_set(obj, cur, ev->canvas.x, ev->canvas.y, &cx, &cy);

   if (en->select_allow)
     {
        //FIXME
        /*
        if (rp->part->select_mode == EDJE_ENTRY_SELECTION_MODE_EXPLICIT)
          {
             if (en->had_sel)
               {
                  if (en->select_mod_end)
                    sel_extend(cur, obj, en);
                  else if (en->select_mod_start)
                    _sel_preextend(cur, obj, en);
               }
             else
               sel_extend(cur, obj, en);
             //efl2_text_cursor_copy(en->cursor, en->sel_end);
          }
        else*/
          {
             efl2_text_cursor_copy(en->sel_end, cur);
          }
     }
   if (en->selecting)
     {
        if (en->have_selection)
          en->had_sel = EINA_TRUE;
        en->selecting = EINA_FALSE;
     }
   if (efl2_text_cursor_compare(tc, cur))
     {
        efl_event_callback_call(efl2_text_raw_editable_main_cursor_get(obj), EFL2_TEXT_CURSOR_EVENT_CHANGED_USER, NULL);
     }

   _entry_imf_cursor_info_set(en);
   efl_del(tc);
}

static void
_mouse_move_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Eo *obj, void *event_info)
{
   Efl2_Text_Raw_Editable_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl2_Text_Cursor *cur = efl2_text_raw_editable_main_cursor_get(obj);
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord x, y, w, h;
   Efl2_Text_Cursor *tc;
   //Eina_Bool multiline;

   //multiline = efl2_canvas_text_multiline_get(obj);

#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        Ecore_IMF_Event_Mouse_Move ecore_ev;
        ecore_imf_evas_event_mouse_move_wrap(ev, &ecore_ev);
        if (ecore_imf_context_filter_event(en->imf_context,
                                           ECORE_IMF_EVENT_MOUSE_MOVE,
                                           (Ecore_IMF_Event *)&ecore_ev))
          return;
     }
#endif

   if (en->selecting)
     {
        tc = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
        efl2_text_cursor_copy(cur, tc);
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        Eina_Position2D position = {
             .x = ev->cur.canvas.x - x,
             .y = ev->cur.canvas.y - y,
        };
        efl2_text_cursor_coord_set(cur, position);
        //FIXME
        /*if (!efl2_text_cursor_coord_set(cur, cx, cy))
          {
             Evas_Coord lx, ly, lw, lh;

             if (evas_textblock_cursor_line_coord_set(cur, cy) < 0)
               {
                  if (multiline)
                    efl2_text_cursor_paragraph_last(cur);
                  else
                    {
                       efl2_text_cursor_paragraph_first(cur);
                       _entry_cursor_line_geometry_get(obj, cur, &lx, &ly, &lw, &lh);
                       if (!efl2_text_cursor_coord_set(cur, cx, ly + (lh / 2)))
                         efl2_text_cursor_paragraph_last(cur);
                    }
               }
             else
               {
                  _entry_cursor_line_geometry_get(obj, cur, &lx, &ly, &lw, &lh);
                  if (cx <= lx)
                    efl2_text_cursor_line_start(cur);
                  else
                    efl2_text_cursor_line_end(cur);
               }
          }*/

        if (en->select_allow)
          {
             //FIXME
             /*
             if (rp->part->select_mode == EDJE_ENTRY_SELECTION_MODE_EXPLICIT)
               {
                  if (en->had_sel)
                    {
                       if (en->select_mod_end)
                         _sel_extend(cur, obj, en);
                       else if (en->select_mod_start)
                         _sel_preextend(cur, obj, en);
                    }
                  else
                    _sel_extend(cur, obj, en);
               }
             else*/
               {
                  _sel_extend(cur, obj, en);
               }

             if (efl2_text_cursor_compare(en->sel_start, en->sel_end) != 0)
               _sel_enable(cur, obj, en);
             /*if (en->have_selection)
               _sel_update(cur, obj, en);*/
          }
        if (efl2_text_cursor_compare(tc, cur))
          {
             efl_event_callback_call(efl2_text_raw_editable_main_cursor_get(obj), EFL2_TEXT_CURSOR_EVENT_CHANGED_USER, NULL);
          }
        efl_del(tc);
     }
}

EOLIAN static Efl_Object *
_efl2_text_raw_editable_efl_object_constructor(Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   en->select_allow = EINA_TRUE;
   en->editable = EINA_TRUE;
   return obj;
}
EOLIAN static void
_efl2_text_raw_editable_efl_object_destructor(Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *en)
{
   //FIXME check all memory
#ifdef HAVE_ECORE_IMF
   ecore_imf_context_del(en->imf_context);
   en->imf_context = NULL;
#endif

   if (en->pw_timer)
     {
        ecore_timer_del(en->pw_timer);
        en->pw_timer = NULL;
     }
}

EOLIAN static Efl_Object *
_efl2_text_raw_editable_efl_object_finalize(Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
   evas_object_event_callback_add(obj, EVAS_CALLBACK_FOCUS_IN, _focus_in_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_FOCUS_OUT, _focus_out_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_DOWN, _key_down_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_UP, _key_up_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, NULL);

   en->main_cursor = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
   en->sel_start = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));
   en->sel_end = efl_add(EFL2_TEXT_CURSOR_CLASS, obj, efl2_text_cursor_handle_set(efl_added, efl2_canvas_text_cursor_handle_new(obj)));

   // FIXME: efl_event_callback_add(en->sel_start, EFL_CANVAS_TEXT_EVENT_CURSOR_CHANGED, _sel_cursor_changed, obj);
   // FIXME: efl_event_callback_add(en->sel_end, EFL_CANVAS_TEXT_EVENT_CURSOR_CHANGED, _sel_cursor_changed, obj);

   en->input_panel_enable = EINA_TRUE;

#ifdef HAVE_ECORE_IMF
     {
        const char *ctx_id;
        const Ecore_IMF_Context_Info *ctx_info;
        Evas *evas = evas_object_evas_get(obj);
        // _need_imf();

        en->commit_cancel = EINA_FALSE;

        ctx_id = ecore_imf_context_default_id_get();
        if (ctx_id)
          {
             ctx_info = ecore_imf_context_info_by_id_get(ctx_id);
             if (!ctx_info->canvas_type ||
                   strcmp(ctx_info->canvas_type, "evas") == 0)
               {
                  en->imf_context = ecore_imf_context_add(ctx_id);
               }
             else
               {
                  ctx_id = ecore_imf_context_default_id_by_canvas_type_get("evas");
                  if (ctx_id)
                    {
                       en->imf_context = ecore_imf_context_add(ctx_id);
                    }
               }
          }
        else
           en->imf_context = NULL;

        if (!en->imf_context) goto done;

        ecore_imf_context_client_window_set
           (en->imf_context,
            (void *)ecore_evas_window_get
            (ecore_evas_ecore_evas_get(evas)));
        ecore_imf_context_client_canvas_set(en->imf_context, evas);

        ecore_imf_context_retrieve_surrounding_callback_set(en->imf_context,
              _entry_imf_retrieve_surrounding_cb, obj);
        ecore_imf_context_retrieve_selection_callback_set(en->imf_context, _entry_imf_retrieve_selection_cb, obj);
        ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_COMMIT, _entry_imf_event_commit_cb, obj);
        ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_DELETE_SURROUNDING, _entry_imf_event_delete_surrounding_cb, obj);
        ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, _entry_imf_event_preedit_changed_cb, obj);
        ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_SELECTION_SET, _entry_imf_event_selection_set_cb, obj);

#if 0
        // FIXME
        ecore_imf_context_input_mode_set(en->imf_context,
              rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD ?
              ECORE_IMF_INPUT_MODE_INVISIBLE : ECORE_IMF_INPUT_MODE_FULL);

        if (rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD)
           ecore_imf_context_input_panel_language_set(en->imf_context, ECORE_IMF_INPUT_PANEL_LANG_ALPHABET);
#endif

        if (efl2_canvas_text_multiline_get(obj))
           ecore_imf_context_input_hint_set(en->imf_context,
                 ecore_imf_context_input_hint_get(en->imf_context) | ECORE_IMF_INPUT_HINT_MULTILINE);
     }
#else
   goto done;
#endif

done:
   return efl_finalize(efl_super(obj, MY_CLASS));
}

static Efl2_Text_Change_Info *
_text_filter_markup_prepend_internal(Efl2_Canvas_Text *obj, Efl2_Text_Raw_Editable_Data *en,
                                     Efl2_Text_Cursor *c,
                                     char *text,
                                     const char *fmtpre, const char *fmtpost,
                                     Eina_Bool clearsel, Eina_Bool changeinfo)
{
   //Edje_Markup_Filter_Callback *cb;
   //Eina_List *l;
   Eina_Bool have_sel = EINA_FALSE;

   if ((clearsel) && (en->have_selection))
     {
        _sel_range_del_emit(obj, en);
        have_sel = EINA_TRUE;
     }

   /*EINA_LIST_FOREACH(ed->markup_filter_callbacks, l, cb)
     {
        if (!strcmp(cb->part, en->rp->part->name))
          {
             cb->func(cb->data, ed->obj, cb->part, &text);
             if (!text) break;
          }
     }*/
#ifdef HAVE_ECORE_IMF
   // For skipping useless commit
   if (en->have_preedit && (!text || !strcmp(text, "")))
     en->commit_cancel = EINA_TRUE;
   else
     en->commit_cancel = EINA_FALSE;
#endif
   if (text)
     {
        Efl2_Text_Change_Info *info = NULL;

        if (changeinfo)
          {
             info = calloc(1, sizeof(*info));
             if (!info)
               {
                  //FIXME
                  //ERR("Running very low on memory");
               }
             else
               {
                  info->insert = EINA_TRUE;
                  info->content = eina_stringshare_add(text);
                  info->length =
                    eina_unicode_utf8_get_len(info->content);
               }
          }
        if (info)
          {
             if (have_sel)
               {
                  info->merge = EINA_TRUE;
               }
             info->position =
                efl2_text_cursor_position_get(efl2_text_raw_editable_main_cursor_get(obj));
          }
        if (fmtpre) _text_filter_format_prepend(obj, en, efl2_text_raw_editable_main_cursor_get(obj), fmtpre);
        //evas_object_textblock_text_markup_prepend(c, text);
        efl2_text_cursor_text_insert(c, text);
        free(text);
        if (fmtpost) _text_filter_format_prepend(obj, en, efl2_text_raw_editable_main_cursor_get(obj), fmtpost);
        return info;
     }
   return NULL;
}

static Efl2_Text_Change_Info *
_text_filter_text_prepend(Efl2_Canvas_Text *obj, Efl2_Text_Raw_Editable_Data *en,
                          Efl2_Text_Cursor *c,
                          const char *text,
                          const char *fmtpre, const char *fmtpost,
                          Eina_Bool clearsel, Eina_Bool changeinfo)
{
   char *text2 = NULL;
   //Edje_Text_Insert_Filter_Callback *cb;
   //Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(text, NULL);

   if ((clearsel) && (en->have_selection))
     {
        _sel_range_del_emit(obj, en);
     }

   text2 = strdup(text);
   /*EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if (!strcmp(cb->part, en->rp->part->name))
          {
             cb->func(cb->data, ed->obj, cb->part, EDJE_TEXT_FILTER_TEXT, &text2);
             if (!text2) break;
          }
     }*/
   if (text2)
     {
        char *markup_text;
        Efl2_Text_Change_Info *info = NULL;

        markup_text = evas_textblock_text_utf8_to_markup(NULL, text2);
        free(text2);
        if (markup_text)
          info = _text_filter_markup_prepend_internal(obj, en, c, markup_text,
                                                      fmtpre, fmtpost,
                                                      clearsel, changeinfo);
        return info;
     }
   return NULL;
}

static void
_text_filter_format_prepend(Efl2_Canvas_Text *obj, Efl2_Text_Raw_Editable_Data *en,
                            Efl2_Text_Cursor *c, const char *text)
{
   char *text2;
   //Edje_Text_Insert_Filter_Callback *cb;
   //Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(text);
   text2 = strdup(text);
   /*EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if (!strcmp(cb->part, en->rp->part->name))
          {
             cb->func(cb->data, ed->obj, cb->part, EDJE_TEXT_FILTER_FORMAT, &text2);
             if (!text2) break;
          }
     }*/
   if (text2)
     {
        char *s, *markup_text;
        size_t size;

        s = text2;
        if (*s == '+')
          {
             s++;
             while (*s == ' ')
               s++;
             if (!*s)
               {
                  free(text2);
                  return;
               }
             size = strlen(s);
             markup_text = (char *)malloc(size + 3);
             if (markup_text)
               {
                  *(markup_text) = '<';
                  memcpy((markup_text + 1), s, size);
                  *(markup_text + size + 1) = '>';
                  *(markup_text + size + 2) = '\0';
               }
          }
        else if (s[0] == '-')
          {
             s++;
             while (*s == ' ')
               s++;
             if (!*s)
               {
                  free(text2);
                  return;
               }
             size = strlen(s);
             markup_text = (char *)malloc(size + 4);
             if (markup_text)
               {
                  *(markup_text) = '<';
                  *(markup_text + 1) = '/';
                  memcpy((markup_text + 2), s, size);
                  *(markup_text + size + 2) = '>';
                  *(markup_text + size + 3) = '\0';
               }
          }
        else
          {
             size = strlen(s);
             markup_text = (char *)malloc(size + 4);
             if (markup_text)
               {
                  *(markup_text) = '<';
                  memcpy((markup_text + 1), s, size);
                  *(markup_text + size + 1) = '/';
                  *(markup_text + size + 2) = '>';
                  *(markup_text + size + 3) = '\0';
               }
          }
        free(text2);
        if (markup_text)
          _text_filter_markup_prepend_internal(obj, en, c, markup_text,
                                               NULL, NULL,
                                               EINA_FALSE, EINA_FALSE);
     }
}

EOLIAN static void
_efl2_text_raw_editable_selection_allowed_set(Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *pd, Eina_Bool allowed)
{
   pd->select_allow = allowed;
}

EOLIAN static Eina_Bool
_efl2_text_raw_editable_selection_allowed_get(const Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *pd)
{
   return pd->select_allow;
}

EOLIAN static void
_efl2_text_raw_editable_selection_cursors_get(const Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *pd, const Efl2_Text_Cursor **start, const Efl2_Text_Cursor **end)
{
   if (efl2_text_cursor_position_get(pd->sel_start) >
         efl2_text_cursor_position_get(pd->sel_end))
     {
        if (start) *start = pd->sel_end;
        if (end) *end = pd->sel_start;
     }
   else
     {
        if (start) *start = pd->sel_start;
        if (end) *end = pd->sel_end;
     }
}

EOLIAN static void
_efl2_text_raw_editable_editable_set(Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *sd, Eina_Bool editable)
{
   sd->editable = editable;
}

EOLIAN static Eina_Bool
_efl2_text_raw_editable_editable_get(const Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *sd)
{
   return sd->editable;
}


EOLIAN static void
_efl2_text_raw_editable_password_mode_set(Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *pd EINA_UNUSED, Eina_Bool enabled EINA_UNUSED)
{
   // FIXME: password mode is just watches changes and updates an internal buffer in tandem to the text changes
   // Can use the same mechanism as undo/redo
#warning IMPLEMENTATION
}


EOLIAN static Eina_Bool
_efl2_text_raw_editable_password_mode_get(const Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
#warning IMPLEMENTATION
}


EOLIAN static void
_efl2_text_raw_editable_replacement_char_set(Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *pd EINA_UNUSED, const char *repch EINA_UNUSED)
{
#warning IMPLEMENTATION
}


EOLIAN static const char *
_efl2_text_raw_editable_replacement_char_get(const Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *pd EINA_UNUSED)
{
#warning IMPLEMENTATION
   return NULL;
}


EOLIAN static Efl2_Text_Cursor *
_efl2_text_raw_editable_main_cursor_get(const Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *pd)
{
   return pd->main_cursor;
}


#warning implement by just manipulating the cursors
EOLIAN static void
_efl2_text_raw_editable_selection_start(Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
   Efl2_Text_Cursor *cur = efl2_text_raw_editable_main_cursor_get(obj);
   _sel_clear(obj, en);
   _sel_enable(cur, obj, en);
   _sel_init(cur, obj, en);
   _sel_extend(cur, obj, en);
}


#warning implement by just manipulating the cursors
EOLIAN static void
_efl2_text_raw_editable_selection_end(Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
   Efl2_Text_Cursor *cur = efl2_text_raw_editable_main_cursor_get(obj);
   _sel_extend(cur, obj, en);
}

#warning implement by just manipulating the cursors
EOLIAN static void
_efl2_text_raw_editable_selection_none(Eo *obj, Efl2_Text_Raw_Editable_Data *pd)
{
   _sel_clear(obj, pd);
}

#warning NEW_METHOD
EOLIAN static void
_efl2_text_raw_editable_select_all(Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
   Efl2_Text_Cursor *cur = efl2_text_raw_editable_main_cursor_get(obj);
   _entry_imf_context_reset(en);

   _sel_clear(obj, en);
   efl2_text_cursor_paragraph_first(cur);
   _entry_imf_context_reset(en);
   _sel_init(cur, obj, en);
   efl2_text_cursor_paragraph_last(cur);
   _sel_extend(cur, obj, en);
}

#warning NEW_METHOD
EOLIAN static void
_efl2_text_raw_editable_select_abort(Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *en)
{
   if (en->selecting)
     {
        en->selecting = EINA_FALSE;

        _entry_imf_context_reset(en);
        _entry_imf_cursor_info_set(en);
     }
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_panel_layout_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, Efl2_Input_Text_Panel_Layout layout)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_layout_set(en->imf_context, (Ecore_IMF_Input_Panel_Layout)layout);
#else
   (void)en;
   (void)obj;
   (void)layout;
#endif
}


EOLIAN static Efl2_Input_Text_Panel_Layout
_efl2_text_raw_editable_efl2_input_text_input_panel_layout_get(const Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return (Efl2_Input_Text_Panel_Layout)ecore_imf_context_input_panel_layout_get(en->imf_context);
   return EFL2_INPUT_TEXT_PANEL_LAYOUT_INVALID;
#else
   return EFL2_INPUT_TEXT_PANEL_LAYOUT_INVALID;
   (void)obj;
   (void)en;
#endif
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_panel_layout_variation_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, int variation)
{
   if (!en) return;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_layout_variation_set(en->imf_context, variation);
#else
   (void)obj;
   (void)en;
   (void)variation;
#endif
}


EOLIAN static int
_efl2_text_raw_editable_efl2_input_text_input_panel_layout_variation_get(const Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
   if (!en) return 0;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return ecore_imf_context_input_panel_layout_variation_get(en->imf_context);
#else
   (void)obj;
   (void)en;
#endif

   return 0;
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_panel_enabled_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, Eina_Bool enabled)
{
   en->input_panel_enable = enabled;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_enabled_set(en->imf_context, enabled);
#else
   (void)obj;
   (void)en;
   (void)enabled;
#endif
}


EOLIAN static Eina_Bool
_efl2_text_raw_editable_efl2_input_text_input_panel_enabled_get(const Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
   return en->input_panel_enable;
   (void)obj;
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_panel_show(Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_show(en->imf_context);
#else
   (void)obj;
   (void)en;
#endif
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_panel_hide(Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_hide(en->imf_context);
#else
   (void)obj;
   (void)en;
#endif
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_panel_language_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, Efl2_Input_Text_Panel_Lang lang)
{
   en->input_panel_lang = lang;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_language_set(en->imf_context, (Ecore_IMF_Input_Panel_Lang)lang);
#else
   (void)obj;
   (void)en;
   (void)lang;
#endif
}


EOLIAN static Efl2_Input_Text_Panel_Lang
_efl2_text_raw_editable_efl2_input_text_input_panel_language_get(const Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
   return en->input_panel_enable;
   (void)obj;
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_panel_imdata_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, const void *data, int len)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_imdata_set(en->imf_context, data, len);
#else
   (void)obj;
   (void)en;
   (void)data;
   (void)len;
#endif
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_panel_imdata_get(const Eo *obj, Efl2_Text_Raw_Editable_Data *en, void *data, int *len)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_imdata_get(en->imf_context, data, len);
#else
   (void)obj;
   (void)en;
   (void)data;
   (void)len;
#endif
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_panel_return_key_type_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, Efl2_Input_Text_Panel_Return_Key_Type return_key_type)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_return_key_type_set(en->imf_context, (Ecore_IMF_Input_Panel_Return_Key_Type)return_key_type);
#else
   (void)obj;
   (void)en;
   (void)return_key_type;
#endif
}


EOLIAN static Efl2_Input_Text_Panel_Return_Key_Type
_efl2_text_raw_editable_efl2_input_text_input_panel_return_key_type_get(const Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return (Efl2_Input_Text_Panel_Return_Key_Type)ecore_imf_context_input_panel_return_key_type_get(en->imf_context);
   return EFL2_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_DEFAULT;
#else 
   return EFL2_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_DEFAULT;
   (void)obj;
   (void)en;
#endif
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_panel_return_key_disabled_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, Eina_Bool disabled)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_return_key_disabled_set(en->imf_context, disabled);
#else
   (void)obj;
   (void)en;
   (void)disabled;
#endif
}


EOLIAN static Eina_Bool
_efl2_text_raw_editable_efl2_input_text_input_panel_return_key_disabled_get(const Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return ecore_imf_context_input_panel_return_key_disabled_get(en->imf_context);
   return EINA_FALSE;
#else
   return EINA_FALSE;
   (void)obj;
   (void)en;
#endif
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_panel_show_on_demand_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, Eina_Bool ondemand)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_show_on_demand_set(en->imf_context, ondemand);
#else
   (void)obj;
   (void)en;
   (void)ondemand;
#endif
}


EOLIAN static Eina_Bool
_efl2_text_raw_editable_efl2_input_text_input_panel_show_on_demand_get(const Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        Eina_Bool ret = ecore_imf_context_input_panel_show_on_demand_get(en->imf_context);
        return ret;
     }
#else
   (void)obj;
   (void)en;
#endif
   return EINA_FALSE;
}


#warning NEW_METHOD
EOLIAN static void
_efl2_text_raw_editable_prediction_hint_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, const char *prediction_hint)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_prediction_hint_set(en->imf_context, prediction_hint);
   (void)obj;
#else
   (void)obj;
   (void)en;
   (void)prediction_hint;
#endif
}

#warning NEW_METHOD
EOLIAN static Eina_Bool
_efl2_text_raw_editable_prediction_hint_hash_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, const char *key, const char *value)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return ecore_imf_context_prediction_hint_hash_set(en->imf_context, key, value);
   (void)obj;
#else
   (void)obj;
   (void)en;
   (void)key;
   (void)value;
#endif

   return EINA_FALSE;
}

#warning NEW_METHOD
EOLIAN static Eina_Bool
_efl2_text_raw_editable_prediction_hint_hash_del(Eo *obj, Efl2_Text_Raw_Editable_Data *en, const char *key)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return ecore_imf_context_prediction_hint_hash_del(en->imf_context, key);
   (void)obj;
#else
   (void)obj;
   (void)en;
   (void)key;
#endif

   return EINA_FALSE;
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_prediction_allow_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, Eina_Bool prediction)
{
   en->prediction_allow = prediction;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_prediction_allow_set(en->imf_context, prediction);
#else
   (void)obj;
   (void)en;
   (void)prediction;
#endif
}


EOLIAN static Eina_Bool
_efl2_text_raw_editable_efl2_input_text_prediction_allow_get(const Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
   return en->prediction_allow;
   (void)obj;
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_hint_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, Efl2_Input_Text_Hints input_hints)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_hint_set(en->imf_context, (Ecore_IMF_Input_Hints)input_hints);
   (void)obj;
#else
   (void)obj;
   (void)en;
   (void)input_hints;
#endif
}


EOLIAN static Efl2_Input_Text_Hints
_efl2_text_raw_editable_efl2_input_text_input_hint_get(const Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return (Efl2_Input_Text_Hints)ecore_imf_context_input_hint_get(en->imf_context);
   (void)obj;
#else
   (void)obj;
   (void)en;
#endif

   return ELM_INPUT_HINT_NONE;
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_capitalize_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, Efl2_Input_Text_Capitalize_Type autocapital_type)
{
#ifdef HAVE_ECORE_IMF
   if (efl2_text_raw_editable_password_mode_get(obj) == EINA_TRUE)
     autocapital_type = EFL2_INPUT_TEXT_CAPITALIZE_TYPE_NONE;

   if (en->imf_context)
     ecore_imf_context_autocapital_type_set(en->imf_context, (Ecore_IMF_Autocapital_Type)autocapital_type);

   (void)obj;
#else
   (void)obj;
   (void)en;
   (void)autocapital_type;
#endif
}


EOLIAN static Efl2_Input_Text_Capitalize_Type
_efl2_text_raw_editable_efl2_input_text_capitalize_get(const Eo *obj, Efl2_Text_Raw_Editable_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return (Efl2_Input_Text_Capitalize_Type)ecore_imf_context_autocapital_type_get(en->imf_context);
   return EFL2_INPUT_TEXT_CAPITALIZE_TYPE_NONE;
   (void)obj;
#else
   return EFL2_INPUT_TEXT_CAPITALIZE_TYPE_NONE;
   (void)obj;
   (void)en;
#endif
}


EOLIAN static void
_efl2_text_raw_editable_efl2_input_text_input_panel_return_key_autoenabled_set(Eo *obj, Efl2_Text_Raw_Editable_Data *en, Eina_Bool enabled)
{
   Eina_Bool return_key_disabled = EINA_FALSE;
   en->auto_return_key = enabled;
   if (!en->auto_return_key) return;

   if (efl2_canvas_text_is_empty_get(obj) == EINA_TRUE)
     return_key_disabled = EINA_TRUE;
   efl2_input_text_input_panel_return_key_disabled_set(obj, return_key_disabled);
}

EOLIAN static void
_efl2_text_raw_editable_text_filter(Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *pd EINA_UNUSED, Efl2_Text_Change_Info *change_info, const char *new_text)
{
   eina_stringshare_replace(&change_info->content, new_text);
   change_info->length = eina_unicode_utf8_get_len(new_text);
}

EOLIAN static void
_efl2_text_raw_editable_validation_failed(Eo *obj EINA_UNUSED, Efl2_Text_Raw_Editable_Data *pd EINA_UNUSED, const char *reason)
{
   #warning IMPLEMENTATION
}

EOLIAN static unsigned int
_efl2_text_cursor_interactive_user_text_insert(Eo *obj, void *pd EINA_UNUSED, const char *text)
{
   // FIXME: we need to call the changed user event but only after filtering has happened. Need to figure this out.
   return efl2_text_cursor_text_insert(obj, text);
}

#include "efl2_text_raw_editable.eo.c"
#include "efl2_text_cursor_interactive.eo.c"
