#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "elm_priv.h"
#include "efl_ui_internal_text_interactive.h"
#include "efl_canvas_text_internal.h"

#define MY_CLASS EFL_UI_INTERNAL_TEXT_INTERACTIVE_CLASS

#define _PARAGRAPH_SEPARATOR_UTF8 "\xE2\x80\xA9"


typedef struct _Efl_Ui_Internal_Text_Interactive_Data
{
   Efl_Text_Cursor       *sel_start, *sel_end;
   Efl_Text_Cursor       *main_cursor;
   Efl_Text_Cursor       *preedit_start, *preedit_end;
   Eina_List             *seq;
   char                  *selection;
   Eina_Bool              composing : 1;
   Eina_Bool              selecting : 1;
   Eina_Bool              have_selection : 1;
   Eina_Bool              select_allow : 1;
   Eina_Bool              editable : 1;
   Eina_Bool              had_sel : 1;
   Eina_Bool              input_panel_enable : 1;
   Eina_Bool              prediction_allow : 1;
   Eina_Bool              anchors_updated : 1;

#ifdef HAVE_ECORE_IMF
   Eina_Bool              have_preedit : 1;
   Eina_Bool              commit_cancel : 1; // For skipping useless commit
   Ecore_IMF_Context     *imf_context;
#endif
} Efl_Ui_Internal_Text_Interactive_Data;

static void _sel_range_del_emit(Evas_Object *obj, Efl_Ui_Internal_Text_Interactive_Data *en);
static void _sel_init(Efl_Text_Cursor *c, Evas_Object *o, Efl_Ui_Internal_Text_Interactive_Data *en);
static void _sel_enable(Efl_Text_Cursor *c EINA_UNUSED, Evas_Object *o EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en);
static void _sel_extend(Efl_Text_Cursor *c, Evas_Object *o, Efl_Ui_Internal_Text_Interactive_Data *en);
static void _sel_clear(Evas_Object *o EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en);
static const char *_entry_selection_get(Efl_Ui_Internal_Text_Interactive *obj, Efl_Ui_Internal_Text_Interactive_Data *en);
static void _entry_imf_cursor_info_set(Eo *obj, Efl_Text_Cursor *cur, Efl_Ui_Internal_Text_Interactive_Data *en);

#ifdef HAVE_ECORE_IMF
static void
_preedit_clear(Efl_Ui_Internal_Text_Interactive_Data *en)
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
_preedit_del(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   if (!en || !en->have_preedit) return;
   if (!en->preedit_start || !en->preedit_end) return;
   if (efl_text_cursor_equal(en->preedit_start, en->preedit_end)) return;

   /* delete the preedit characters */
   efl_text_cursor_range_delete(en->preedit_start, en->preedit_end);
}

static Eina_Bool
_entry_imf_retrieve_surrounding_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, char **text, int *cursor_pos)
{
   Efl_Canvas_Text *obj = data;
   Efl_Text_Cursor *cur = efl_text_interactive_main_cursor_get(obj);
   const char *str;

   if (text)
     {
        str = efl_text_get(obj);
        if (str)
          {
             *text = strdup(str);
          }
        else
          *text = strdup("");
     }

   if (cursor_pos)
     {
        if (cur)
          *cursor_pos = efl_text_cursor_position_get(cur);
        else
          *cursor_pos = 0;
     }

   return EINA_TRUE;
}

static void
_entry_imf_event_commit_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info)
{
   Efl_Canvas_Text *obj = data;
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   char *commit_str = event_info;

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
   _preedit_del(obj, en);
   _preedit_clear(en);

   // Skipping commit process when it is useless
   if (en->commit_cancel)
     {
        en->commit_cancel = EINA_FALSE;
        return;
     }

#if 0
   Edje_Entry_Change_Info *info = NULL;
   if ((rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD) &&
       _password_show_last)
     _entry_hide_visible_password(en->rp);
   if ((rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD) &&
       _password_show_last && (!en->preedit_start))
     {
        info = _text_filter_text_prepend(en, cur, commit_str,
                                         "+ password=off", "- password",
                                         EINA_TRUE, EINA_TRUE);
        if (info)
          {
             if (en->pw_timer)
               {
                  ecore_timer_del(en->pw_timer);
                  en->pw_timer = NULL;
               }
             if (_password_show_last_timeout >= 0)
               en->pw_timer = ecore_timer_add
                   (_password_show_last_timeout,
                   _password_timer_cb, en);
          }
     }
   else
     {
        info = _text_filter_text_prepend(en, cur, commit_str,
                                         NULL, NULL,
                                         EINA_TRUE, EINA_TRUE);
     }

   _entry_imf_cursor_info_set(en);
   _anchors_get(cur, obj, en);
   if (info)
     {
        _emit("entry,changed", rp->part->name);
        _emit_full("entry,changed,user", rp->part->name,
                        info, _free_entry_change_info);
        _emit("cursor,changed", rp->part->name);
     }
   _entry_imf_cursor_info_set(en);
   _entry_real_part_configure(rp);
#endif
}

static void
_entry_imf_event_preedit_changed_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Canvas_Text *obj = data;
   Efl_Text_Cursor *cur = efl_text_interactive_main_cursor_get(obj);
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
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
   _preedit_del(obj, en);

   preedit_start_pos = efl_text_cursor_position_get(cur);

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
#if 0
        if ((rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD) &&
            _password_show_last)
          {
             Edje_Entry_Change_Info *info;

             _entry_hide_visible_password(en->rp);
             info = _text_filter_markup_prepend(en, cur,
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
                  if (_password_show_last_timeout >= 0)
                    en->pw_timer = ecore_timer_add
                        (_password_show_last_timeout,
                        _password_timer_cb, en);
                  free(info);
               }
          }
        else
          _text_filter_markup_prepend(en, cur,
                                      eina_strbuf_string_get(buf),
                                      NULL, NULL,
                                      EINA_TRUE, EINA_FALSE);
#endif
        eina_strbuf_free(buf);
     }

   if (!preedit_end_state)
     {
        /* set preedit start cursor */
        if (!en->preedit_start)
          en->preedit_start = efl_canvas_text_cursor_create(obj);
        efl_text_cursor_copy(cur, en->preedit_start);

        /* set preedit end cursor */
        if (!en->preedit_end)
          en->preedit_end = efl_canvas_text_cursor_create(obj);
        efl_text_cursor_copy(cur, en->preedit_end);

        preedit_end_pos = efl_text_cursor_position_get(cur);

        for (i = 0; i < (preedit_end_pos - preedit_start_pos); i++)
          {
             efl_text_cursor_move(en->preedit_start, EFL_TEXT_CURSOR_MOVE_TYPE_CHAR_PREV);
          }

        en->have_preedit = EINA_TRUE;

        /* set cursor position */
        efl_text_cursor_position_set(cur, preedit_start_pos + cursor_pos);
     }

   _entry_imf_cursor_info_set(obj, cur, en);

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
   Efl_Canvas_Text *obj = data;
   Efl_Text_Cursor *cur = efl_text_interactive_main_cursor_get(obj);
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Ecore_IMF_Event_Delete_Surrounding *ev = event_info;
   Efl_Text_Cursor *del_start, *del_end;
   Efl_Ui_Text_Change_Info info = { NULL, 0, 0, 0, 0 };
   int cursor_pos;
   int start, end;
   char *tmp;

   cursor_pos = efl_text_cursor_position_get(cur);

   del_start = efl_canvas_text_cursor_create(obj);
   efl_text_cursor_position_set(del_start, cursor_pos + ev->offset);

   del_end = efl_canvas_text_cursor_create(obj);
   efl_text_cursor_position_set(del_end, cursor_pos + ev->offset + ev->n_chars);

   start = efl_text_cursor_position_get(del_start);
   end = efl_text_cursor_position_get(del_end);
   if (start == end) goto end;

   tmp = efl_text_cursor_range_text_get(del_start, del_end);

   info.insert = EINA_FALSE;
   info.position = start;
   info.length = end - start;
   info.content = tmp;

   efl_text_cursor_range_delete(del_start, del_end);

   efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED_USER, &info);
   free(tmp);

   _entry_imf_cursor_info_set(obj, cur, en);

end:
   efl_del(del_start);
   efl_del(del_end);
}

static void
_entry_imf_event_selection_set_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info)
{
   Efl_Canvas_Text *obj = data;
   Efl_Text_Cursor *cur = efl_text_interactive_main_cursor_get(obj);
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Ecore_IMF_Event_Selection *ev = event_info;

   if (ev->start == ev->end)
     {
        efl_text_cursor_position_set(cur, ev->start);
     }
   else
     {
        _sel_clear(obj, en);
        efl_text_cursor_position_set(cur, ev->start);
        _sel_enable(cur, obj, en);
        _sel_init(cur, obj, en);
        efl_text_cursor_position_set(cur, ev->end);
        _sel_extend(cur, obj, en);
     }
}

static Eina_Bool
_entry_imf_retrieve_selection_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, char **text)
{
   Efl_Canvas_Text *obj = data;
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
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

static void
_entry_imf_cursor_location_set(Eo *obj EINA_UNUSED, Efl_Text_Cursor *cur, Efl_Ui_Internal_Text_Interactive_Data *en)
{
#ifdef HAVE_ECORE_IMF
   Eina_Rect rect;
   if (!en->imf_context) return;

   rect = efl_text_cursor_geometry_get(cur, EFL_TEXT_CURSOR_TYPE_BEFORE);
   ecore_imf_context_cursor_location_set(en->imf_context, rect.x, rect.y, rect.w, rect.h);
   // FIXME: ecore_imf_context_bidi_direction_set(en->imf_context, (Ecore_IMF_BiDi_Direction)dir);
#else
   (void)en;
#endif
}

static void
_entry_imf_cursor_info_set(Eo *obj, Efl_Text_Cursor *cur, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   int cursor_pos;

#ifdef HAVE_ECORE_IMF
   if (!en->imf_context) return;

   if (en->have_selection)
     {
        if (efl_text_cursor_compare(en->sel_start, en->sel_end) < 0)
          cursor_pos = efl_text_cursor_position_get(en->sel_start);
        else
          cursor_pos = efl_text_cursor_position_get(en->sel_end);
     }
   else
     cursor_pos = efl_text_cursor_position_get(cur);

   ecore_imf_context_cursor_position_set(en->imf_context, cursor_pos);

   _entry_imf_cursor_location_set(obj, cur, en);
#else
   (void)en;
#endif
}

static void
_focus_in_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
#ifdef HAVE_ECORE_IMF
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl_Text_Cursor *cur;

   if (!en->imf_context) return;

   cur = efl_text_interactive_main_cursor_get(obj);
   ecore_imf_context_focus_in(en->imf_context);
   _entry_imf_cursor_info_set(obj, cur, en);
#endif
}

void
_entry_imf_context_reset(Efl_Ui_Internal_Text_Interactive_Data *en)
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
_focus_out_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
#ifdef HAVE_ECORE_IMF
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   if (!en->imf_context) return;

   ecore_imf_context_reset(en->imf_context);
   ecore_imf_context_focus_out(en->imf_context);
#endif
}

static const char *
_entry_selection_get(Efl_Ui_Internal_Text_Interactive *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   if ((!en->selection) && (en->have_selection))
     en->selection = efl_text_cursor_range_text_get(en->sel_start, en->sel_end);
   return en->selection;
}

static void
_sel_cursor_changed(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
//   Eo *obj = data;
}

static void
_sel_init(Efl_Text_Cursor *c, Evas_Object *o EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   if (en->have_selection)
      return;

   efl_text_cursor_copy(c, en->sel_start);
   efl_text_cursor_copy(c, en->sel_end);

   en->have_selection = EINA_FALSE;
   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }
}

static void
_sel_enable(Efl_Text_Cursor *c EINA_UNUSED,
            Evas_Object *o EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   if (en->have_selection) return;
   en->have_selection = EINA_TRUE;
   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }

   _entry_imf_context_reset(en);
}

static void
_sel_extend(Efl_Text_Cursor *c, Evas_Object *o, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   if (!en->sel_end) return;
   _sel_enable(c, o, en);
   if (efl_text_cursor_equal(c, en->sel_end)) return;

   efl_text_cursor_copy(c, en->sel_end);

   _entry_imf_cursor_info_set(o, c, en);

   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }
   efl_event_callback_call(o, EFL_TEXT_INTERACTIVE_EVENT_TEXT_SELECTION_CHANGED, NULL);
}

static void
_sel_clear(Evas_Object *o EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
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
        efl_text_cursor_copy(en->sel_start, en->sel_end);
        efl_event_callback_call(o, EFL_TEXT_INTERACTIVE_EVENT_TEXT_SELECTION_CHANGED, NULL);
     }
}

static void
_efl_ui_internal_text_interactive_efl_text_interactive_select_none(
      Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   _sel_clear(obj, en);
}

static void
_range_del_emit(Evas_Object *obj, Efl_Text_Cursor *cur1, Efl_Text_Cursor *cur2)
{
   size_t start, end;
   char *tmp;
   Efl_Ui_Text_Change_Info info = { NULL, 0, 0, 0, 0 };

   start = efl_text_cursor_position_get(cur1);
   end = efl_text_cursor_position_get(cur2);
   if (start == end)
      return;

   info.insert = EINA_FALSE;
   info.position = start;
   info.length = end - start;

   tmp = efl_text_cursor_range_text_get(cur1, cur2);
   info.content = tmp;

   efl_text_cursor_range_delete(cur1, cur2);

   efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED_USER, &info);
   efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED, NULL);
   if (tmp) free(tmp);
}

static void
_sel_range_del_emit(Evas_Object *obj, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   _range_del_emit(obj, en->sel_start, en->sel_end);
   _sel_clear(obj, en);
}

static void
_delete_emit(Eo *obj, Efl_Text_Cursor *c, Efl_Ui_Internal_Text_Interactive_Data *en EINA_UNUSED, size_t pos)
{
   Efl_Ui_Text_Change_Info info = { NULL, 0, 0, 0, 0 };
   Eina_Unicode content[2];
   content[0] = efl_text_cursor_content_get(c);
   content[1] = 0;
   if (!content[0])
      return;

   char *tmp = eina_unicode_unicode_to_utf8(content, NULL);

   info.insert = EINA_FALSE;
   info.position = pos;
   info.length = 1;
   info.content = tmp;

   efl_text_cursor_char_delete(c);

   efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED_USER, &info);
   efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED, NULL);
   if (tmp) free(tmp);
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
_compose_seq_reset(Efl_Ui_Internal_Text_Interactive_Data *en)
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
_key_down_sel_pre(Efl_Ui_Internal_Text_Interactive *obj, Efl_Text_Cursor *cur, Efl_Ui_Internal_Text_Interactive_Data *en, Eina_Bool shift, Eina_Bool movement_forward)
{
   if (en->select_allow)
     {
        if (shift)
          {
             _sel_init(cur, obj, en);
          }
        else if (en->have_selection)
          {
             Eina_Bool sel_forward = efl_text_cursor_compare(en->sel_start, en->sel_end);
             if ((sel_forward && movement_forward) || (!sel_forward && !movement_forward))
                efl_text_cursor_copy(en->sel_end, cur);
             else
                efl_text_cursor_copy(en->sel_start, cur);
             _sel_clear(obj, en);
          }
     }
}

static void
_key_down_sel_post(Efl_Ui_Internal_Text_Interactive *obj, Efl_Text_Cursor *cur, Efl_Ui_Internal_Text_Interactive_Data *en, Eina_Bool shift)
{
   if (en->select_allow)
     {
        if (shift) _sel_extend(cur, obj, en);
        else _sel_clear(obj, en);
     }
}

static void
_key_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Efl_Text_Cursor *cur;
   Eina_Bool control, alt, shift;
#if defined(__APPLE__) && defined(__MACH__)
   Eina_Bool super, altgr;
#endif
   Eina_Bool multiline;
   int old_cur_pos;
   char *string = (char *)ev->string;
   Eina_Bool free_string = EINA_FALSE;
   Eina_Bool changed_user = EINA_FALSE;
   Efl_Ui_Text_Change_Info info = { NULL, 0, 0, 0, 0 };

   if (!ev->key) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);

   /* FIXME: Maybe allow selctions to happen even when not editable. */
   if (!en->editable) return;

   cur = efl_text_interactive_main_cursor_get(obj);
   old_cur_pos = efl_text_cursor_position_get(cur);
   if (old_cur_pos < 0) return;

   control = evas_key_modifier_is_set(ev->modifiers, "Control");
   alt = evas_key_modifier_is_set(ev->modifiers, "Alt");
   shift = evas_key_modifier_is_set(ev->modifiers, "Shift");
#if defined(__APPLE__) && defined(__MACH__)
   super = evas_key_modifier_is_set(ev->modifiers, "Super");
   altgr = evas_key_modifier_is_set(ev->modifiers, "AltGr");
#endif
   multiline = efl_text_multiline_get(obj);

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
             if (shift || efl_canvas_text_newline_as_paragraph_separator_get(obj))
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

             efl_text_cursor_line_jump_by(cur, -1);
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

             _key_down_sel_post(obj, cur, en, shift);
          }
     }
   else if (!strcmp(ev->key, "Down") ||
            (!strcmp(ev->key, "KP_Down") && !ev->string))
     {
        _compose_seq_reset(en);
        if (multiline)
          {
             _key_down_sel_pre(obj, cur, en, shift, EINA_TRUE);

             efl_text_cursor_line_jump_by(cur, 1);
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

             _key_down_sel_post(obj, cur, en, shift);
          }
     }
   else if (!strcmp(ev->key, "Left") ||
            (!strcmp(ev->key, "KP_Left") && !ev->string))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_FALSE);

        efl_text_cursor_move(cur,EFL_TEXT_CURSOR_MOVE_TYPE_CHAR_PREV);
#if defined(__APPLE__) && defined(__MACH__)
        if (altgr) efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START);
#else
        /* If control is pressed, go to the start of the word */
        if (control) efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START);
#endif
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        _key_down_sel_post(obj, cur, en, shift);
     }
   else if (!strcmp(ev->key, "Right") ||
            (!strcmp(ev->key, "KP_Right") && !ev->string))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_TRUE);

#if defined(__APPLE__) && defined(__MACH__)
        if (altgr) efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END);
#else
        /* If control is pressed, go to the end of the word */
        if (control) efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END);
#endif
        efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHAR_NEXT);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        _key_down_sel_post(obj, cur, en, shift);
     }
   else if (!strcmp(ev->key, "BackSpace"))
     {
        _compose_seq_reset(en);
        if (control && !en->have_selection)
          {
             // del to start of previous word
             Efl_Text_Cursor *tc = efl_canvas_text_cursor_create(obj);

             efl_text_cursor_copy(tc, cur);
             efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHAR_PREV);
             efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START);

             _range_del_emit(obj, cur, tc);

             //efl_del(tc);
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
                  if (efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHAR_PREV))
                    {
                       _delete_emit(obj, cur, en, old_cur_pos - 1);
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
             Efl_Text_Cursor *tc = efl_canvas_text_cursor_create(obj);

             efl_text_cursor_copy(tc, cur);
             efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END);
             efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHAR_NEXT);

             _range_del_emit(obj, cur, tc);

             //efl_del(tc);
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
                  _delete_emit(obj, cur, en, old_cur_pos);
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
           efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_FIRST);
        else
           efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_START);

        _key_down_sel_post(obj, cur, en, shift);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((!alt) &&
            (!strcmp(ev->key, "End") ||
             ((!strcmp(ev->key, "KP_End")) && !ev->string)))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_TRUE);

        if ((control) && (multiline))
           efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_LAST);
        else
           efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_END);

        _key_down_sel_post(obj, cur, en, shift);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (shift && !strcmp(ev->key, "Tab"))
     {
        _compose_seq_reset(en);
        if (multiline)
          {
             // remove a tab
          }
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

        efl_text_cursor_line_jump_by(cur, -10);

        _key_down_sel_post(obj, cur, en, shift);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Next") ||
            (!strcmp(ev->key, "KP_Next") && !ev->string))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_TRUE);

        efl_text_cursor_line_jump_by(cur, 10);

        _key_down_sel_post(obj, cur, en, shift);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else
     {
        char *compres = NULL;
        Ecore_Compose_State state;

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
             if (en->have_selection)
               {
                  _sel_range_del_emit(obj, en);
                  info.merge = EINA_TRUE;
               }
             info.insert = EINA_TRUE;
             info.content = string;
             info.position = efl_text_cursor_position_get(cur);
             info.length = eina_unicode_utf8_get_len(string);

             efl_event_freeze(obj);
             efl_text_cursor_text_insert(cur, string);
             efl_event_thaw(obj);
             changed_user = EINA_TRUE;

             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

             if (free_string) free(string);
          }
     }
end:
   if (changed_user)
     {
        efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED_USER, &info);
        /* FIXME: this is kinda gross */
        efl_event_callback_call(obj, EFL_UI_TEXT_EVENT_CHANGED, NULL);
     }
   (void) 0;
}

static void
_cursor_char_coord_set(Efl_Canvas_Text *obj, Efl_Text_Cursor *cur, Evas_Coord canvasx, Evas_Coord canvasy, Evas_Coord *_cx, Evas_Coord *_cy)
{
   Evas_Coord cx, cy;
   Evas_Coord x, y, lh = 0, cly = 0;
   Efl_Text_Cursor *line_cur;
   Efl_Text_Cursor *tc;

   tc = efl_canvas_text_cursor_create(obj);
   efl_text_cursor_copy(cur, tc);
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   cx = canvasx - x;
   cy = canvasy - y;

   line_cur = efl_canvas_text_cursor_create(obj);
   efl_text_cursor_move(line_cur, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_LAST);
   evas_textblock_cursor_line_geometry_get(efl_text_cursor_handle_get(line_cur), NULL, &cly, NULL, &lh);
   /* Consider a threshold of half the line height */
   if (cy > (cly + lh) && cy < (cly + lh + lh / 2))
     {
        cy = cly + lh - 1; // Make it inside Textblock
     }
   efl_text_cursor_move(line_cur, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_FIRST);
   evas_textblock_cursor_line_geometry_get(efl_text_cursor_handle_get(line_cur), NULL, &cly, NULL, NULL);

   if (cy < cly && cy > (cly - lh / 2))
     {
        cy = cly;
     }
   efl_del(line_cur);
   /* No need to check return value if not able to set the char coord Textblock
    * will take care */
   efl_text_cursor_char_coord_set(cur, EINA_POSITION2D(cx, cy));
   if (_cx) *_cx = cx;
   if (_cy) *_cy = cy;
}

static void
_mouse_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Coord cx, cy;
   Evas_Event_Mouse_Down *ev = event_info;
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl_Text_Cursor *cur = efl_text_interactive_main_cursor_get(obj);
   Efl_Text_Cursor *tc = NULL;
   Eina_Bool dosel = EINA_FALSE;
   Eina_Bool shift;

   if ((ev->button != 1) && (ev->button != 2)) return;

#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        Ecore_IMF_Event_Mouse_Down ecore_ev;
        // ecore_imf_evas_event_mouse_down_wrap(ev, &ecore_ev);
        if (ecore_imf_context_filter_event(en->imf_context,
                                           ECORE_IMF_EVENT_MOUSE_DOWN,
                                           (Ecore_IMF_Event *)&ecore_ev))
          return;
     }
#endif

   _entry_imf_context_reset(en);

   shift = evas_key_modifier_is_set(ev->modifiers, "Shift");

   if (en->select_allow && ev->button != 2) dosel = EINA_TRUE;
   if (dosel)
     {
        if (ev->flags & EVAS_BUTTON_TRIPLE_CLICK)
          {
             if (shift)
               {
                  tc = efl_canvas_text_cursor_create(obj);
                  efl_text_cursor_copy(cur, tc);
                  if (efl_text_cursor_compare(cur, en->sel_start) < 0)
                    efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_START);
                  else
                    efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_END);
                  _sel_extend(cur, obj, en);
               }
             else
               {
                  en->have_selection = EINA_FALSE;
                  en->selecting = EINA_FALSE;
                  _sel_clear(obj, en);
                  tc = efl_canvas_text_cursor_create(obj);
                  efl_text_cursor_copy(cur, tc);
                  efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_START);
                  _sel_init(cur, obj, en);
                  efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_END);
                  _sel_extend(cur, obj, en);
               }
             goto end;
          }
        else if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
          {
             if (shift)
               {
                  tc = efl_canvas_text_cursor_create(obj);
                  efl_text_cursor_copy(cur, tc);
                  if (efl_text_cursor_compare(cur, en->sel_start) < 0)
                    efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START);
                  else
                    {
                       efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END);
                       efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHAR_NEXT);
                    }
                  _sel_extend(cur, obj, en);
               }
             else
               {
                  en->have_selection = EINA_FALSE;
                  en->selecting = EINA_FALSE;
                  _sel_clear(obj, en);
                  tc = efl_canvas_text_cursor_create(obj);
                  efl_text_cursor_copy(cur, tc);
                  efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START);
                  _sel_init(cur, obj, en);
                  efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END);
                  efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHAR_NEXT);
                  _sel_extend(cur, obj, en);
               }
             goto end;
          }
     }
   _cursor_char_coord_set(obj, cur, ev->canvas.x, ev->canvas.y, &cx, &cy);

   if (dosel)
     {
        if ((en->have_selection) && (shift))
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

   if (ev->button == 2)
     {
     }
end:
   (void) 0;
}

static void
_mouse_up_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Coord cx, cy;
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl_Text_Cursor *cur = efl_text_interactive_main_cursor_get(obj);
   Evas_Event_Mouse_Up *ev = event_info;

   if ((!ev) || (ev->button != 1)) return;

   /* We don't check for ON_HOLD because we'd like to end selection anyway when
    * mouse is up, even if it's held. */

#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        Ecore_IMF_Event_Mouse_Up ecore_ev;
//        ecore_imf_evas_event_mouse_up_wrap(ev, &ecore_ev);
        if (ecore_imf_context_filter_event(en->imf_context,
                                           ECORE_IMF_EVENT_MOUSE_UP,
                                           (Ecore_IMF_Event *)&ecore_ev))
          return;
     }
#endif

   _cursor_char_coord_set(obj, cur, ev->canvas.x, ev->canvas.y, &cx, &cy);

   if (en->select_allow)
     {
        efl_text_cursor_copy(en->sel_end, cur);
     }
   if (en->selecting)
     {
        if (en->have_selection)
          en->had_sel = EINA_TRUE;
        en->selecting = EINA_FALSE;
     }

   _entry_imf_cursor_info_set(obj, cur, en);
}

static void
_mouse_move_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Coord cx, cy;
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl_Text_Cursor *cur = efl_text_interactive_main_cursor_get(obj);
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord x, y, w, h;
   Efl_Text_Cursor *tc;
   Eina_Bool multiline;

   multiline = efl_text_multiline_get(obj);

#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        Ecore_IMF_Event_Mouse_Move ecore_ev;
//        ecore_imf_evas_event_mouse_move_wrap(ev, &ecore_ev);
        if (ecore_imf_context_filter_event(en->imf_context,
                                           ECORE_IMF_EVENT_MOUSE_MOVE,
                                           (Ecore_IMF_Event *)&ecore_ev))
          return;
     }
#endif

   if (en->selecting)
     {
        tc = efl_canvas_text_cursor_create(obj);
        efl_text_cursor_copy(cur, tc);
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        cx = ev->cur.canvas.x - x;
        cy = ev->cur.canvas.y - y;

        if (multiline)
          {
             efl_text_cursor_char_coord_set(cur, EINA_POSITION2D(cx, cy));
          }
        else
          {
             Evas_Coord lx, ly, lw, lh;
             efl_text_cursor_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_PARAGRAPH_FIRST);
             evas_textblock_cursor_line_geometry_get(efl_text_cursor_handle_get(cur), &lx, &ly, &lw, &lh);
             efl_text_cursor_char_coord_set(cur, EINA_POSITION2D(cx, ly + (lh / 2)));
          }

        if (en->select_allow)
          {
             _sel_extend(cur, obj, en);

             if (!efl_text_cursor_equal(en->sel_start, en->sel_end))
               _sel_enable(cur, obj, en);
          }
        efl_del(tc);
     }
}

EOLIAN static Efl_Text_Cursor *
_efl_ui_internal_text_interactive_efl_text_interactive_main_cursor_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *pd)
{
   return pd->main_cursor;
}

EOLIAN static Efl_Object *
_efl_ui_internal_text_interactive_efl_object_constructor(Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   en->select_allow = EINA_TRUE;
   en->editable = EINA_TRUE;
   return obj;
}

EOLIAN static Efl_Object *
_efl_ui_internal_text_interactive_efl_object_finalize(Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   evas_object_event_callback_add(obj, EVAS_CALLBACK_FOCUS_IN, _focus_in_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_FOCUS_OUT, _focus_out_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_DOWN, _key_down_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, NULL);

   en->main_cursor = efl_canvas_text_cursor_create(obj);
   en->sel_start = efl_canvas_text_cursor_create(obj);
   en->sel_end = efl_canvas_text_cursor_create(obj);

   efl_event_callback_add(efl_text_interactive_main_cursor_get(obj), EFL_TEXT_CURSOR_EVENT_CHANGED,
         _sel_cursor_changed, obj);

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

        if (efl_text_multiline_get(obj))
           ecore_imf_context_input_hint_set(en->imf_context,
                 ecore_imf_context_input_hint_get(en->imf_context) | ECORE_IMF_INPUT_HINT_MULTILINE);
     }
#endif

done:
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_internal_text_interactive_efl_text_interactive_selection_allowed_set(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *pd, Eina_Bool allowed)
{
   pd->select_allow = allowed;
}

EOLIAN static Eina_Bool
_efl_ui_internal_text_interactive_efl_text_interactive_selection_allowed_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *pd)
{
   return pd->select_allow;
}

EOLIAN static void
_efl_ui_internal_text_interactive_efl_text_interactive_selection_cursors_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *pd, Efl_Text_Cursor **start, Efl_Text_Cursor **end)
{
   if (efl_text_cursor_position_get(pd->sel_start) >
         efl_text_cursor_position_get(pd->sel_end))
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
_efl_ui_internal_text_interactive_efl_text_interactive_editable_set(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *sd, Eina_Bool editable)
{
   sd->editable = editable;
}

EOLIAN static Eina_Bool
_efl_ui_internal_text_interactive_efl_text_interactive_editable_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *sd)
{
   return sd->editable;
}

#include "efl_ui_internal_text_interactive.eo.c"
#include "efl_text_interactive.eo.c"
