#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "elm_priv.h"
#include "efl_ui_internal_text_interactive.h"
#include "efl_canvas_textblock_internal.h"

#define MY_CLASS EFL_UI_INTERNAL_TEXT_INTERACTIVE_CLASS

#define _PARAGRAPH_SEPARATOR_UTF8 "\xE2\x80\xA9"


typedef struct _Efl_Ui_Internal_Text_Interactive_Data
{
   Efl_Text_Cursor_Object                       *sel_start, *sel_end;
   Eina_Bool                              watch_selection;
   Efl_Text_Cursor_Object                       *main_cursor;
   Efl_Text_Cursor_Object                       *preedit_start, *preedit_end;
   Ecore_Timer                           *pw_timer;
   Eina_List                             *seq;
   char                                  *selection;
   const char                            *file;
   Elm_Text_Format                        format;
   Eina_Bool                              composing : 1;
   Eina_Bool                              selecting : 1;
   Eina_Bool                              have_selection : 1;
   Eina_Bool                              select_allow : 1;
   Eina_Bool                              editable : 1;
   Eina_Bool                              had_sel : 1;
   Eina_Bool                              auto_save : 1;
   Eina_Bool                              prediction_allow : 1;
   Eina_Bool                              anchors_updated : 1;
   Eina_Bool                              auto_return_key : 1;
   int                                    input_panel_layout_variation;
   Efl_Input_Text_Panel_Layout_Type       input_panel_layout;
   Efl_Input_Text_Capitalize_Type         autocapital_type;
   Efl_Input_Text_Panel_Language_Type     input_panel_lang;
   Efl_Input_Text_Panel_Return_Key_Type   input_panel_return_key_type;
   Efl_Input_Text_Content_Type            input_hints;
   Efl_Input_Text_Panel_Return_Key_State  input_panel_return_key_state;

#ifdef HAVE_ECORE_IMF
   Eina_Bool              have_preedit : 1;
   Eina_Bool              commit_cancel : 1; // For skipping useless commit
   Ecore_IMF_Context     *imf_context;
#endif
} Efl_Ui_Internal_Text_Interactive_Data;

static void _sel_range_del_emit(Evas_Object *obj, Efl_Ui_Internal_Text_Interactive_Data *en);
static void _sel_init(Efl_Text_Cursor_Object *c, Efl_Ui_Internal_Text_Interactive_Data *en);
static void _sel_enable(Evas_Object *o,Efl_Ui_Internal_Text_Interactive_Data *en);
static void _sel_extend(Efl_Text_Cursor_Object *c, Evas_Object *o, Efl_Ui_Internal_Text_Interactive_Data *en);
static void _sel_clear(Evas_Object *o EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en);
static void _emit_sel_state( Eo *o, Efl_Ui_Internal_Text_Interactive_Data *en);
static const char *_entry_selection_get(Efl_Ui_Internal_Text_Interactive *obj, Efl_Ui_Internal_Text_Interactive_Data *en);
static void _entry_imf_cursor_info_set(Efl_Ui_Internal_Text_Interactive_Data *en);

static void _sel_watch_freeze(Efl_Ui_Internal_Text_Interactive_Data *en);
static void _sel_watch_thaw(Efl_Ui_Internal_Text_Interactive_Data *en);

static void
_text_filter_format_prepend(Efl_Canvas_Textblock *obj, Efl_Ui_Internal_Text_Interactive_Data *en,
                            Efl_Text_Cursor_Object *c, const char *text);

static Efl_Text_Change_Info *
_text_filter_text_prepend(Efl_Canvas_Textblock *obj, Efl_Ui_Internal_Text_Interactive_Data *en,
                          Efl_Text_Cursor_Object *c,
                          const char *text,
                          const char *fmtpre, const char *fmtpost,
                          Eina_Bool clearsel, Eina_Bool changeinfo);

static Efl_Text_Change_Info *
_text_filter_markup_prepend_internal(Efl_Canvas_Textblock *obj, Efl_Ui_Internal_Text_Interactive_Data *en,
                                     Efl_Text_Cursor_Object *c,
                                     char *text,
                                     const char *fmtpre, const char *fmtpost,
                                     Eina_Bool clearsel, Eina_Bool changeinfo);

static Efl_Text_Change_Info *
_text_filter_markup_prepend(Efl_Canvas_Textblock *obj, Efl_Ui_Internal_Text_Interactive_Data *en,
                            Efl_Text_Cursor_Object *c,
                            const char *text,
                            const char *fmtpre, const char *fmtpost,
                            Eina_Bool clearsel, Eina_Bool changeinfo);

static void
_cur_pos_copy(Efl_Text_Cursor_Object *src, Efl_Text_Cursor_Object *dest)
{
   efl_text_cursor_object_position_set(dest, efl_text_cursor_object_position_get(src));
}

static void
_sel_watch_freeze(Efl_Ui_Internal_Text_Interactive_Data *en)
{
   en->watch_selection = EINA_FALSE;
}

static void
_sel_watch_thaw(Efl_Ui_Internal_Text_Interactive_Data *en)
{
   en->watch_selection = EINA_TRUE;
}

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
   if (efl_text_cursor_object_equal(en->preedit_start, en->preedit_end)) return;

   /* delete the preedit characters */
   efl_text_cursor_object_range_delete(en->preedit_start, en->preedit_end);
}

static Eina_Bool
_entry_imf_retrieve_surrounding_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, char **text, int *cursor_pos)
{
   Efl_Canvas_Textblock *obj = data;
   Efl_Text_Cursor_Object *cur = efl_text_interactive_main_cursor_get(obj);
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
          *cursor_pos = efl_text_cursor_object_position_get(cur);
        else
          *cursor_pos = 0;
     }

   return EINA_TRUE;
}

static void
_return_key_update(Evas_Object *obj)
{
#ifdef HAVE_ECORE_IMF
   Eina_Bool return_key_disabled = EINA_FALSE;
   Efl_Ui_Internal_Text_Interactive_Data *sd = efl_data_scope_get(obj, MY_CLASS);

   if (sd->input_panel_return_key_state != EFL_INPUT_TEXT_PANEL_RETURN_KEY_STATE_AUTO) return;

   if (efl_canvas_textblock_is_empty_get(obj) == EINA_TRUE)
     return_key_disabled = EINA_TRUE;

   if (sd->imf_context)
     ecore_imf_context_input_panel_return_key_disabled_set(sd->imf_context, return_key_disabled);
#else
   (void)obj;
#endif
}

Eina_Bool
_entry_hide_visible_password(Eo *obj)
{
   Eina_Bool b_ret = EINA_FALSE;
   const Evas_Object_Textblock_Node_Format *node, *node_next;
   node = evas_textblock_node_format_first_get(obj);

   if (!node) return EINA_FALSE;

   do
     {
        node_next = evas_textblock_node_format_next_get(node);
        const char *text = evas_textblock_node_format_text_get(node);
        if (text)
          {
             if (!strcmp(text, "+ password=off"))
               {
                  evas_textblock_node_format_remove_pair(obj, (Evas_Object_Textblock_Node_Format *)node);
                  b_ret = EINA_TRUE;
               }
          }
        node = node_next;
     } while (node);

   return b_ret;
}

static void
_entry_imf_event_commit_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info)
{
   Efl_Canvas_Textblock *obj = data;
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   char *commit_str = event_info;
   Efl_Text_Change_Info *info = NULL;

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

   if (efl_text_password_get(obj) && (!en->preedit_start))
     {
        info = _text_filter_text_prepend(obj, en, en->main_cursor, commit_str,
                                         "+ password=off", "- password",
                                         EINA_TRUE, EINA_TRUE);
     }
   else
     {
        info = _text_filter_text_prepend(obj, en, en->main_cursor, commit_str,
                                         NULL, NULL,
                                         EINA_TRUE, EINA_TRUE);
     }

   _entry_imf_cursor_info_set(en);
   if (info)
     {
        efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_CHANGED_USER, info);
        eina_stringshare_del(info->content);
        free(info);
        info = NULL;
     }
   _entry_imf_cursor_info_set(en);
}

static Efl_Text_Change_Info *
_text_filter_markup_prepend_internal(Efl_Canvas_Textblock *obj, Efl_Ui_Internal_Text_Interactive_Data *en,
                                     Efl_Text_Cursor_Object *c,
                                     char *text,
                                     const char *fmtpre, const char *fmtpost,
                                     Eina_Bool clearsel, Eina_Bool changeinfo)
{
   Eina_Bool have_sel = EINA_FALSE;

   if ((clearsel) && (en->have_selection))
     {
        _sel_range_del_emit(obj, en);
        have_sel = EINA_TRUE;
     }

#ifdef HAVE_ECORE_IMF
   // For skipping useless commit
   if (en->have_preedit && (!text || !strcmp(text, "")))
     en->commit_cancel = EINA_TRUE;
   else
     en->commit_cancel = EINA_FALSE;
#endif
   if (text)
     {
        Efl_Text_Change_Info *info = NULL;

        if (changeinfo)
          {
             info = calloc(1, sizeof(*info));
             info->type = EFL_TEXT_CHANGE_TYPE_INSERT;
             info->content = eina_stringshare_add(text);
             info->length =
             eina_unicode_utf8_get_len(info->content);
          }
        if (info)
          {
             if (have_sel)
               {
                  info->mergeable = EINA_TRUE;
               }
             info->position =
                efl_text_cursor_object_position_get(efl_text_interactive_main_cursor_get(obj));
          }
        if (fmtpre) _text_filter_format_prepend(obj, en, efl_text_interactive_main_cursor_get(obj), fmtpre);
        //evas_object_textblock_text_markup_prepend(c, text);
        efl_text_cursor_object_text_insert(c, text);
        free(text);
        if (fmtpost) _text_filter_format_prepend(obj, en, efl_text_interactive_main_cursor_get(obj), fmtpost);
        return info;
     }
   return NULL;
}

static Efl_Text_Change_Info *
_text_filter_markup_prepend(Efl_Canvas_Textblock *obj, Efl_Ui_Internal_Text_Interactive_Data *en,
                            Efl_Text_Cursor_Object *c,
                            const char *text,
                            const char *fmtpre, const char *fmtpost,
                            Eina_Bool clearsel, Eina_Bool changeinfo)
{
   char *text2;

   EINA_SAFETY_ON_NULL_RETURN_VAL(text, NULL);

   if ((clearsel) && (en->have_selection))
     {
        _sel_range_del_emit(obj, en);
     }

   text2 = strdup(text);
   if (text2)
     {
        Efl_Text_Change_Info *info;

        info = _text_filter_markup_prepend_internal(obj, en, c, text2,
                                                    fmtpre, fmtpost,
                                                    clearsel, changeinfo);
        return info;
     }
   return NULL;
}

static Efl_Text_Change_Info *
_text_filter_text_prepend(Efl_Canvas_Textblock *obj, Efl_Ui_Internal_Text_Interactive_Data *en,
                          Efl_Text_Cursor_Object *c,
                          const char *text,
                          const char *fmtpre, const char *fmtpost,
                          Eina_Bool clearsel, Eina_Bool changeinfo)
{
   char *markup_text;
   Efl_Text_Change_Info *info = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(text, NULL);

   if ((clearsel) && (en->have_selection))
     {
        _sel_range_del_emit(obj, en);
     }

   markup_text = evas_textblock_text_utf8_to_markup(NULL, text);
   if (markup_text)
     info = _text_filter_markup_prepend_internal(obj, en, c, markup_text,
                                                 fmtpre, fmtpost,
                                                 clearsel, changeinfo);
   return info;
}

static void
_text_filter_format_prepend(Efl_Canvas_Textblock *obj, Efl_Ui_Internal_Text_Interactive_Data *en,
                            Efl_Text_Cursor_Object *c, const char *text)
{
   EINA_SAFETY_ON_NULL_RETURN(text);

   if (text)
     {
        const char *s;
        char *markup_text;
        size_t size;

        s = text;
        if (*s == '+')
          {
             s++;
             while (*s == ' ')
               s++;
             if (!*s)
               {
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
        if (markup_text)
          _text_filter_markup_prepend_internal(obj, en, c, markup_text,
                                               NULL, NULL,
                                               EINA_FALSE, EINA_FALSE);
     }
}

static void
_entry_imf_event_preedit_changed_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Canvas_Textblock *obj = data;
   Efl_Text_Cursor_Object *cur = efl_text_interactive_main_cursor_get(obj);
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl_Text_Change_Info *info = NULL;
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

   preedit_start_pos = efl_text_cursor_object_position_get(cur);

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

        if (efl_text_password_get(obj))
          {
             _entry_hide_visible_password(obj);
             info = _text_filter_markup_prepend(obj, en, cur,
                                              eina_strbuf_string_get(buf),
                                              "+ password=off",
                                              "- password",
                                              EINA_TRUE, EINA_TRUE);
          }
        else
          _text_filter_markup_prepend(obj, en, cur,
                                      eina_strbuf_string_get(buf),
                                      NULL, NULL,
                                      EINA_TRUE, EINA_FALSE);

        eina_strbuf_free(buf);
     }

   if (!preedit_end_state)
     {
        /* set preedit start cursor */
        if (!en->preedit_start)
          en->preedit_start = efl_canvas_textblock_cursor_create(obj);
        _cur_pos_copy(cur, en->preedit_start);

        /* set preedit end cursor */
        if (!en->preedit_end)
          en->preedit_end = efl_canvas_textblock_cursor_create(obj);
        _cur_pos_copy(cur, en->preedit_end);

        preedit_end_pos = efl_text_cursor_object_position_get(cur);

        for (i = 0; i < (preedit_end_pos - preedit_start_pos); i++)
          {
             efl_text_cursor_object_move(en->preedit_start, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_PREVIOUS);
          }

        en->have_preedit = EINA_TRUE;

        /* set cursor position */
        efl_text_cursor_object_position_set(cur, preedit_start_pos + cursor_pos);
     }

   if (info)
     {
        _entry_imf_cursor_info_set(en);
        efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_PREEDIT_CHANGED, info);
        eina_stringshare_del(info->content);
        free(info);
        info = NULL;
     }

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
   Efl_Canvas_Textblock *obj = data;
   Efl_Text_Cursor_Object *cur = efl_text_interactive_main_cursor_get(obj);
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Ecore_IMF_Event_Delete_Surrounding *ev = event_info;
   Efl_Text_Cursor_Object *del_start, *del_end;
   Efl_Text_Change_Info info = { NULL, 0, 0, 0, 0 };
   int cursor_pos;
   int start, end;
   char *tmp;

   cursor_pos = efl_text_cursor_object_position_get(cur);

   del_start = efl_canvas_textblock_cursor_create(obj);
   efl_text_cursor_object_position_set(del_start, cursor_pos + ev->offset);

   del_end = efl_canvas_textblock_cursor_create(obj);
   efl_text_cursor_object_position_set(del_end, cursor_pos + ev->offset + ev->n_chars);

   start = efl_text_cursor_object_position_get(del_start);
   end = efl_text_cursor_object_position_get(del_end);
   if (start == end) goto end;

   tmp = efl_text_cursor_object_range_text_get(del_start, del_end);

   info.type = EFL_TEXT_CHANGE_TYPE_REMOVE;
   info.position = start;
   info.length = end - start;
   info.content = tmp;

   efl_text_cursor_object_range_delete(del_start, del_end);

   efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_CHANGED_USER, &info);
   free(tmp);

   _entry_imf_cursor_info_set(en);

end:
   efl_del(del_start);
   efl_del(del_end);
}

static void
_entry_imf_event_selection_set_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info)
{
   Efl_Canvas_Textblock *obj = data;
   Efl_Text_Cursor_Object *cur = efl_text_interactive_main_cursor_get(obj);
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Ecore_IMF_Event_Selection *ev = event_info;

   if (ev->start == ev->end)
     {
        efl_text_cursor_object_position_set(cur, ev->start);
     }
   else
     {
        _sel_clear(obj, en);
        efl_text_cursor_object_position_set(cur, ev->start);
        _sel_enable(obj, en);
        _sel_init(cur, en);
        efl_text_cursor_object_position_set(cur, ev->end);
        _sel_extend(cur, obj, en);
     }
}

static Eina_Bool
_entry_imf_retrieve_selection_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, char **text)
{
   Efl_Canvas_Textblock *obj = data;
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
_entry_imf_cursor_location_set(Efl_Ui_Internal_Text_Interactive_Data *en)
{
#ifdef HAVE_ECORE_IMF
   Eina_Rect rect;
   if (!en->imf_context) return;

   rect = efl_text_cursor_object_cursor_geometry_get(en->main_cursor ,EFL_TEXT_CURSOR_TYPE_BEFORE);
   ecore_imf_context_cursor_location_set(en->imf_context, rect.x, rect.y, rect.w, rect.h);
   // FIXME: ecore_imf_context_bidi_direction_set(en->imf_context, (Ecore_IMF_BiDi_Direction)dir);
#else
   (void)en;
#endif
}

static void
_entry_imf_cursor_info_set(Efl_Ui_Internal_Text_Interactive_Data *en)
{
   int cursor_pos;

#ifdef HAVE_ECORE_IMF
   if (!en->imf_context) return;

   if (en->have_selection)
     {
        if (efl_text_cursor_object_compare(en->sel_start, en->sel_end) < 0)
          cursor_pos = efl_text_cursor_object_position_get(en->sel_start);
        else
          cursor_pos = efl_text_cursor_object_position_get(en->sel_end);
     }
   else
     cursor_pos = efl_text_cursor_object_position_get(en->main_cursor);

   ecore_imf_context_cursor_position_set(en->imf_context, cursor_pos);

   _entry_imf_cursor_location_set(en);
#else
   (void)en;
#endif
}

static void
_focus_in_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
#ifdef HAVE_ECORE_IMF
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);

   if (!en->imf_context) return;

   ecore_imf_context_focus_in(en->imf_context);
   _entry_imf_cursor_info_set(en);

   _return_key_update(obj);
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
     en->selection = efl_text_cursor_object_range_text_get(en->sel_start, en->sel_end);
   return en->selection;
}

static void
_sel_reset(Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *en){

   if (!en->watch_selection)
     return;

   if (!en->have_selection && efl_text_cursor_object_equal(en->sel_start, en->sel_end))
     return;

   if (en->have_selection)
     {
        if (efl_text_cursor_object_equal(en->sel_start, en->sel_end))
          {
             _sel_clear(obj, en);
          }
        else
          {
             _entry_imf_cursor_info_set(en);
             if (en->selection)
               {
                  free(en->selection);
                  en->selection = NULL;
               }
             _emit_sel_state(obj, en);
          }
     }
   else
     {
        if (!efl_text_cursor_object_equal(en->sel_start, en->sel_end))
          {
             _sel_enable(obj, en);
             _entry_imf_cursor_info_set(en);
             if (en->selection)
               {
                  free(en->selection);
                  en->selection = NULL;
               }
             _emit_sel_state(obj, en);
          }
     }
}

static void
_sel_cursor_changed(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Internal_Text_Interactive *obj = data;
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);

   if (!efl_text_interactive_selection_allowed_get(obj))
     return;

   _sel_reset(obj, en);
}

static void
_sel_init(Efl_Text_Cursor_Object *c, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   if (en->have_selection)
      return;

   _sel_watch_freeze(en);
   _cur_pos_copy(c, en->sel_start);
   _cur_pos_copy(c, en->sel_end);
   _sel_watch_thaw(en);

   en->have_selection = EINA_FALSE;
   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }
}

static void
_sel_enable(Eo *o, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   if (en->have_selection) return;
   en->have_selection = EINA_TRUE;
   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }

   Eina_Bool b_value = EINA_TRUE;
   efl_event_callback_call(o, EFL_TEXT_INTERACTIVE_EVENT_HAVE_SELECTION_CHANGED, &b_value);
   _entry_imf_context_reset(en);
}

static void
_emit_sel_state( Eo *o, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   if (!efl_text_cursor_object_compare(en->sel_start, en->sel_end))
     {
        _sel_clear(o, en);
     }
   else
     {
        Eina_Range range = eina_range_from_to(efl_text_cursor_object_position_get(en->sel_start),
                                              efl_text_cursor_object_position_get(en->sel_end));
        efl_event_callback_call(o, EFL_TEXT_INTERACTIVE_EVENT_SELECTION_CHANGED, &range);
     }
}

static void
_sel_extend(Efl_Text_Cursor_Object *c, Evas_Object *o, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   _sel_enable(o, en);
   if (efl_text_cursor_object_equal(c, en->sel_end)) return;

   _sel_watch_freeze(en);
   _cur_pos_copy(c, en->sel_end);
   _sel_watch_thaw(en);

   _entry_imf_cursor_info_set(en);

   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }

   _emit_sel_state(o, en);
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
        Eina_Bool b_value = en->have_selection;
        _sel_watch_freeze(en);
        _cur_pos_copy(en->sel_start, en->sel_end);
        _sel_watch_thaw(en);
        efl_event_callback_call(o, EFL_TEXT_INTERACTIVE_EVENT_HAVE_SELECTION_CHANGED, &b_value);
     }
}

EOLIAN static void
_efl_ui_internal_text_interactive_efl_text_interactive_all_unselect(
      Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   _sel_clear(obj, en);
}

EOLIAN static Eina_Bool
_efl_ui_internal_text_interactive_efl_text_interactive_have_selection_get(
         const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   if (!en->have_selection) return en->have_selection;
   return !efl_text_cursor_object_equal(en->sel_start, en->sel_end);
}


EOLIAN static void
_efl_ui_internal_text_interactive_efl_text_interactive_all_select(
      Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *en EINA_UNUSED)
{
   if (!efl_text_interactive_selection_allowed_get(obj))
     return;

   Eo *c1 = efl_canvas_textblock_cursor_create(obj);
   Eo *c2 = efl_canvas_textblock_cursor_create(obj);

   efl_text_cursor_object_move(c1, EFL_TEXT_CURSOR_MOVE_TYPE_FIRST);
   efl_text_cursor_object_move(c2, EFL_TEXT_CURSOR_MOVE_TYPE_LAST);

   efl_text_interactive_selection_cursors_set(obj, c1, c2);

   efl_del(c1);
   efl_del(c2);
}


static void
_range_del_emit(Evas_Object *obj, Efl_Text_Cursor_Object *cur1, Efl_Text_Cursor_Object *cur2)
{
   size_t start, end;
   char *tmp;
   Efl_Text_Change_Info info = { NULL, 0, 0, 0, 0 };

   start = efl_text_cursor_object_position_get(cur1);
   end = efl_text_cursor_object_position_get(cur2);
   if (start == end)
      return;

   info.type = EFL_TEXT_CHANGE_TYPE_REMOVE;
   info.position = start;
   info.length = end - start;

   tmp = efl_text_cursor_object_range_text_get(cur1, cur2);
   info.content = tmp;

   efl_text_cursor_object_range_delete(cur1, cur2);

   efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_CHANGED_USER, &info);
   if (tmp) free(tmp);
}

static void
_sel_range_del_emit(Evas_Object *obj, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   _range_del_emit(obj, en->sel_start, en->sel_end);
   _sel_clear(obj, en);
}

static void
_delete_emit(Eo *obj, Efl_Text_Cursor_Object *c, Efl_Ui_Internal_Text_Interactive_Data *en EINA_UNUSED, size_t pos,
             Eina_Bool backspace)
{
   Eo * cur = efl_duplicate(c);
   if (backspace)
     {
        if (!efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_PREVIOUS))
          {
             return;
          }
        efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT);
     }
   else
     {
        if (!efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT))
          {
             return;
          }
        efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_PREVIOUS);
     }
   efl_del(cur);
   cur = NULL;

   Efl_Text_Change_Info info = { NULL, 0, 0, 0, 0 };
   char *tmp = NULL;
   info.type = EFL_TEXT_CHANGE_TYPE_REMOVE;
   if (backspace)
     {

        Evas_Textblock_Cursor *cc = evas_object_textblock_cursor_new(obj);
        evas_textblock_cursor_copy(efl_text_cursor_object_handle_get(c), cc);
        Eina_Bool remove_cluster = evas_textblock_cursor_at_cluster_as_single_glyph(cc,EINA_FALSE);
        if (remove_cluster)
          {
             evas_textblock_cursor_cluster_prev(cc);
          }
        else
          {
             evas_textblock_cursor_char_prev(cc);
          }

        info.position = evas_textblock_cursor_pos_get(cc);
        info.length = pos -info.position;

        tmp = evas_textblock_cursor_range_text_get(efl_text_cursor_object_handle_get(c), cc, EVAS_TEXTBLOCK_TEXT_MARKUP);
        evas_textblock_cursor_range_delete(efl_text_cursor_object_handle_get(c), cc);
        evas_textblock_cursor_free(cc);
     }
   else
     {
        Evas_Textblock_Cursor *cc = evas_object_textblock_cursor_new(obj);
        evas_textblock_cursor_copy(efl_text_cursor_object_handle_get(c), cc);

        Eina_Bool remove_cluster = evas_textblock_cursor_at_cluster_as_single_glyph(cc,EINA_TRUE);
        if (remove_cluster)
          {
             evas_textblock_cursor_cluster_next(cc);
          }
        else
          {
             evas_textblock_cursor_char_next(cc);
          }

        info.position = pos;
        info.length = evas_textblock_cursor_pos_get(cc) - info.position;

        tmp = evas_textblock_cursor_range_text_get(efl_text_cursor_object_handle_get(c), cc, EVAS_TEXTBLOCK_TEXT_MARKUP);
        evas_textblock_cursor_range_delete(efl_text_cursor_object_handle_get(c), cc);
        evas_textblock_cursor_free(cc);
     }

   info.type = EFL_TEXT_CHANGE_TYPE_REMOVE;
   info.position = pos;
   info.length = 1;
   info.content = tmp;

   efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_CHANGED_USER, &info);
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
_key_down_sel_pre(Efl_Ui_Internal_Text_Interactive *obj, Efl_Text_Cursor_Object *cur, Efl_Ui_Internal_Text_Interactive_Data *en, Eina_Bool shift, Eina_Bool movement_forward)
{
   if (en->select_allow)
     {
        if (shift)
          {
             _sel_init(cur, en);
          }
        else if (en->have_selection)
          {
             Eina_Bool sel_forward = efl_text_cursor_object_compare(en->sel_start, en->sel_end);
             if ((sel_forward && movement_forward) || (!sel_forward && !movement_forward))
                _cur_pos_copy(en->sel_end, cur);
             else
               _cur_pos_copy(en->sel_start, cur);

             _sel_clear(obj, en);
          }
     }
}

static void
_key_down_sel_post(Efl_Ui_Internal_Text_Interactive *obj, Efl_Text_Cursor_Object *cur, Efl_Ui_Internal_Text_Interactive_Data *en, Eina_Bool shift)
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
   Efl_Text_Cursor_Object *cur;
   Eina_Bool control, alt, shift;
#if defined(__APPLE__) && defined(__MACH__)
   Eina_Bool super, altgr;
#endif
   Eina_Bool multiline;
   int old_cur_pos;
   char *string = (char *)ev->string;
   Eina_Bool free_string = EINA_FALSE;
   Eina_Bool changed_user = EINA_FALSE;
   Efl_Text_Change_Info info = { NULL, 0, 0, 0, 0 };

   if (!ev->key) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);

   /* FIXME: Maybe allow selections to happen even when not editable. */
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

   cur = efl_text_interactive_main_cursor_get(obj);
   old_cur_pos = efl_text_cursor_object_position_get(cur);
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
             if (shift || efl_canvas_textblock_newline_as_paragraph_separator_get(obj))
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

             if (efl_text_interactive_have_selection_get(obj))
               ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             if (efl_text_cursor_object_line_jump_by(cur, -1))
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

             if (efl_text_interactive_have_selection_get(obj))
               ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             if (efl_text_cursor_object_line_jump_by(cur, 1))
               ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

             _key_down_sel_post(obj, cur, en, shift);
          }
     }
   else if (!strcmp(ev->key, "Left") ||
            (!strcmp(ev->key, "KP_Left") && !ev->string))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_FALSE);

#if defined(__APPLE__) && defined(__MACH__)
        if (altgr) efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START);
#else
        /* If control is pressed, go to the start of the word */
        if (control) efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START);
#endif
        if (efl_text_interactive_have_selection_get(obj))
          ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        if (efl_text_cursor_object_move(cur,EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_PREVIOUS))
          ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        _key_down_sel_post(obj, cur, en, shift);
     }
   else if (!strcmp(ev->key, "Right") ||
            (!strcmp(ev->key, "KP_Right") && !ev->string))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_TRUE);

#if defined(__APPLE__) && defined(__MACH__)
        if (altgr) efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END);
#else
        /* If control is pressed, go to the end of the word */
        if (control) efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END);
#endif
        if (efl_text_interactive_have_selection_get(obj))
          ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        if (efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT))
          ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        _key_down_sel_post(obj, cur, en, shift);
     }
   else if (!strcmp(ev->key, "BackSpace"))
     {
        _compose_seq_reset(en);
        if (control && !en->have_selection)
          {
             // del to start of previous word
             Efl_Text_Cursor_Object *tc = efl_canvas_textblock_cursor_create(obj);
             _cur_pos_copy(cur, tc);
             efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_PREVIOUS);
             efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START);

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
                  _delete_emit(obj, cur, en, old_cur_pos, EINA_TRUE);
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
             Efl_Text_Cursor_Object *tc = efl_canvas_textblock_cursor_create(obj);

             _cur_pos_copy(cur, tc);
             efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END);
             efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT);

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
           efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_FIRST);
        else
           efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_START);

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
           efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LAST);
        else
           efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_END);

        _key_down_sel_post(obj, cur, en, shift);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
#if defined(__APPLE__) && defined(__MACH__)
   else if ((super) && (!strcmp(ev->key, "a")))
#else
   else if ((control) && (!strcmp(ev->key, "a")))
#endif
     {
        _compose_seq_reset(en);
        if (shift)
          {
             efl_text_interactive_all_unselect(obj);
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
          }
        else
          {
             efl_text_interactive_all_select(obj);
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
          }
     }
#if defined(__APPLE__) && defined(__MACH__)
   else if ((super) && (!strcmp(ev->key, "z")))
#else
   else if ((control) && (!strcmp(ev->key, "z")))
#endif
     {
        _compose_seq_reset(en);
        if (shift)
          {
             // redo
             efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_REDO_REQUEST, NULL);
          }
        else
          {
             // undo
             efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_UNDO_REQUEST, NULL);
          }
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
#if defined(__APPLE__) && defined(__MACH__)
   else if ((super) && (!shift) && (!strcmp(ev->key, "y")))
#else
   else if ((control) && (!shift) && (!strcmp(ev->key, "y")))
#endif
     {
        _compose_seq_reset(en);
        // redo
        efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_REDO_REQUEST, NULL);
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

        efl_text_cursor_object_line_jump_by(cur, -10);

        _key_down_sel_post(obj, cur, en, shift);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Next") ||
            (!strcmp(ev->key, "KP_Next") && !ev->string))
     {
        _compose_seq_reset(en);
        _key_down_sel_pre(obj, cur, en, shift, EINA_TRUE);

        efl_text_cursor_object_line_jump_by(cur, 10);

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
                  info.mergeable = EINA_TRUE;
               }
             info.type = EFL_TEXT_CHANGE_TYPE_INSERT;
             info.content = string;
             info.position = efl_text_cursor_object_position_get(cur);
             info.length = eina_unicode_utf8_get_len(string);

             efl_text_cursor_object_text_insert(cur, string);
             changed_user = EINA_TRUE;

             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

             if (free_string) free(string);
          }
     }
end:
   if (changed_user)
     {
        efl_event_callback_call(obj, EFL_TEXT_INTERACTIVE_EVENT_CHANGED_USER, &info);
     }
   (void) 0;
}

static void
_cursor_char_coord_set(Efl_Canvas_Textblock *obj, Efl_Text_Cursor_Object *cur, Evas_Coord canvasx, Evas_Coord canvasy, Evas_Coord *_cx, Evas_Coord *_cy)
{
   Evas_Coord cx, cy;
   Evas_Coord x, y, lh = 0, cly = 0;
   Efl_Text_Cursor_Object *line_cur;
   Efl_Text_Cursor_Object *tc;

   tc = efl_canvas_textblock_cursor_create(obj);
   _cur_pos_copy(cur, tc);
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   cx = canvasx - x;
   cy = canvasy - y;

   line_cur = efl_canvas_textblock_cursor_create(obj);
   efl_text_cursor_object_move(line_cur, EFL_TEXT_CURSOR_MOVE_TYPE_LAST);
   evas_textblock_cursor_line_geometry_get(efl_text_cursor_object_handle_get(line_cur), NULL, &cly, NULL, &lh);
   /* Consider a threshold of half the line height */
   if (cy > (cly + lh) && cy < (cly + lh + lh / 2))
     {
        cy = cly + lh - 1; // Make it inside Textblock
     }
   efl_text_cursor_object_move(line_cur, EFL_TEXT_CURSOR_MOVE_TYPE_FIRST);
   evas_textblock_cursor_line_geometry_get(efl_text_cursor_object_handle_get(line_cur), NULL, &cly, NULL, NULL);

   if (cy < cly && cy > (cly - lh / 2))
     {
        cy = cly;
     }
   efl_del(line_cur);
   /* No need to check return value if not able to set the char coord Textblock
    * will take care */
   efl_text_cursor_object_char_coord_set(cur, EINA_POSITION2D(cx, cy));
   if (_cx) *_cx = cx;
   if (_cy) *_cy = cy;
}

static void
_mouse_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Coord cx, cy;
   Evas_Event_Mouse_Down *ev = event_info;
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl_Text_Cursor_Object *cur = efl_text_interactive_main_cursor_get(obj);
   Efl_Text_Cursor_Object *tc = NULL;
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
                  tc = efl_canvas_textblock_cursor_create(obj);
                  _cur_pos_copy(cur, tc);
                  if (efl_text_cursor_object_compare(cur, en->sel_start) < 0)
                    efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_START);
                  else
                    efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_END);
                  _sel_extend(cur, obj, en);
               }
             else
               {
                  en->have_selection = EINA_FALSE;
                  en->selecting = EINA_FALSE;
                  _sel_clear(obj, en);
                  tc = efl_canvas_textblock_cursor_create(obj);
                  _cur_pos_copy(cur, tc);
                  efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_START);
                  _sel_init(cur, en);
                  efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_LINE_END);
                  _sel_extend(cur, obj, en);
               }
             goto end;
          }
        else if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
          {
             if (shift)
               {
                  tc = efl_canvas_textblock_cursor_create(obj);
                  _cur_pos_copy(cur, tc);
                  if (efl_text_cursor_object_compare(cur, en->sel_start) < 0)
                    efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START);
                  else
                    {
                       efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END);
                       efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT);
                    }
                  _sel_extend(cur, obj, en);
               }
             else
               {
                  en->have_selection = EINA_FALSE;
                  en->selecting = EINA_FALSE;
                  _sel_clear(obj, en);
                  tc = efl_canvas_textblock_cursor_create(obj);
                  _cur_pos_copy(cur, tc);
                  efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_START);
                  _sel_init(cur, en);
                  efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_WORD_END);
                  efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_CHARACTER_NEXT);
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
             _sel_init(cur, en);
          }
     }

end:
   (void) 0;
}

static void
_mouse_up_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Coord cx, cy;
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl_Text_Cursor_Object *cur = efl_text_interactive_main_cursor_get(obj);
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
        _cur_pos_copy(en->sel_end, cur);
     }
   if (en->selecting)
     {
        if (en->have_selection)
          en->had_sel = EINA_TRUE;
        en->selecting = EINA_FALSE;
     }

   _entry_imf_cursor_info_set(en);
}

static void
_mouse_move_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Coord cx, cy;
   Efl_Ui_Internal_Text_Interactive_Data *en = efl_data_scope_get(obj, MY_CLASS);
   Efl_Text_Cursor_Object *cur = efl_text_interactive_main_cursor_get(obj);
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord x, y, w, h;
   Efl_Text_Cursor_Object *tc;
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
        tc = efl_canvas_textblock_cursor_create(obj);
        _cur_pos_copy(cur, tc);
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        cx = ev->cur.canvas.x - x;
        cy = ev->cur.canvas.y - y;

        if (multiline)
          {
             efl_text_cursor_object_char_coord_set(cur, EINA_POSITION2D(cx, cy));
          }
        else
          {
             Evas_Coord lx, ly, lw, lh;
             efl_text_cursor_object_move(cur, EFL_TEXT_CURSOR_MOVE_TYPE_FIRST);
             evas_textblock_cursor_line_geometry_get(efl_text_cursor_object_handle_get(cur), &lx, &ly, &lw, &lh);
             efl_text_cursor_object_char_coord_set(cur, EINA_POSITION2D(cx, ly + (lh / 2)));
          }

        if (en->select_allow)
          {
             _sel_extend(cur, obj, en);

             if (!efl_text_cursor_object_equal(en->sel_start, en->sel_end))
               _sel_enable(obj, en);
          }
        efl_del(tc);
     }
}

EOLIAN static Efl_Text_Cursor_Object *
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
   en->watch_selection = EINA_TRUE;
   return obj;
}

EOLIAN static void
_efl_ui_internal_text_interactive_efl_object_destructor(Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *sd)
{
   eina_stringshare_del(sd->file);
   sd->file = NULL;
   efl_destructor(efl_super(obj, MY_CLASS));
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

   en->main_cursor = efl_canvas_textblock_cursor_create(obj);
   en->sel_start = efl_canvas_textblock_cursor_create(obj);
   en->sel_end = efl_canvas_textblock_cursor_create(obj);

   efl_event_callback_add(en->sel_start, EFL_TEXT_CURSOR_OBJECT_EVENT_CHANGED,
         _sel_cursor_changed, obj);
   efl_event_callback_add(en->sel_end, EFL_TEXT_CURSOR_OBJECT_EVENT_CHANGED,
         _sel_cursor_changed, obj);

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
_efl_ui_internal_text_interactive_efl_text_text_set(Eo *eo_obj, Efl_Ui_Internal_Text_Interactive_Data *o,
      const char *text)
{
   efl_text_set(efl_super(eo_obj, MY_CLASS), text);
   efl_text_cursor_object_move(o->main_cursor, EFL_TEXT_CURSOR_MOVE_TYPE_LAST);
}

EOLIAN void
_efl_ui_internal_text_interactive_efl_text_markup_markup_set(Eo *eo_obj, Efl_Ui_Internal_Text_Interactive_Data *o,
      const char *text)
{
   efl_text_markup_set(efl_super(eo_obj, MY_CLASS), text);
   efl_text_cursor_object_move(o->main_cursor, EFL_TEXT_CURSOR_MOVE_TYPE_LAST);
}

EOLIAN static void
_efl_ui_internal_text_interactive_efl_text_interactive_selection_allowed_set(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *pd, Eina_Bool allowed)
{
   if (pd->select_allow == allowed)
     return;

   pd->select_allow = allowed;
   if (!allowed)
     {
        _sel_clear(obj, pd);
     }
}

EOLIAN static Eina_Bool
_efl_ui_internal_text_interactive_efl_text_interactive_selection_allowed_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *pd)
{
   return pd->select_allow;
}

EOLIAN static void
_efl_ui_internal_text_interactive_efl_text_interactive_selection_cursors_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *pd, Efl_Text_Cursor_Object **start, Efl_Text_Cursor_Object **end)
{
   if (efl_text_cursor_object_position_get(pd->sel_start) >
         efl_text_cursor_object_position_get(pd->sel_end))
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
_efl_ui_internal_text_interactive_efl_text_interactive_selection_cursors_set(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en, Efl_Text_Cursor_Object *start, Efl_Text_Cursor_Object *end)
{
   if (!efl_text_interactive_selection_allowed_get(obj))
     return;

   int new_sel_start_pos = efl_text_cursor_object_position_get(start);
   int new_sel_end_pos = efl_text_cursor_object_position_get(end);

   int current_sel_start_pos = efl_text_cursor_object_position_get(en->sel_start);
   int current_sel_end_pos = efl_text_cursor_object_position_get(en->sel_end);

   Eina_Bool b_start_changed = (new_sel_start_pos == current_sel_start_pos);
   Eina_Bool b_end_changed = (new_sel_end_pos == current_sel_end_pos);

   if (b_start_changed && b_end_changed)
     return;

   _sel_watch_freeze(en);
   efl_text_cursor_object_position_set(en->sel_start, new_sel_start_pos);
   efl_text_cursor_object_position_set(en->sel_end, new_sel_end_pos);
   _sel_watch_thaw(en);
   _sel_reset(obj, en);
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

EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_hide(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_hide(en->imf_context);
#else
   (void)en;
#endif
}


EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_language_set(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en, Efl_Input_Text_Panel_Language_Type lang)
{
   en->input_panel_lang = lang;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_language_set(en->imf_context, (Ecore_IMF_Input_Panel_Lang)lang);
#else
   (void)en;
   (void)lang;
#endif
}


EOLIAN static Efl_Input_Text_Panel_Language_Type
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_language_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   return en->input_panel_lang;
}


EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_imdata_set(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en, Eina_Slice slice)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_imdata_set(en->imf_context, slice.mem, slice.len);
#else
   (void)en;
   (void)data;
   (void)len;
#endif
}


EOLIAN static Eina_Slice
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_imdata_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   Eina_Slice slice = {0};

#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        int len;
        ecore_imf_context_input_panel_imdata_get(en->imf_context, &slice.mem, &len);
        slice.len = (size_t)len;
     }
#else
   (void)en;
   (void)data;
   (void)len;
#endif
   return slice;
}


EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_return_key_type_set(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en, Efl_Input_Text_Panel_Return_Key_Type return_key_type)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_return_key_type_set(en->imf_context, (Ecore_IMF_Input_Panel_Return_Key_Type)return_key_type);
#else
   (void)en;
   (void)return_key_type;
#endif
}


EOLIAN static Efl_Input_Text_Panel_Return_Key_Type
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_return_key_type_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return (Efl_Input_Text_Panel_Return_Key_Type)ecore_imf_context_input_panel_return_key_type_get(en->imf_context);
   return EFL_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_DEFAULT;
#else
   return EFL_INPUT_TEXT_PANEL_RETURN_KEY_TYPE_DEFAULT;
   (void)en;
#endif
}

EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_return_key_state_set(Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *en, Efl_Input_Text_Panel_Return_Key_State state)
{
   if (en->input_panel_return_key_state == state)
     return;

   en->input_panel_return_key_state = state;

#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        switch (state)
        {
           case EFL_INPUT_TEXT_PANEL_RETURN_KEY_STATE_ENABLED:
              ecore_imf_context_input_panel_return_key_disabled_set(en->imf_context, EINA_TRUE);
           break;
           case EFL_INPUT_TEXT_PANEL_RETURN_KEY_STATE_DISABLED:
              ecore_imf_context_input_panel_return_key_disabled_set(en->imf_context, EINA_FALSE);
           break;
           case EFL_INPUT_TEXT_PANEL_RETURN_KEY_STATE_AUTO:
              _return_key_update(obj);
           break;
        default:
           break;
        }
     }
#else
   (void)obj;
   (void)en;
   (void)disabled;
#endif
}


EOLIAN static Efl_Input_Text_Panel_Return_Key_State
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_return_key_state_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   return en->input_panel_return_key_state;
}


EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_show_on_demand_set(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en, Eina_Bool ondemand)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_show_on_demand_set(en->imf_context, ondemand);
#else
   (void)en;
   (void)ondemand;
#endif
}

EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_layout_set(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *sd, Efl_Input_Text_Panel_Layout_Type layout)
{
   sd->input_panel_layout = layout;

#ifdef HAVE_ECORE_IMF
    if (sd->imf_context)
     ecore_imf_context_input_panel_layout_set(sd->imf_context, (Ecore_IMF_Input_Panel_Layout)layout);
#endif

   if (layout == EFL_INPUT_TEXT_PANEL_LAYOUT_TYPE_PASSWORD)
     efl_input_text_input_content_type_set(obj, ((sd->input_hints & ~EFL_INPUT_TEXT_CONTENT_TYPE_AUTO_COMPLETE) | EFL_INPUT_TEXT_CONTENT_TYPE_SENSITIVE_DATA));
   else if (layout == EFL_INPUT_TEXT_PANEL_LAYOUT_TYPE_TERMINAL)
     efl_input_text_input_content_type_set(obj, (sd->input_hints & ~EFL_INPUT_TEXT_CONTENT_TYPE_AUTO_COMPLETE));
}

EOLIAN static Efl_Input_Text_Panel_Layout_Type
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_layout_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *sd)
{
   return sd->input_panel_layout;
}

EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_layout_variation_set(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *sd, int variation)
{
   sd->input_panel_layout_variation = variation;

#ifdef HAVE_ECORE_IMF
   if (sd->imf_context)
     ecore_imf_context_input_panel_layout_variation_set(sd->imf_context, variation);
#else
   (void)variation;
#endif

   if (sd->input_panel_layout == EFL_INPUT_TEXT_PANEL_LAYOUT_TYPE_NORMAL &&
       variation == EFL_INPUT_TEXT_PANEL_LAYOUT_NORMAL_VARIATION_TYPE_PERSON_NAME)
     efl_input_text_autocapitalization_set(obj, EFL_INPUT_TEXT_CAPITALIZE_TYPE_WORD);
}

EOLIAN static int
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_layout_variation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *sd)
{
   return sd->input_panel_layout_variation;
}

EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_show(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_show(en->imf_context);
#else
   (void)en;
#endif
}

EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_autoshow_set(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en, Eina_Bool enabled)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_enabled_set(en->imf_context, enabled);
#else
   (void)en;
   (void)enabled;
#endif
}

EOLIAN static Eina_Bool
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_autoshow_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return ecore_imf_context_input_panel_enabled_get(en->imf_context);
#else
   (void)en;
#endif
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_internal_text_interactive_efl_input_text_entity_input_panel_show_on_demand_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        Eina_Bool ret = ecore_imf_context_input_panel_show_on_demand_get(en->imf_context);
        return ret;
     }
#else
   (void)en;
#endif
   return EINA_FALSE;
}


EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_predictable_set(Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en, Eina_Bool prediction)
{
   en->prediction_allow = prediction;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_prediction_allow_set(en->imf_context, prediction);
#else
   (void)en;
   (void)prediction;
#endif
}


EOLIAN static Eina_Bool
_efl_ui_internal_text_interactive_efl_input_text_entity_predictable_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
   return en->prediction_allow;
}


EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_input_content_type_set(Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *en, Efl_Input_Text_Content_Type input_hints)
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


EOLIAN static Efl_Input_Text_Content_Type
_efl_ui_internal_text_interactive_efl_input_text_entity_input_content_type_get(const Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return (Efl_Input_Text_Content_Type)ecore_imf_context_input_hint_get(en->imf_context);
   (void)obj;
#else
   (void)obj;
   (void)en;
#endif

   return EFL_INPUT_TEXT_CONTENT_TYPE_NONE;
}


EOLIAN static void
_efl_ui_internal_text_interactive_efl_input_text_entity_autocapitalization_set(Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *en, Efl_Input_Text_Capitalize_Type autocapital_type)
{
#ifdef HAVE_ECORE_IMF
   if (efl_text_password_get(obj) == EINA_TRUE)
     autocapital_type = EFL_INPUT_TEXT_CAPITALIZE_TYPE_NONE;

   if (en->imf_context)
     ecore_imf_context_autocapital_type_set(en->imf_context, (Ecore_IMF_Autocapital_Type)autocapital_type);
#else
   (void)obj;
   (void)en;
   (void)autocapital_type;
#endif
}


EOLIAN static Efl_Input_Text_Capitalize_Type
_efl_ui_internal_text_interactive_efl_input_text_entity_autocapitalization_get(const Eo *obj EINA_UNUSED, Efl_Ui_Internal_Text_Interactive_Data *en)
{
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return (Efl_Input_Text_Capitalize_Type)ecore_imf_context_autocapital_type_get(en->imf_context);
   return EFL_INPUT_TEXT_CAPITALIZE_TYPE_NONE;
#else
   (void)en;
   return EFL_INPUT_TEXT_CAPITALIZE_TYPE_NONE;
#endif
}


static char *
_file_load(Eo *obj)
{
   Eina_File *f;
   char *text = NULL;
   void *tmp = NULL;

   f = eina_file_dup(efl_file_mmap_get(obj));

   tmp = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!tmp) goto on_error;

   text = malloc(eina_file_size_get(f) + 1);
   if (!text) goto on_error;

   memcpy(text, tmp, eina_file_size_get(f));
   text[eina_file_size_get(f)] = 0;

   if (eina_file_map_faulted(f, tmp))
     {
        ELM_SAFE_FREE(text, free);
     }

 on_error:
   if (tmp) eina_file_map_free(f, tmp);
   eina_file_close(f);

   return text;
}

static char *
_plain_load(Eo *obj)
{
   return _file_load(obj);
}

static Eina_Error
_load_do(Evas_Object *obj)
{
   char *text;
   Eina_Error err = 0;

   Efl_Ui_Internal_Text_Interactive_Data * sd = efl_data_scope_get(obj, MY_CLASS);

   if (!sd->file)
     {
        efl_text_set(obj, "");
        return 0;
     }

   switch (sd->format)
     {
      /* Only available format */
      case ELM_TEXT_FORMAT_PLAIN_UTF8:
         text = _plain_load(obj);
         if (!text)
           {
              err = errno;
              if (!err) err = ENOENT;
           }
         break;

      default:
         text = NULL;
         break;
     }

   if (text)
     {
        efl_text_set(obj, text);
        free(text);
        return 0;
     }
   efl_text_set(obj, "");
   return err;
}

static void
_text_save(const char *file,
           const char *text)
{
   FILE *f;

   if (!text)
     {
        ecore_file_unlink(file);
        return;
     }

   f = fopen(file, "wb");
   if (!f)
     {
        ERR("Failed to open %s for writing", file);
        return;
     }

   if (fputs(text, f) == EOF)
     ERR("Failed to write text to file %s", file);
   fclose(f);
}

static void
_save_do(Evas_Object *obj)
{
   Efl_Ui_Internal_Text_Interactive_Data * sd = efl_data_scope_get(obj, MY_CLASS);

   if (!sd->file) return;
   switch (sd->format)
     {
      /* Only supported format */
      case ELM_TEXT_FORMAT_PLAIN_UTF8:
        _text_save(sd->file, efl_text_get(obj));
        break;

      case ELM_TEXT_FORMAT_MARKUP_UTF8:
      default:
        break;
     }
}


EOLIAN static Eina_Error
_efl_ui_internal_text_interactive_efl_file_file_set(Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *sd, const char *file)
{
   eina_stringshare_replace(&sd->file, file);
   return efl_file_set(efl_super(obj, MY_CLASS), file);
}

EOLIAN static void
_efl_ui_internal_text_interactive_efl_file_unload(Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *sd EINA_UNUSED)
{
   efl_file_unload(efl_super(obj, MY_CLASS));
   efl_text_set(obj, "");
}

EOLIAN static Eina_Error
_efl_ui_internal_text_interactive_efl_file_load(Eo *obj, Efl_Ui_Internal_Text_Interactive_Data *sd)
{
   Eina_Error err;

   if (efl_file_loaded_get(obj)) return 0;
   err = efl_file_load(efl_super(obj, MY_CLASS));
   if (err) return err;
   if (sd->auto_save) _save_do(obj);
   return _load_do(obj);
}


#include "efl_ui_internal_text_interactive.eo.c"
#include "efl_text_interactive.eo.c"
