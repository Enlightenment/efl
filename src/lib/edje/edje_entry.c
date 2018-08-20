#include "edje_private.h"

#ifdef HAVE_ECORE_IMF
static Eina_Bool _edje_entry_imf_retrieve_surrounding_cb(void *data, Ecore_IMF_Context *ctx, char **text, int *cursor_pos);
static void      _edje_entry_imf_event_commit_cb(void *data, Ecore_IMF_Context *ctx, void *event_info);
static void      _edje_entry_imf_event_preedit_changed_cb(void *data, Ecore_IMF_Context *ctx, void *event_info);
static void      _edje_entry_imf_event_delete_surrounding_cb(void *data, Ecore_IMF_Context *ctx, void *event);
static void      _edje_entry_imf_event_selection_set_cb(void *data, Ecore_IMF_Context *ctx, void *event_info);
static Eina_Bool _edje_entry_imf_retrieve_selection_cb(void *data, Ecore_IMF_Context *ctx, char **text);
#endif

typedef struct _Entry  Entry;
typedef struct _Sel    Sel;
typedef struct _Anchor Anchor;
typedef struct _Item_Obj Item_Obj;

static void _edje_entry_imf_cursor_location_set(Entry *en);
static void _edje_entry_imf_cursor_info_set(Entry *en);
static void _range_del_emit(Edje *ed, Evas_Textblock_Cursor *c EINA_UNUSED, Evas_Object *o EINA_UNUSED, Entry *en);
static void _text_filter_format_prepend(Edje *ed, Entry *en, Evas_Textblock_Cursor *c, const char *text);
static void _free_entry_change_info(void *_info);

struct _Entry
{
   Edje_Real_Part        *rp;
   Edje                  *ed;
   Evas_Coord             ox, oy;
   Evas_Object           *cursor_bg;
   Evas_Object           *cursor_fg, *cursor_fg2;
   Evas_Textblock_Cursor *cursor;
   Evas_Textblock_Cursor *sel_start, *sel_end;
   Evas_Textblock_Cursor *cursor_user, *cursor_user_extra;
   Evas_Textblock_Cursor *preedit_start, *preedit_end;
   Ecore_Timer           *pw_timer;
   Eina_List             *sel;
   Eina_List             *anchors;
   Eina_List             *anchorlist;
   Eina_List             *itemlist;
   Eina_List             *seq;
   Item_Obj              *item_objs;
   char                  *selection;
   Edje_Input_Panel_Lang  input_panel_lang;
   Eina_Bool              composing : 1;
   Eina_Bool              selecting : 1;
   Eina_Bool              have_selection : 1;
   Eina_Bool              select_allow : 1;
   Eina_Bool              select_mod_start : 1;
   Eina_Bool              select_mod_end : 1;
   Eina_Bool              had_sel : 1;
   Eina_Bool              input_panel_enable : 1;
   Eina_Bool              prediction_allow : 1;
   Eina_Bool              anchors_updated : 1;
   Eina_Bool              have_link_pressed : 1;

#ifdef HAVE_ECORE_IMF
   Eina_Bool              have_preedit : 1;
   Eina_Bool              commit_cancel : 1; // For skipping useless commit
   Ecore_IMF_Context     *imf_context;
#endif
};

struct _Sel
{
   Evas_Textblock_Rectangle rect;
   Evas_Object             *obj_fg, *obj_bg, *obj, *sobj;
};

struct _Anchor
{
   Entry                 *en;
   char                  *name;
   Evas_Textblock_Cursor *start, *end;
   Eina_List             *sel;
   Eina_Bool              item : 1;
};

struct _Item_Obj
{
   EINA_INLIST;
   Anchor                *an;
   char                  *name;
   Evas_Object           *obj;
};

#ifdef HAVE_ECORE_IMF
static void
_preedit_clear(Entry *en)
{
   if (en->preedit_start)
     {
        evas_textblock_cursor_free(en->preedit_start);
        en->preedit_start = NULL;
     }

   if (en->preedit_end)
     {
        evas_textblock_cursor_free(en->preedit_end);
        en->preedit_end = NULL;
     }

   en->have_preedit = EINA_FALSE;
}

static void
_preedit_del(Entry *en)
{
   if (!en || !en->have_preedit) return;
   if (!en->preedit_start || !en->preedit_end) return;
   if (!evas_textblock_cursor_compare(en->preedit_start, en->preedit_end)) return;

   /* delete the preedit characters */
   evas_textblock_cursor_range_delete(en->preedit_start, en->preedit_end);
}

static void
_edje_entry_focus_in_cb(void *data, Evas_Object *o, const char *emission, const char *source EINA_UNUSED)
{
   Efl_Input_Device *seat;
   const char *seat_name;
   Edje_Real_Part *rp;
   Entry *en;
   Edje *ed;

   rp = data;
   if ((!rp) || (rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   if (!rp->typedata.text->entry_data) return;

   ed = _edje_fetch(o);
   if (!ed) return;

   en = rp->typedata.text->entry_data;
   if (!en || !en->imf_context) return;

   seat_name = emission + sizeof("focus,part,in,") - 1;
   seat = _edje_seat_get(ed, seat_name);

   if (efl_canvas_object_seat_focus_check(ed->obj, seat))
     {
        ecore_imf_context_focus_in(en->imf_context);
        _edje_entry_imf_cursor_info_set(en);
     }
}

static void
_edje_entry_focus_out_cb(void *data, Evas_Object *o EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   Edje_Real_Part *rp;
   Entry *en;

   rp = data;
   if ((!rp) || (rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   if (!rp->typedata.text->entry_data) return;

   en = rp->typedata.text->entry_data;
   if (!en || !en->imf_context) return;

   ecore_imf_context_reset(en->imf_context);
   ecore_imf_context_focus_out(en->imf_context);
}

#endif

static void
_edje_focus_in(Edje *ed, Efl_Input_Device *seat)
{
#ifdef HAVE_ECORE_IMF
   Edje_Real_Part *rp;
   Entry *en;
#endif

   _edje_seat_emit(ed, seat, "focus,in", "");
#ifdef HAVE_ECORE_IMF
   rp = _edje_focused_part_get(ed, _edje_seat_name_get(ed, seat));
   if (!rp) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;

   if (!en->imf_context) return;

   ecore_imf_context_focus_in(en->imf_context);
   _edje_entry_imf_cursor_info_set(en);
#endif
}

static void
_edje_focus_in_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Focus *ev = event->info;

   _edje_focus_in(data, efl_input_device_get(ev));
}

static void
_edje_focus_out(Edje *ed, Efl_Input_Device *seat)
{
#ifdef HAVE_ECORE_IMF
   Edje_Real_Part *rp;
   Entry *en;
#endif

   _edje_seat_emit(ed, seat, "focus,out", "");

#ifdef HAVE_ECORE_IMF
   rp = _edje_focused_part_get(ed, _edje_seat_name_get(ed, seat));
   if (!rp) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;

   if (!en->imf_context) return;

   ecore_imf_context_reset(en->imf_context);
   ecore_imf_context_focus_out(en->imf_context);
#endif
}

static void
_edje_focus_out_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Focus *ev = event->info;

   _edje_focus_out(data, efl_input_device_get(ev));
}

static Edje_Entry_Change_Info *
_text_filter_markup_prepend_internal(Edje *ed, Entry *en, Evas_Textblock_Cursor *c,
                                     char *text,
                                     const char *fmtpre, const char *fmtpost,
                                     Eina_Bool clearsel, Eina_Bool changeinfo)
{
   Edje_Markup_Filter_Callback *cb;
   Eina_List *l;
   Eina_Bool have_sel = EINA_FALSE;

   if ((clearsel) && (en->have_selection))
     {
        _range_del_emit(ed, en->cursor, en->rp->object, en);
        have_sel = EINA_TRUE;
     }

   EINA_LIST_FOREACH(ed->markup_filter_callbacks, l, cb)
     {
        if (!strcmp(cb->part, en->rp->part->name))
          {
             cb->func(cb->data, ed->obj, cb->part, &text);
             if (!text) break;
          }
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
        Edje_Entry_Change_Info *info = NULL;

        if (changeinfo)
          {
             info = calloc(1, sizeof(*info));
             if (!info)
               {
                  ERR("Running very low on memory");
               }
             else
               {
                  info->insert = EINA_TRUE;
                  info->change.insert.content = eina_stringshare_add(text);
                  info->change.insert.plain_length =
                    eina_unicode_utf8_get_len(info->change.insert.content);
               }
          }
        if (info)
          {
             if (have_sel)
               {
                  info->merge = EINA_TRUE;
               }
             info->change.insert.pos =
                evas_textblock_cursor_pos_get(en->cursor);
          }
        if (fmtpre) _text_filter_format_prepend(ed, en, en->cursor, fmtpre);
        evas_object_textblock_text_markup_prepend(c, text);
        free(text);
        if (fmtpost) _text_filter_format_prepend(ed, en, en->cursor, fmtpost);
        return info;
     }
   return NULL;
}

static Edje_Entry_Change_Info *
_text_filter_text_prepend(Edje *ed, Entry *en, Evas_Textblock_Cursor *c,
                          const char *text,
                          const char *fmtpre, const char *fmtpost,
                          Eina_Bool clearsel, Eina_Bool changeinfo)
{
   char *text2 = NULL;
   Edje_Text_Insert_Filter_Callback *cb;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(text, NULL);

   if ((clearsel) && (en->have_selection))
     {
        _range_del_emit(ed, en->cursor, en->rp->object, en);
     }

   text2 = strdup(text);
   EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if (!strcmp(cb->part, en->rp->part->name))
          {
             cb->func(cb->data, ed->obj, cb->part, EDJE_TEXT_FILTER_TEXT, &text2);
             if (!text2) break;
          }
     }
   if (text2)
     {
        char *markup_text;
        Edje_Entry_Change_Info *info = NULL;

        markup_text = evas_textblock_text_utf8_to_markup(NULL, text2);
        free(text2);
        if (markup_text)
          info = _text_filter_markup_prepend_internal(ed, en, c, markup_text,
                                                      fmtpre, fmtpost,
                                                      clearsel, changeinfo);
        return info;
     }
   return NULL;
}

static void
_text_filter_format_prepend(Edje *ed, Entry *en, Evas_Textblock_Cursor *c, const char *text)
{
   char *text2;
   Edje_Text_Insert_Filter_Callback *cb;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(text);
   text2 = strdup(text);
   EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if (!strcmp(cb->part, en->rp->part->name))
          {
             cb->func(cb->data, ed->obj, cb->part, EDJE_TEXT_FILTER_FORMAT, &text2);
             if (!text2) break;
          }
     }
   if (text2)
     {
        char *s, *markup_text;

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
             markup_text = (char *)malloc(strlen(s) + 3);
             if (markup_text)
               {
                  *(markup_text) = '<';
                  strncpy((markup_text + 1), s, strlen(s));
                  *(markup_text + strlen(s) + 1) = '>';
                  *(markup_text + strlen(s) + 2) = '\0';
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
             markup_text = (char *)malloc(strlen(s) + 4);
             if (markup_text)
               {
                  *(markup_text) = '<';
                  *(markup_text + 1) = '/';
                  strncpy((markup_text + 2), s, strlen(s));
                  *(markup_text + strlen(s) + 2) = '>';
                  *(markup_text + strlen(s) + 3) = '\0';
               }
          }
        else
          {
             markup_text = (char *)malloc(strlen(s) + 4);
             if (markup_text)
               {
                  *(markup_text) = '<';
                  strncpy((markup_text + 1), s, strlen(s));
                  *(markup_text + strlen(s) + 1) = '/';
                  *(markup_text + strlen(s) + 2) = '>';
                  *(markup_text + strlen(s) + 3) = '\0';
               }
          }
        free(text2);
        if (markup_text)
          _text_filter_markup_prepend_internal(ed, en, c, markup_text,
                                               NULL, NULL,
                                               EINA_FALSE, EINA_FALSE);
     }
}

static Edje_Entry_Change_Info *
_text_filter_markup_prepend(Edje *ed, Entry *en, Evas_Textblock_Cursor *c,
                            const char *text,
                            const char *fmtpre, const char *fmtpost,
                            Eina_Bool clearsel, Eina_Bool changeinfo)
{
   char *text2;
   Edje_Text_Insert_Filter_Callback *cb;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(text, NULL);

   if ((clearsel) && (en->have_selection))
     {
        _range_del_emit(ed, en->cursor, en->rp->object, en);
     }

   text2 = strdup(text);
   EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if (!strcmp(cb->part, en->rp->part->name))
          {
             cb->func(cb->data, ed->obj, cb->part, EDJE_TEXT_FILTER_MARKUP, &text2);
             if (!text2) break;
          }
     }
   if (text2)
     {
        Edje_Entry_Change_Info *info;

        info = _text_filter_markup_prepend_internal(ed, en, c, text2,
                                                    fmtpre, fmtpost,
                                                    clearsel, changeinfo);
        return info;
     }
   return NULL;
}

static void
_curs_update_from_curs(Evas_Textblock_Cursor *c, Evas_Object *o EINA_UNUSED, Entry *en, Evas_Coord *cx, Evas_Coord *cy)
{
   Evas_Coord cw, ch;
   Evas_Textblock_Cursor_Type cur_type;
   if (c != en->cursor) return;
   switch (en->rp->part->cursor_mode)
     {
      case EDJE_ENTRY_CURSOR_MODE_BEFORE:
        cur_type = EVAS_TEXTBLOCK_CURSOR_BEFORE;
        break;

      case EDJE_ENTRY_CURSOR_MODE_UNDER:
      /* no break for a reason */
      default:
        cur_type = EVAS_TEXTBLOCK_CURSOR_UNDER;
     }
   evas_textblock_cursor_geometry_get(c, cx, cy, &cw, &ch, NULL, cur_type);
   *cx += (cw / 2);
   *cy += (ch / 2);
}

static int
_curs_line_last_get(Evas_Textblock_Cursor *c EINA_UNUSED, Evas_Object *o, Entry *en EINA_UNUSED)
{
   Evas_Textblock_Cursor *cc;
   int ln;

   cc = evas_object_textblock_cursor_new(o);
   evas_textblock_cursor_paragraph_last(cc);
   ln = evas_textblock_cursor_line_geometry_get(cc, NULL, NULL, NULL, NULL);
   evas_textblock_cursor_free(cc);
   return ln;
}

static void
_curs_lin_start(Evas_Textblock_Cursor *c, Evas_Object *o EINA_UNUSED,
                Entry *en EINA_UNUSED)
{
   evas_textblock_cursor_line_char_first(c);
}

static void
_curs_lin_end(Evas_Textblock_Cursor *c, Evas_Object *o EINA_UNUSED,
              Entry *en EINA_UNUSED)
{
   evas_textblock_cursor_line_char_last(c);
}

static void
_curs_start(Evas_Textblock_Cursor *c, Evas_Object *o EINA_UNUSED,
            Entry *en EINA_UNUSED)
{
   evas_textblock_cursor_paragraph_first(c);
}

static void
_curs_end(Evas_Textblock_Cursor *c, Evas_Object *o EINA_UNUSED, Entry *en EINA_UNUSED)
{
   evas_textblock_cursor_paragraph_last(c);
}

static Eina_Bool
_curs_jump_line(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en, int ln)
{
   Evas_Coord cx, cy;
   Evas_Coord lx, ly, lw, lh;
   int last = _curs_line_last_get(c, o, en);

   if (ln < 0) return EINA_FALSE;
   if (ln > last) return EINA_FALSE;

   _curs_update_from_curs(c, o, en, &cx, &cy);

   if (!evas_object_textblock_line_number_geometry_get(o, ln, &lx, &ly, &lw, &lh))
     return EINA_FALSE;
   if (evas_textblock_cursor_cluster_coord_set(c, cx, ly + (lh / 2)))
     return EINA_TRUE;
   evas_textblock_cursor_line_set(c, ln);
   if (cx < (lx + (lw / 2)))
     {
        if (ln == last) _curs_end(c, o, en);
        _curs_lin_start(c, o, en);
     }
   else
     {
        if (ln == last)
          _curs_end(c, o, en);
        else
          _curs_lin_end(c, o, en);
     }
   return EINA_TRUE;
}

static Eina_Bool
_curs_jump_line_by(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en, int by)
{
   int ln;

   ln = evas_textblock_cursor_line_geometry_get(c, NULL, NULL, NULL, NULL) + by;
   return _curs_jump_line(c, o, en, ln);
}

static Eina_Bool
_curs_up(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   return _curs_jump_line_by(c, o, en, -1);
}

static Eina_Bool
_curs_down(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   return _curs_jump_line_by(c, o, en, 1);
}

static void
_sel_start(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (en->sel_start) return;
   en->sel_start = evas_object_textblock_cursor_new(o);
   evas_textblock_cursor_copy(c, en->sel_start);
   en->sel_end = evas_object_textblock_cursor_new(o);
   evas_textblock_cursor_copy(c, en->sel_end);

   en->have_selection = EINA_FALSE;
   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }
}

static void
_sel_enable(Edje *ed, Evas_Textblock_Cursor *c EINA_UNUSED,
            Evas_Object *o EINA_UNUSED, Entry *en)
{
   if (en->have_selection) return;
   en->have_selection = EINA_TRUE;
   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }

   _edje_entry_imf_context_reset(en->rp);
   _edje_emit(ed, "selection,start", en->rp->part->name);
}

static void
_emit_sel_state(Edje *ed, Entry *en)
{
   if (!evas_textblock_cursor_compare(en->sel_start, en->sel_end))
     {
        _edje_emit(ed, "selection,reset", en->rp->part->name);
     }
   else
     {
        _edje_emit(ed, "selection,changed", en->rp->part->name);
     }
}

static void
_sel_extend(Edje *ed, Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (!en->sel_end) return;
   _sel_enable(ed, c, o, en);
   if (!evas_textblock_cursor_compare(c, en->sel_end)) return;

   evas_textblock_cursor_copy(c, en->sel_end);

   _edje_entry_imf_cursor_info_set(en);

   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }
   _emit_sel_state(ed, en);
}

static void
_sel_preextend(Edje *ed, Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (!en->sel_end) return;
   _sel_enable(ed, c, o, en);
   if (!evas_textblock_cursor_compare(c, en->sel_start)) return;

   evas_textblock_cursor_copy(c, en->sel_start);

   _edje_entry_imf_cursor_info_set(en);

   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }
   _emit_sel_state(ed, en);
}

static void
_sel_clear(Edje *ed, Evas_Textblock_Cursor *c EINA_UNUSED, Evas_Object *o EINA_UNUSED, Entry *en)
{
   en->had_sel = EINA_FALSE;
   if (en->sel_start)
     {
        evas_textblock_cursor_free(en->sel_start);
        evas_textblock_cursor_free(en->sel_end);
        en->sel_start = NULL;
        en->sel_end = NULL;
     }
   if (en->selection)
     {
        free(en->selection);
        en->selection = NULL;
     }
   while (en->sel)
     {
        Sel *sel;

        sel = en->sel->data;
        if (sel->obj_bg) evas_object_del(sel->obj_bg);
        if (sel->obj_fg) evas_object_del(sel->obj_fg);
        free(sel);
        en->sel = eina_list_remove_list(en->sel, en->sel);
     }
   if (en->have_selection)
     {
        en->have_selection = EINA_FALSE;
        _edje_emit(ed, "selection,cleared", en->rp->part->name);
     }
}

static void
_sel_update(Edje *ed, Evas_Textblock_Cursor *c EINA_UNUSED, Evas_Object *o, Entry *en)
{
   Evas_Coord x, y;
   Evas_Object *smart, *clip;

   smart = evas_object_smart_parent_get(o);
   clip = evas_object_clip_get(o);
   if (!en->sel_start)
     return;

   evas_object_geometry_get(o, &x, &y, NULL, NULL);
   if (en->have_selection)
     {
        Eina_Iterator *range = NULL;
        Eina_List *l;
        Sel *sel = NULL;
        Evas_Textblock_Rectangle *r;

        range = evas_textblock_cursor_range_simple_geometry_get(en->sel_start,
                                                                en->sel_end);

        l = en->sel;
        EINA_ITERATOR_FOREACH(range, r)
          {
             if (!l)
               {
                  Evas_Object *ob;

                  sel = calloc(1, sizeof(Sel));
                  if (!sel)
                    {
                       ERR("Running very low on memory");
                       break;
                    }
                  en->sel = eina_list_append(en->sel, sel);
                  if (en->rp->part->source)
                    {
                       ob = edje_object_add(ed->base.evas);
                       edje_object_file_set(ob, ed->path, en->rp->part->source);
                       evas_object_smart_member_add(ob, smart);
                       evas_object_stack_below(ob, o);
                       evas_object_clip_set(ob, clip);
                       evas_object_pass_events_set(ob, EINA_TRUE);
                       evas_object_show(ob);
                       sel->obj_bg = ob;
                       _edje_subobj_register(ed, sel->obj_bg);
                    }

                  if (en->rp->part->source2)
                    {
                       ob = edje_object_add(ed->base.evas);
                       edje_object_file_set(ob, ed->path, en->rp->part->source2);
                       evas_object_smart_member_add(ob, smart);
                       evas_object_stack_above(ob, o);
                       evas_object_clip_set(ob, clip);
                       evas_object_pass_events_set(ob, EINA_TRUE);
                       evas_object_show(ob);
                       sel->obj_fg = ob;
                       _edje_subobj_register(ed, sel->obj_fg);
                    }
               }
             else
               {
                  sel = eina_list_data_get(l);
                  l = l->next;
               }
             *(&(sel->rect)) = *r;

             if (sel->obj_bg)
               {
                  evas_object_move(sel->obj_bg, x + r->x, y + r->y);
                  evas_object_resize(sel->obj_bg, r->w, r->h);
               }
             if (sel->obj_fg)
               {
                  evas_object_move(sel->obj_fg, x + r->x, y + r->y);
                  evas_object_resize(sel->obj_fg, r->w, r->h);
               }
          }
        eina_iterator_free(range);

        /* delete redundant selection rects */
        while (l)
          {
             Eina_List *temp = l->next;
             sel = eina_list_data_get(l);
             if (sel)
               {
                  if (sel->obj_bg) evas_object_del(sel->obj_bg);
                  if (sel->obj_fg) evas_object_del(sel->obj_fg);
                  free(sel);
               }
             en->sel = eina_list_remove_list(en->sel, l);
             l = temp;
          }
     }
}

static Eina_Bool
_edje_entry_style_tag_check(Edje_Real_Part *rp, const char *tag)
{
    const Evas_Textblock_Style *ts = NULL;

    ts = evas_object_textblock_style_user_peek(rp->object);
    if (ts)
      {
         if (strstr(evas_textblock_style_get(ts), tag)) return EINA_TRUE;
      }
    else
      {
         ts = evas_object_textblock_style_get(rp->object);
         if (ts)
           {
              if (strstr(evas_textblock_style_get(ts), tag)) return EINA_TRUE;
           }
      }

   return EINA_FALSE;
}

static void
_edje_anchor_mouse_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Anchor *an = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Edje_Real_Part *rp = an->en->rp;
   char *buf, *n;
   size_t len;
   int ignored;
   Entry *en;
   Edje *ed = an->en->ed;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if ((rp->part->select_mode == EDJE_ENTRY_SELECTION_MODE_EXPLICIT) &&
       (en->select_allow))
     return;

   ignored = rp->ignore_flags & ev->event_flags;
   if ((!ev->event_flags) || (!ignored))
     {
        n = an->name;
        if (!n) n = "";
        len = 200 + strlen(n);
        buf = alloca(len);
        if (ev->flags & EVAS_BUTTON_TRIPLE_CLICK)
          snprintf(buf, len, "anchor,mouse,down,%i,%s,triple", ev->button, n);
        else if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
          snprintf(buf, len, "anchor,mouse,down,%i,%s,double", ev->button, n);
        else
          snprintf(buf, len, "anchor,mouse,down,%i,%s", ev->button, n);
        _edje_emit(ed, buf, rp->part->name);

        /* Link Pressed effect */
        if (_edje_entry_style_tag_check(rp, "link_pressed="))
          {
             an->en->have_link_pressed = EINA_TRUE;
             evas_textblock_cursor_format_append(an->start, "<link_pressed>");
             evas_textblock_cursor_format_prepend(an->end, "</>");
          }
     }
   ev->event_flags |= rp->mask_flags;
}

static void
_edje_anchor_mouse_up_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Anchor *an = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Edje_Real_Part *rp = an->en->rp;
   char *buf, *n;
   size_t len;
   int ignored;
   Entry *en;
   Edje *ed = an->en->ed;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;

   en = rp->typedata.text->entry_data;
   ignored = rp->ignore_flags & ev->event_flags;
   n = an->name;
   if (!n) n = "";
   len = 200 + strlen(n);
   buf = alloca(len);
   if ((rp->part->select_mode != EDJE_ENTRY_SELECTION_MODE_EXPLICIT) ||
       (!en->select_allow))
     {
        if ((!ev->event_flags) || (!ignored))
          {
             snprintf(buf, len, "anchor,mouse,up,%i,%s", ev->button, n);
             _edje_emit(ed, buf, rp->part->name);
             /* Link Pressed effect */
             if (an->en->have_link_pressed)
               {
                  const Evas_Object_Textblock_Node_Format *node;
                  node = evas_textblock_node_format_first_get(rp->object);
                  for (; node; node = evas_textblock_node_format_next_get(node))
                    {
                        const char *text = evas_textblock_node_format_text_get(node);

                        if (text)
                          {
                             if (!strcmp(text, "+ link_pressed"))
                               {
                                  evas_textblock_node_format_remove_pair(rp->object,
                                           (Evas_Object_Textblock_Node_Format *)node);
                                  break;
                               }
                          }
                    }
               }
          }
     }

   if ((rp->still_in) && (rp->clicked_button == ev->button) && (!ignored))
     {
        snprintf(buf, len, "anchor,mouse,clicked,%i,%s", ev->button, n);
        _edje_emit(ed, buf, rp->part->name);
     }
   ev->event_flags |= rp->mask_flags;
}

static void
_edje_anchor_mouse_move_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Anchor *an = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Edje_Real_Part *rp = an->en->rp;
   char *buf, *n;
   size_t len;
   int ignored;
   Entry *en;
   Edje *ed = an->en->ed;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if ((rp->part->select_mode == EDJE_ENTRY_SELECTION_MODE_EXPLICIT) &&
       (en->select_allow))
     return;

   ignored = rp->ignore_flags & ev->event_flags;
   if ((!ev->event_flags) || (!ignored))
     {
        n = an->name;
        if (!n) n = "";
        len = 200 + strlen(n);
        buf = alloca(len);
        snprintf(buf, len, "anchor,mouse,move,%s", n);
        _edje_emit(ed, buf, rp->part->name);
     }
   ev->event_flags |= rp->mask_flags;
}

static void
_edje_anchor_mouse_in_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Anchor *an = data;
   Evas_Event_Mouse_In *ev = event_info;
   Edje_Real_Part *rp = an->en->rp;
   Edje *ed = an->en->ed;
   char *buf, *n;
   size_t len;
   int ignored;

   ignored = rp->ignore_flags & ev->event_flags;
   if ((!ev->event_flags) || (!ignored))
     {
        /* set to allow handling in elementary, in case we have
         * an unwanted event propagation */
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        n = an->name;
        if (!n) n = "";
        len = 200 + strlen(n);
        buf = alloca(len);
        snprintf(buf, len, "anchor,mouse,in,%s", n);
        _edje_emit(ed, buf, rp->part->name);
     }
   ev->event_flags |= rp->mask_flags;
}

static void
_edje_anchor_mouse_out_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Anchor *an = data;
   Evas_Event_Mouse_Out *ev = event_info;
   Edje_Real_Part *rp = an->en->rp;
   char *buf, *n;
   size_t len;
   int ignored;
   Edje *ed = an->en->ed;

   ignored = rp->ignore_flags & ev->event_flags;
   if ((!ev->event_flags) || (!ignored))
     {
        /* set to allow handling in elementary, in case we have
         * an unwanted event propagation */
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        n = an->name;
        if (!n) n = "";
        len = 200 + strlen(n);
        buf = alloca(len);
        snprintf(buf, len, "anchor,mouse,out,%s", n);
        _edje_emit(ed, buf, rp->part->name);
     }
   ev->event_flags |= rp->mask_flags;
}

static void
_item_obj_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Item_Obj *io = data;
   Anchor *an = io->an;
   Entry *en;

   if (!an)
     {
        ERR("Failed to free item object struct. Anchor is NULL!");
        return;
     }

   en = an->en;
   en->item_objs = (Item_Obj *)eina_inlist_remove(EINA_INLIST_GET(en->item_objs),
                                                  EINA_INLIST_GET(io));
   io->an = NULL;
   free(io->name);
   free(io);
}

static Evas_Object *
_item_obj_get(Anchor *an, Evas_Object *o, Evas_Object *smart, Evas_Object *clip)
{
   Evas_Object *obj;
   Item_Obj *io;
   Entry *en = an->en;
   Edje *ed = en->ed;

   EINA_INLIST_FOREACH(en->item_objs, io)
     {
        if (!io->an && io->name && !strcmp(an->name ? an->name : "", io->name))
          {
             io->an = an;
             return io->obj;
          }
     }

   io = calloc(1, sizeof(Item_Obj));
   if (!io)
     {
        ERR("Running very low on memory");
        return NULL;
     }

   obj = ed->item_provider.func
      (ed->item_provider.data, smart,
       en->rp->part->name, an->name);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _item_obj_del_cb, io);
   evas_object_smart_member_add(obj, smart);
   evas_object_stack_above(obj, o);
   evas_object_clip_set(obj, clip);
   evas_object_pass_events_set(obj, EINA_TRUE);

   io->an = an;
   io->name = strdup(an->name ? an->name : "");
   io->obj = obj;
   en->item_objs = (Item_Obj *)eina_inlist_append(EINA_INLIST_GET(en->item_objs),
                                                  EINA_INLIST_GET(io));

   return io->obj;
}

static void
_unused_item_objs_free(Entry *en)
{
   Item_Obj *io;
   Eina_Inlist *l;

   EINA_INLIST_FOREACH_SAFE(en->item_objs, l, io)
     {
        if (!io->an)
          {
             if (io->obj)
               {
                  evas_object_event_callback_del_full(io->obj, EVAS_CALLBACK_DEL, _item_obj_del_cb, io);
                  evas_object_del(io->obj);
               }

             en->item_objs = (Item_Obj *)eina_inlist_remove(EINA_INLIST_GET(en->item_objs),
                                                            EINA_INLIST_GET(io));
             free(io->name);
             free(io);
          }
     }
}

static Eina_Bool
_is_anchors_outside_viewport(Evas_Coord oxy, Evas_Coord axy, Evas_Coord awh,
                                                 Evas_Coord vxy, Evas_Coord vwh)
{
   if (((oxy + axy + awh) < vxy) || ((oxy + axy) > vwh))
     {
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static void
_anchors_update(Evas_Textblock_Cursor *c EINA_UNUSED, Evas_Object *o, Entry *en)
{
   Eina_List *l, *ll, *range = NULL;
   Evas_Coord x, y, w, h;
   Evas_Coord vx, vy, vw, vh;
   Evas_Coord tvh, tvw;
   Evas_Object *smart, *clip;
   Sel *sel = NULL;
   Anchor *an;
   Edje *ed = en->ed;

   /* Better not to update anchors outside the view port. */
   if (en->anchors_updated) return;

   smart = evas_object_smart_parent_get(o);
   clip = evas_object_clip_get(o);
   x = y = w = h = -1;
   evas_object_geometry_get(o, &x, &y, &w, &h);
   evas_output_viewport_get(en->ed->base.evas, &vx, &vy, &vw, &vh);
   tvw = vx + vw;
   tvh = vy + vh;

   EINA_LIST_FOREACH(en->anchors, l, an)
     {
        // for item anchors
        if (an->item)
          {
             Evas_Coord cx, cy, cw, ch;

             if (!evas_textblock_cursor_format_item_geometry_get
                                                (an->start, &cx, &cy, &cw, &ch))
               {
                  continue;
               }

             if (_is_anchors_outside_viewport(y, cy, ch, vy, tvh) ||
                           _is_anchors_outside_viewport(x, cx, cw, vx, tvw))
               {
                  if (an->sel)
                    {
                       sel = an->sel->data;
                       evas_object_hide(sel->obj);
                    }
                  continue;
               }

             if (!an->sel)
               {
                  Evas_Object *ob;

                  sel = calloc(1, sizeof(Sel));
                  if (!sel)
                    {
                       ERR("Running very low on memory");
                       break;
                    }
                  an->sel = eina_list_append(an->sel, sel);
                  if (!an->sel)
                    {
                       ERR("Running very low on memory");
                       break;
                    }
                  if (ed->item_provider.func)
                    {
                       ob = _item_obj_get(an, o, smart, clip);
                       sel->obj = ob;
                    }
               }
             /* We have only one sel per item */
             sel = an->sel->data;
             evas_object_move(sel->obj, x + cx, y + cy);
             evas_object_resize(sel->obj, cw, ch);
             evas_object_show(sel->obj);
          }
        // for link anchors
        else
          {
             range =
               evas_textblock_cursor_range_geometry_get(an->start, an->end);
             if (eina_list_count(range) != eina_list_count(an->sel))
               {
                  while (an->sel)
                    {
                       sel = an->sel->data;
                       if (sel->obj_bg) evas_object_del(sel->obj_bg);
                       if (sel->obj_fg) evas_object_del(sel->obj_fg);
                       if (sel->obj) evas_object_del(sel->obj);
                       free(sel);
                       an->sel = eina_list_remove_list(an->sel, an->sel);
                    }
                  Evas_Textblock_Rectangle *r, *r_last;

                  r = range->data;
                  r_last = eina_list_last_data_get(range);
                  if (r->y != r_last->y)
                    {
                       /* For multiple range */
                       r->h = r->y + r_last->y + r_last->h;
                    }
                  /* For vertically layout entry */
                  if (_is_anchors_outside_viewport(y, r->y, r->h, vy, tvh))
                    {
                       EINA_LIST_FREE(range, r)
                         free(r);
                       continue;
                    }
                  else
                    {
                       /* XXX: Should consider for horizontal entry but has
                        * very minimal usage. Probably we should get the min x
                        * and max w for range and then decide whether it is in
                        * the viewport or not. Unnecessary calculation for this
                        * minimal usage. Please test with large number of anchors
                        * after implementing it, if its needed to be.
                        */
                    }
                  for (ll = range; ll; ll = eina_list_next(ll))
                    {
                       Evas_Object *ob;

                       sel = calloc(1, sizeof(Sel));
                       if (!sel)
                         {
                            ERR("Running very low on memory");
                            break;
                         }
                       an->sel = eina_list_append(an->sel, sel);
                       if (en->rp->part->source5)
                         {
                            ob = edje_object_add(ed->base.evas);
                            edje_object_file_set(ob, ed->path, en->rp->part->source5);
                            evas_object_smart_member_add(ob, smart);
                            evas_object_stack_below(ob, o);
                            evas_object_clip_set(ob, clip);
                            evas_object_pass_events_set(ob, EINA_TRUE);
                            sel->obj_bg = ob;
                            _edje_subobj_register(ed, sel->obj_bg);
                         }

                       if (en->rp->part->source6)
                         {
                            ob = edje_object_add(ed->base.evas);
                            edje_object_file_set(ob, ed->path, en->rp->part->source6);
                            evas_object_smart_member_add(ob, smart);
                            evas_object_stack_above(ob, o);
                            evas_object_clip_set(ob, clip);
                            evas_object_pass_events_set(ob, EINA_TRUE);
                            sel->obj_fg = ob;
                            _edje_subobj_register(ed, sel->obj_fg);
                         }

                       ob = evas_object_rectangle_add(ed->base.evas);
                       evas_object_color_set(ob, 0, 0, 0, 0);
                       evas_object_smart_member_add(ob, smart);
                       evas_object_stack_above(ob, o);
                       evas_object_clip_set(ob, clip);
                       evas_object_repeat_events_set(ob, EINA_TRUE);
                       evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_DOWN, _edje_anchor_mouse_down_cb, an);
                       evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_UP, _edje_anchor_mouse_up_cb, an);
                       evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_MOVE, _edje_anchor_mouse_move_cb, an);
                       evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_IN, _edje_anchor_mouse_in_cb, an);
                       evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_OUT, _edje_anchor_mouse_out_cb, an);
                       sel->obj = ob;
                    }
               }

             EINA_LIST_FOREACH(an->sel, ll, sel)
               {
                  Evas_Textblock_Rectangle *r;

                  r = range->data;
                  *(&(sel->rect)) = *r;
                  if (_is_anchors_outside_viewport(y, r->y, r->h, vy, tvh) ||
                      _is_anchors_outside_viewport(x, r->x, r->w, vx, tvw))
                    {
                       range = eina_list_remove_list(range, range);
                       free(r);
                       evas_object_hide(sel->obj_bg);
                       evas_object_hide(sel->obj_fg);
                       evas_object_hide(sel->obj);
                       continue;
                    }

                  if (sel->obj_bg)
                    {
                       evas_object_move(sel->obj_bg, x + r->x, y + r->y);
                       evas_object_resize(sel->obj_bg, r->w, r->h);
                       evas_object_show(sel->obj_bg);
                    }
                  if (sel->obj_fg)
                    {
                       evas_object_move(sel->obj_fg, x + r->x, y + r->y);
                       evas_object_resize(sel->obj_fg, r->w, r->h);
                       evas_object_show(sel->obj_fg);
                    }
                  if (sel->obj)
                    {
                       evas_object_move(sel->obj, x + r->x, y + r->y);
                       evas_object_resize(sel->obj, r->w, r->h);
                       evas_object_show(sel->obj);
                    }
                  range = eina_list_remove_list(range, range);
                  free(r);
               }
          }
     }

   _unused_item_objs_free(en);
}

static void
_anchors_update_check(Edje *ed, Edje_Real_Part *rp)
{
   Evas_Coord x, y, w, h;
   Evas_Coord vx, vy, vw, vh;
   Eina_Bool anchors_updated = EINA_FALSE;
   Entry *en;

   en = rp->typedata.text->entry_data;
   x = y = w = h = -1;
   vx = vy = vw = vh = -1;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   evas_output_viewport_get(ed->base.evas, &vx, &vy, &vw, &vh);
   if (((y + h) <= vy) || (y >= (vy + vh)))
     anchors_updated = EINA_TRUE;
   else if (((x + w) <= vx) || (x >= (vx + vw)))
     anchors_updated = EINA_TRUE;

   if (en->anchors_updated)
     en->anchors_updated = anchors_updated;
   _anchors_update(en->cursor, rp->object, en);
   en->anchors_updated = anchors_updated;
}

static void
_anchors_need_update(Edje_Real_Part *rp)
{
   Entry *en;
   Eina_Bool anchors_updated;

   en = rp->typedata.text->entry_data;
   anchors_updated = en->anchors_updated;
   en->anchors_updated = EINA_FALSE;
   _anchors_update(en->cursor, rp->object, en);
   en->anchors_updated = anchors_updated;
}

static void
_anchors_clear(Evas_Textblock_Cursor *c EINA_UNUSED, Evas_Object *o EINA_UNUSED, Entry *en)
{
   Item_Obj *io;

   while (en->anchorlist)
     {
        free(en->anchorlist->data);
        en->anchorlist = eina_list_remove_list(en->anchorlist, en->anchorlist);
     }
   while (en->itemlist)
     {
        free(en->itemlist->data);
        en->itemlist = eina_list_remove_list(en->itemlist, en->itemlist);
     }
   while (en->anchors)
     {
        Anchor *an = en->anchors->data;

        while (an->sel)
          {
             Sel *sel = an->sel->data;
             if (sel->obj_bg) evas_object_del(sel->obj_bg);
             if (sel->obj_fg) evas_object_del(sel->obj_fg);
             if (!an->item && sel->obj) evas_object_del(sel->obj);
             free(sel);
             an->sel = eina_list_remove_list(an->sel, an->sel);
          }
        evas_textblock_cursor_free(an->start);
        evas_textblock_cursor_free(an->end);
        free(an->name);
        free(an);
        en->anchors = eina_list_remove_list(en->anchors, en->anchors);
     }

   EINA_INLIST_FOREACH(en->item_objs, io)
      io->an = NULL;
}

/* FIXME: This is horrible. It's just a copy&paste (with some adjustments)
 * from textblock. I didn't want to introduce any non-API links between the
 * libs so I just copied it. Should have been handled differently. */
static char *
_anchor_format_parse(const char *item)
{
   const char *start, *end;
   char *tmp;
   size_t len;

   start = strchr(item, '=');
   if (!start) return NULL;

   start++; /* Advance after the '=' */
   /* If we can find a quote as the first non-space char,
    * our new delimiter is a quote, not a space. */
   while (*start == ' ')
     start++;

   if (*start == '\'')
     {
        start++;
        end = strchr(start, '\'');
        while ((end) && (end > start) && (end[-1] == '\\'))
          end = strchr(end + 1, '\'');
     }
   else
     {
        end = strchr(start, ' ');
        while ((end) && (end > start) && (end[-1] == '\\'))
          end = strchr(end + 1, ' ');
     }

   /* Null terminate before the spaces */
   if (end) len = end - start;
   else len = strlen(start);

   tmp = malloc(len + 1);
   if (!tmp)
     {
        ERR("Running out of memory when allocating %lu byte string", (unsigned long)len + 1);
        return NULL;
     }
   strncpy(tmp, start, len);
   tmp[len] = '\0';

   return tmp;
}

static void
_anchors_get(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   const Eina_List *anchors_a, *anchors_item;
   Anchor *an = NULL;
   _anchors_clear(c, o, en);

   anchors_a = evas_textblock_node_format_list_get(o, "a");
   anchors_item = evas_textblock_node_format_list_get(o, "item");

   if (anchors_a)
     {
        const Evas_Object_Textblock_Node_Format *node;
        const Eina_List *itr;
        EINA_LIST_FOREACH(anchors_a, itr, node)
          {
             const char *s = evas_textblock_node_format_text_get(node);
             char *p;
             an = calloc(1, sizeof(Anchor));
             if (!an)
               {
                  ERR("Running very low on memory");
                  break;
               }

             an->en = en;
             p = strstr(s, "href=");
             if (p)
               {
                  an->name = _anchor_format_parse(p);
               }
             en->anchors = eina_list_append(en->anchors, an);
             an->start = evas_object_textblock_cursor_new(o);
             an->end = evas_object_textblock_cursor_new(o);
             evas_textblock_cursor_at_format_set(an->start, node);
             evas_textblock_cursor_copy(an->start, an->end);

             /* Close the anchor, if the anchor was without text,
              * free it as well */
             node = evas_textblock_node_format_next_get(node);
             for (; node; node = evas_textblock_node_format_next_get(node))
               {
                  s = evas_textblock_node_format_text_get(node);
                  if ((!strcmp(s, "- a")) || (!strcmp(s, "-a")))
                    break;
               }

             if (node)
               {
                  evas_textblock_cursor_at_format_set(an->end, node);
               }
             else if (!evas_textblock_cursor_compare(an->start, an->end))
               {
                  free(an->name);
                  evas_textblock_cursor_free(an->start);
                  evas_textblock_cursor_free(an->end);
                  en->anchors = eina_list_remove(en->anchors, an);
                  free(an);
               }
             an = NULL;
          }
     }

   if (anchors_item)
     {
        const Evas_Object_Textblock_Node_Format *node;
        const Eina_List *itr;
        EINA_LIST_FOREACH(anchors_item, itr, node)
          {
             const char *s = evas_textblock_node_format_text_get(node);
             char *p;
             an = calloc(1, sizeof(Anchor));
             if (!an)
               {
                  ERR("Running very low on memory");
                  break;
               }

             an->en = en;
             an->item = 1;
             p = strstr(s, "href=");
             if (p)
               {
                  an->name = _anchor_format_parse(p);
               }
             en->anchors = eina_list_append(en->anchors, an);
             an->start = evas_object_textblock_cursor_new(o);
             an->end = evas_object_textblock_cursor_new(o);
             evas_textblock_cursor_at_format_set(an->start, node);
             evas_textblock_cursor_copy(an->start, an->end);
             /* Although needed in textblock, don't bother with finding the end
              * here cause it doesn't really matter. */
          }
     }
}

static void
_free_entry_change_info(void *_info)
{
   Edje_Entry_Change_Info *info = (Edje_Entry_Change_Info *)_info;
   if (info->insert)
     {
        eina_stringshare_del(info->change.insert.content);
     }
   else
     {
        eina_stringshare_del(info->change.del.content);
     }
   free(info);
}

static void
_range_del_emit(Edje *ed, Evas_Textblock_Cursor *c EINA_UNUSED, Evas_Object *o EINA_UNUSED, Entry *en)
{
   size_t start, end;
   char *tmp;
   Edje_Entry_Change_Info *info;

   start = evas_textblock_cursor_pos_get(en->sel_start);
   end = evas_textblock_cursor_pos_get(en->sel_end);
   if (start == end)
     goto noop;

   info = calloc(1, sizeof(*info));
   if (!info)
     {
        ERR("Running very low on memory");
        goto noop;
     }
   info->insert = EINA_FALSE;
   info->change.del.start = start;
   info->change.del.end = end;

   tmp = evas_textblock_cursor_range_text_get(en->sel_start, en->sel_end, EVAS_TEXTBLOCK_TEXT_MARKUP);
   info->change.del.content = eina_stringshare_add(tmp);
   if (tmp) free(tmp);
   evas_textblock_cursor_range_delete(en->sel_start, en->sel_end);
   _edje_emit(ed, "entry,changed", en->rp->part->name);
   _edje_emit_full(ed, "entry,changed,user", en->rp->part->name, info,
                   _free_entry_change_info);
noop:
   _sel_clear(ed, en->cursor, en->rp->object, en);
}

//static void
//_range_del(Evas_Textblock_Cursor *c EINA_UNUSED, Evas_Object *o EINA_UNUSED, Entry *en)
//{
//   evas_textblock_cursor_range_delete(en->sel_start, en->sel_end);
//   _sel_clear(ed, en->cursor, en->rp->object, en);
//}

static void
_delete_emit(Edje *ed, Evas_Textblock_Cursor *c, Entry *en, size_t pos,
             Eina_Bool backspace)
{
   if (!evas_textblock_cursor_char_next(c))
     {
        return;
     }
   evas_textblock_cursor_char_prev(c);

   Edje_Entry_Change_Info *info = calloc(1, sizeof(*info));
   if (!info)
     {
        ERR("Running very low on memory");
        return;
     }
   char *tmp = NULL;

   info->insert = EINA_FALSE;
   if (backspace)
     {
        info->change.del.start = pos - 1;
        info->change.del.end = pos;
        tmp = evas_textblock_cursor_content_get(c);
        evas_textblock_cursor_char_delete(c);
     }
   else
     {
        Evas_Textblock_Cursor *cc = evas_object_textblock_cursor_new(en->rp->object);
        evas_textblock_cursor_copy(c, cc);
        evas_textblock_cursor_cluster_next(cc);

        info->change.del.start = evas_textblock_cursor_pos_get(cc);
        info->change.del.end = pos;

        tmp = evas_textblock_cursor_range_text_get(c, cc, EVAS_TEXTBLOCK_TEXT_MARKUP);
        evas_textblock_cursor_range_delete(c, cc);
        evas_textblock_cursor_free(cc);
     }

   info->change.del.content = eina_stringshare_add(tmp);
   if (tmp) free(tmp);

   _edje_emit(ed, "entry,changed", en->rp->part->name);
   _edje_emit_full(ed, "entry,changed,user", en->rp->part->name,
                   info, _free_entry_change_info);
}

Eina_Bool
_edje_entry_hide_visible_password(Edje *ed, Edje_Real_Part *rp)
{
   Eina_Bool int_ret = EINA_FALSE;
   const Evas_Object_Textblock_Node_Format *node;
   node = evas_textblock_node_format_first_get(rp->object);
   for (; node; node = evas_textblock_node_format_next_get(node))
     {
        const char *text = evas_textblock_node_format_text_get(node);
        if (text)
          {
             if (!strcmp(text, "+ password=off"))
               {
                  evas_textblock_node_format_remove_pair(rp->object,
                                                         (Evas_Object_Textblock_Node_Format *)node);
                  _edje_emit(ed, "entry,changed", rp->part->name);
                  int_ret = EINA_TRUE;
                  break;
               }
          }
     }
   _edje_entry_real_part_configure(ed, rp);

   return int_ret;
}

static Eina_Bool
_password_timer_cb(void *data)
{
   Entry *en = (Entry *)data;
   _edje_entry_hide_visible_password(en->ed, en->rp);
   en->pw_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
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
_compose_seq_reset(Entry *en)
{
   char *str;

   EINA_LIST_FREE(en->seq, str)
     eina_stringshare_del(str);
   en->composing = EINA_FALSE;
}

static void
_edje_key_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Edje *ed = data;
   Evas_Event_Key_Down *ev = event_info;
   Edje_Real_Part *rp;
   Efl_Input_Device *seat;

   Entry *en;
   Eina_Bool control, alt, shift;
#if defined(__APPLE__) && defined(__MACH__)
   Eina_Bool super, altgr;
#endif
   Eina_Bool multiline;
   Eina_Bool cursor_changed;
   int old_cur_pos;

   seat = efl_input_device_seat_get(ev->dev);
   rp = _edje_focused_part_get(ed, _edje_seat_name_get(ed, seat));

   if (!rp) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;
   if (!ev->key) return;

   _edje_emit(ed, "entry,keydown", rp->part->name);
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        Ecore_IMF_Event_Key_Down ecore_ev;
        ecore_imf_evas_event_key_down_wrap(ev, &ecore_ev);
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

   old_cur_pos = evas_textblock_cursor_pos_get(en->cursor);

#if defined(__APPLE__) && defined(__MACH__)
   super = evas_key_modifier_is_set(ev->modifiers, "Super");
   altgr = evas_key_modifier_is_set(ev->modifiers, "AltGr");
#endif
   control = evas_key_modifier_is_set(ev->modifiers, "Control");
   alt = evas_key_modifier_is_set(ev->modifiers, "Alt");
   shift = evas_key_modifier_is_set(ev->modifiers, "Shift");
   multiline = rp->part->multiline;
   cursor_changed = EINA_FALSE;
   if (!strcmp(ev->key, "Escape"))
     {
        _compose_seq_reset(en);
        // dead keys here. Escape for now (should emit these)
        _edje_emit(ed, "entry,key,escape", rp->part->name);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Up") ||
            (!strcmp(ev->key, "KP_Up") && !ev->string))
     {
        _compose_seq_reset(en);
        if (multiline)
          {
             if (en->have_selection &&
                 (evas_textblock_cursor_pos_get(en->sel_start) != evas_textblock_cursor_pos_get(en->sel_end)))
               ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             if (en->select_allow)
               {
                  if (shift)
                    {
                       _sel_start(en->cursor, rp->object, en);
                       ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                    }
                  else if (en->have_selection)
                    {
                       if (evas_textblock_cursor_compare(en->sel_start, en->sel_end) < 0)
                         evas_textblock_cursor_copy(en->sel_start, en->cursor);
                       else
                         evas_textblock_cursor_copy(en->sel_end, en->cursor);
                       _sel_clear(ed, en->cursor, rp->object, en);
                    }
               }
             if (_curs_up(en->cursor, rp->object, en))
               ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             if (en->select_allow)
               {
                  if (shift) _sel_extend(ed, en->cursor, rp->object, en);
                  else _sel_clear(ed, en->cursor, rp->object, en);
               }
          }
        _edje_emit(ed, "entry,key,up", rp->part->name);
        _edje_emit(ed, "cursor,changed,manual", rp->part->name);
     }
   else if (!strcmp(ev->key, "Down") ||
            (!strcmp(ev->key, "KP_Down") && !ev->string))
     {
        _compose_seq_reset(en);
        if (multiline)
          {
             if (en->have_selection &&
                 (evas_textblock_cursor_pos_get(en->sel_start) != evas_textblock_cursor_pos_get(en->sel_end)))
               ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             if (en->select_allow)
               {
                  if (shift)
                    {
                       _sel_start(en->cursor, rp->object, en);
                       ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                    }
                  else if (en->have_selection)
                    {
                       if (evas_textblock_cursor_compare(en->sel_start, en->sel_end) < 0)
                         evas_textblock_cursor_copy(en->sel_end, en->cursor);
                       else
                         evas_textblock_cursor_copy(en->sel_start, en->cursor);
                       _sel_clear(ed, en->cursor, rp->object, en);
                    }
               }
             if (_curs_down(en->cursor, rp->object, en))
               ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             if (en->select_allow)
               {
                  if (shift) _sel_extend(ed, en->cursor, rp->object, en);
                  else _sel_clear(ed, en->cursor, rp->object, en);
               }
          }
        _edje_emit(ed, "entry,key,down", rp->part->name);
        _edje_emit(ed, "cursor,changed,manual", rp->part->name);
     }
   else if (!strcmp(ev->key, "Left") ||
            (!strcmp(ev->key, "KP_Left") && !ev->string))
     {
        _compose_seq_reset(en);
        if (en->have_selection &&
            (evas_textblock_cursor_pos_get(en->sel_start) != evas_textblock_cursor_pos_get(en->sel_end)))
          ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        if (en->select_allow)
          {
             if (shift)
               {
                  _sel_start(en->cursor, rp->object, en);
                  ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
               }
             else
               {
                  if (en->have_selection)
                    {
                       if (evas_textblock_cursor_compare(en->sel_start, en->sel_end) < 0)
                         evas_textblock_cursor_copy(en->sel_start, en->cursor);
                       else
                         evas_textblock_cursor_copy(en->sel_end, en->cursor);
                       _sel_clear(ed, en->cursor, rp->object, en);
                    }
               }
          }
        if (evas_textblock_cursor_cluster_prev(en->cursor))
          ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
#if defined(__APPLE__) && defined(__MACH__)
        if (altgr) evas_textblock_cursor_word_start(en->cursor);
#else
        /* If control is pressed, go to the start of the word */
        if (control) evas_textblock_cursor_word_start(en->cursor);
#endif
        if (en->select_allow)
          {
             if (shift) _sel_extend(ed, en->cursor, rp->object, en);
             else _sel_clear(ed, en->cursor, rp->object, en);
          }
        _edje_emit(ed, "entry,key,left", rp->part->name);
        _edje_emit(ed, "cursor,changed,manual", rp->part->name);
     }
   else if (!strcmp(ev->key, "Right") ||
            (!strcmp(ev->key, "KP_Right") && !ev->string))
     {
        _compose_seq_reset(en);
        if (en->have_selection &&
            (evas_textblock_cursor_pos_get(en->sel_start) != evas_textblock_cursor_pos_get(en->sel_end)))
          ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        if (en->select_allow)
          {
             if (shift)
               {
                  _sel_start(en->cursor, rp->object, en);
                  ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
               }
             else
               {
                  if (en->have_selection)
                    {
                       if (evas_textblock_cursor_compare(en->sel_start, en->sel_end) < 0)
                         evas_textblock_cursor_copy(en->sel_end, en->cursor);
                       else
                         evas_textblock_cursor_copy(en->sel_start, en->cursor);
                       _sel_clear(ed, en->cursor, rp->object, en);
                    }
               }
          }
        /* If control is pressed, go to the end of the word */
#if defined(__APPLE__) && defined(__MACH__)
        if (altgr) evas_textblock_cursor_word_end(en->cursor);
#else
        if (control) evas_textblock_cursor_word_end(en->cursor);
#endif
        if (evas_textblock_cursor_cluster_next(en->cursor))
          ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        if (en->select_allow)
          {
             if (shift) _sel_extend(ed, en->cursor, rp->object, en);
             else _sel_clear(ed, en->cursor, rp->object, en);
          }
        _edje_emit(ed, "entry,key,right", rp->part->name);
        _edje_emit(ed, "cursor,changed,manual", rp->part->name);
     }
   else if (!strcmp(ev->key, "BackSpace"))
     {
        _compose_seq_reset(en);
        if (control && !en->have_selection)
          {
             // del to start of previous word
             _sel_start(en->cursor, rp->object, en);

             evas_textblock_cursor_cluster_prev(en->cursor);
             evas_textblock_cursor_word_start(en->cursor);

             _sel_preextend(ed, en->cursor, rp->object, en);

             _range_del_emit(ed, en->cursor, rp->object, en);
          }
        else if ((alt) && (shift))
          {
             // undo last action
          }
        else
          {
             if (en->have_selection)
               {
                  _range_del_emit(ed, en->cursor, rp->object, en);
               }
             else
               {
                  if (evas_textblock_cursor_char_prev(en->cursor))
                    {
                       _delete_emit(ed, en->cursor, en, old_cur_pos, EINA_TRUE);
                    }
               }
          }
        _sel_clear(ed, en->cursor, rp->object, en);
        _anchors_get(en->cursor, rp->object, en);
        _edje_emit(ed, "entry,key,backspace", rp->part->name);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Delete") ||
            (!strcmp(ev->key, "KP_Delete") && !ev->string))
     {
        _compose_seq_reset(en);
        if (control)
          {
             // del to end of next word
             _sel_start(en->cursor, rp->object, en);

             evas_textblock_cursor_word_end(en->cursor);
             evas_textblock_cursor_cluster_next(en->cursor);

             _sel_extend(ed, en->cursor, rp->object, en);

             _range_del_emit(ed, en->cursor, rp->object, en);
          }
        else if (shift)
          {
             // cut
          }
        else
          {
             if (en->have_selection)
               {
                  _range_del_emit(ed, en->cursor, rp->object, en);
               }
             else
               {
                  _delete_emit(ed, en->cursor, en, old_cur_pos, EINA_FALSE);
               }
          }
        _sel_clear(ed, en->cursor, rp->object, en);
        _anchors_get(en->cursor, rp->object, en);
        _edje_emit(ed, "entry,key,delete", rp->part->name);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((!alt) &&
            (!strcmp(ev->key, "Home") ||
             ((!strcmp(ev->key, "KP_Home")) && !ev->string)))
     {
        _compose_seq_reset(en);
        if (en->select_allow)
          {
             if (shift) _sel_start(en->cursor, rp->object, en);
             else _sel_clear(ed, en->cursor, rp->object, en);
          }
        if ((control) && (multiline))
          _curs_start(en->cursor, rp->object, en);
        else
          _curs_lin_start(en->cursor, rp->object, en);
        if (en->select_allow)
          {
             if (shift) _sel_extend(ed, en->cursor, rp->object, en);
          }
        _edje_emit(ed, "entry,key,home", rp->part->name);
        _edje_emit(ed, "cursor,changed,manual", rp->part->name);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((!alt) &&
            (!strcmp(ev->key, "End") ||
             ((!strcmp(ev->key, "KP_End")) && !ev->string)))
     {
        _compose_seq_reset(en);
        if (en->select_allow)
          {
             if (shift) _sel_start(en->cursor, rp->object, en);
             else _sel_clear(ed, en->cursor, rp->object, en);
          }
        if ((control) && (multiline))
          _curs_end(en->cursor, rp->object, en);
        else
          _curs_lin_end(en->cursor, rp->object, en);
        if (en->select_allow)
          {
             if (shift) _sel_extend(ed, en->cursor, rp->object, en);
          }
        _edje_emit(ed, "entry,key,end", rp->part->name);
        _edje_emit(ed, "cursor,changed,manual", rp->part->name);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
#if defined(__APPLE__) && defined(__MACH__)
   else if ((super) && (!shift) && (!strcmp(ev->keyname, "v")))
#else
   else if ((control) && (!shift) && (!strcmp(ev->keyname, "v")))
#endif
     {
        _compose_seq_reset(en);
        _edje_emit(ed, "entry,paste,request", rp->part->name);
        _edje_emit(ed, "entry,paste,request,3", rp->part->name);
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
             _edje_emit(ed, "entry,selection,none,request", rp->part->name);
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
          }
        else
          {
             _edje_emit(ed, "entry,selection,all,request", rp->part->name);
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
        _edje_emit(ed, "entry,copy,notify", rp->part->name);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
#if defined(__APPLE__) && defined(__MACH__)
   else if ((super) && (!shift) && ((!strcmp(ev->keyname, "x") || (!strcmp(ev->keyname, "m")))))
#else
   else if ((control) && (!shift) && ((!strcmp(ev->keyname, "x") || (!strcmp(ev->keyname, "m")))))
#endif
     {
        _compose_seq_reset(en);
        _edje_emit(ed, "entry,cut,notify", rp->part->name);
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
             _edje_emit(ed, "entry,redo,request", rp->part->name);
          }
        else
          {
             // undo
             _edje_emit(ed, "entry,undo,request", rp->part->name);
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
        _edje_emit(ed, "entry,redo,request", rp->part->name);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   //else if ((control) && (!shift) && (!strcmp(ev->key, "w")))
   //{
   //_compose_seq_reset(en);
   //_sel_clear(ed, en->cursor, rp->object, en);
   //select current word?
   //ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   //}
   else if (!strcmp(ev->key, "Tab"))
     {
        _compose_seq_reset(en);
        if (multiline)
          {
             if (shift)
               {
                  // remove a tab
               }
             else
               {
                  Edje_Entry_Change_Info *info = calloc(1, sizeof(*info));
                  if (!info)
                    {
                       ERR("Running very low on memory");
                    }
                  else
                    {
                       info->insert = EINA_TRUE;
                       info->change.insert.plain_length = 1;

                       if (en->have_selection)
                         {
                            _range_del_emit(ed, en->cursor, rp->object, en);
                            info->merge = EINA_TRUE;
                         }
                       info->change.insert.pos =
                         evas_textblock_cursor_pos_get(en->cursor);
                       info->change.insert.content = eina_stringshare_add("<tab/>");
                       _text_filter_format_prepend(ed, en, en->cursor, "tab");
                       _anchors_get(en->cursor, rp->object, en);
                       _edje_emit(ed, "entry,changed", rp->part->name);
                       _edje_emit_full(ed, "entry,changed,user", rp->part->name,
                                       info, _free_entry_change_info);
                    }
               }
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
          }
        _edje_emit(ed, "entry,key,tab", rp->part->name);
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
        if (en->select_allow)
          {
             if (shift) _sel_start(en->cursor, rp->object, en);
             else _sel_clear(ed, en->cursor, rp->object, en);
          }
        if (!_curs_jump_line_by(en->cursor, rp->object, en, -10))
          {
             evas_textblock_cursor_line_set(en->cursor, 0);
             _curs_lin_start(en->cursor, rp->object, en);
          }
        if (en->select_allow)
          {
             if (shift) _sel_extend(ed, en->cursor, rp->object, en);
             else _sel_clear(ed, en->cursor, rp->object, en);
          }
        _edje_emit(ed, "entry,key,pgup", rp->part->name);
        _edje_emit(ed, "cursor,changed,manual", rp->part->name);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (!strcmp(ev->key, "Next") ||
            (!strcmp(ev->key, "KP_Next") && !ev->string))
     {
        _compose_seq_reset(en);
        if (en->select_allow)
          {
             if (shift) _sel_start(en->cursor, rp->object, en);
             else _sel_clear(ed, en->cursor, rp->object, en);
          }
        if (!_curs_jump_line_by(en->cursor, rp->object, en, 10))
          {
             int last = _curs_line_last_get(en->cursor, rp->object, en);
             evas_textblock_cursor_line_set(en->cursor, last);
             _curs_lin_end(en->cursor, rp->object, en);
          }
        if (en->select_allow)
          {
             if (shift) _sel_extend(ed, en->cursor, rp->object, en);
             else _sel_clear(ed, en->cursor, rp->object, en);
          }
        _edje_emit(ed, "entry,key,pgdn", rp->part->name);
        _edje_emit(ed, "cursor,changed,manual", rp->part->name);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if ((!strcmp(ev->key, "Return")) || (!strcmp(ev->key, "KP_Enter")))
     {
        _compose_seq_reset(en);
        if (multiline)
          {
             Edje_Entry_Change_Info *info = calloc(1, sizeof(*info));
             if (!info)
               {
                  ERR("Running very low on memory");
               }
             else
               {
                  info->insert = EINA_TRUE;
                  info->change.insert.plain_length = 1;
                  if (en->have_selection)
                    {
                       _range_del_emit(ed, en->cursor, rp->object, en);
                       info->merge = EINA_TRUE;
                    }

                  info->change.insert.pos =
                    evas_textblock_cursor_pos_get(en->cursor);
                  if (shift ||
                      evas_object_textblock_legacy_newline_get(rp->object))
                    {
                       _text_filter_format_prepend(ed, en, en->cursor, "br");
                       info->change.insert.content = eina_stringshare_add("<br/>");
                    }
                  else
                    {
                       _text_filter_format_prepend(ed, en, en->cursor, "ps");
                       info->change.insert.content = eina_stringshare_add("<ps/>");
                    }
                  _anchors_get(en->cursor, rp->object, en);
                  _edje_emit(ed, "entry,changed", rp->part->name);
                  _edje_emit_full(ed, "entry,changed,user", rp->part->name,
                                  info, _free_entry_change_info);
                  _edje_emit(ed, "cursor,changed", rp->part->name);
                  cursor_changed = EINA_TRUE;
               }
          }
        _edje_emit(ed, "entry,key,enter", rp->part->name);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else
     {
        char *compres = NULL, *string = (char *)ev->string;
        Eina_Bool free_string = EINA_FALSE;
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
                  if (ev->string && (!ev->string[1]) &&
                      ((ev->string[0] < 0x20) || (ev->string[0] == 0x7f)))
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
             Edje_Entry_Change_Info *info = NULL;
// if PASSWORD_SHOW_LAST mode, appending text with password=off tag
             if ((rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD) &&
                 _edje_password_show_last)
               {
                  _edje_entry_hide_visible_password(ed, en->rp);
                  info = _text_filter_text_prepend(ed, en, en->cursor, string,
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
                             _password_timer_cb, en);
                    }
               }
             else
               info = _text_filter_text_prepend(ed, en, en->cursor, string,
                                                NULL, NULL,
                                                EINA_TRUE, EINA_TRUE);
             _anchors_get(en->cursor, rp->object, en);
             if (info)
               {
                  _edje_emit(ed, "entry,changed", rp->part->name);
                  _edje_emit_full(ed, "entry,changed,user", rp->part->name,
                                  info, _free_entry_change_info);
                  _edje_emit(ed, "cursor,changed", rp->part->name);
                  cursor_changed = EINA_TRUE;
                  ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
               }
             if (free_string) free(string);
          }
     }
end:
   if (!cursor_changed &&
       (old_cur_pos != evas_textblock_cursor_pos_get(en->cursor)))
     _edje_emit(ed, "cursor,changed", rp->part->name);

   _edje_entry_imf_cursor_info_set(en);
   _edje_entry_real_part_configure(ed, rp);
}

static void
_edje_key_up_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Up *ev = event_info;
   Efl_Input_Device *seat;
   Edje *ed = data;
   Edje_Real_Part *rp;
   Entry *en;

   seat = efl_input_device_seat_get(ev->dev);
   rp = _edje_focused_part_get(ed, _edje_seat_name_get(ed, seat));
   if (!rp) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;

   _edje_emit(ed, "entry,keyup", rp->part->name);
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        Ecore_IMF_Event_Key_Up ecore_ev;

        ecore_imf_evas_event_key_up_wrap(ev, &ecore_ev);
        if (ecore_imf_context_filter_event(en->imf_context,
                                           ECORE_IMF_EVENT_KEY_UP,
                                           (Ecore_IMF_Event *)&ecore_ev))
          return;
     }
#else
   (void)event_info;
#endif
}

static Evas_Textblock_Cursor *
_edje_cursor_cluster_coord_set(Edje_Real_Part *rp, Evas_Coord canvasx, Evas_Coord canvasy, Evas_Coord *cx, Evas_Coord *cy)
{
   Entry *en;
   Evas_Coord x, y, lh = 0, cly = 0;
   Evas_Textblock_Cursor *line_cur;
   Evas_Textblock_Cursor *tc;

   en = rp->typedata.text->entry_data;
   tc = evas_object_textblock_cursor_new(rp->object);
   evas_textblock_cursor_copy(en->cursor, tc);
   evas_object_geometry_get(rp->object, &x, &y, NULL, NULL);
   *cx = canvasx - x;
   *cy = canvasy - y;

   line_cur = evas_object_textblock_cursor_new(rp->object);
   evas_textblock_cursor_paragraph_last(line_cur);
   evas_textblock_cursor_line_geometry_get(line_cur, NULL, &cly, NULL, &lh);
   /* Consider a threshold of half the line height */
   if (*cy > (cly + lh) && *cy < (cly + lh + lh / 2))
     {
        *cy = cly + lh - 1; // Make it inside Textblock
     }
   evas_textblock_cursor_paragraph_first(line_cur);
   evas_textblock_cursor_line_geometry_get(line_cur, NULL, &cly, NULL, NULL);

   if (*cy < cly && *cy > (cly - lh / 2))
     {
        *cy = cly;
     }
   evas_textblock_cursor_free(line_cur);
   /* No need to check return value if not able to set the char coord Textblock
    * will take care */
   evas_textblock_cursor_cluster_coord_set(en->cursor, *cx, *cy);

   return tc;
}

static void
_edje_part_move_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Edje_Real_Part *rp = data;
   Entry *en;
   if (!rp) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   _edje_entry_imf_cursor_location_set(en);
}

static void
_edje_part_mouse_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Coord cx, cy;
   Edje_Real_Part *rp = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Entry *en;
   //   Eina_Bool multiline;
   Evas_Textblock_Cursor *tc = NULL;
   Eina_Bool dosel = EINA_FALSE;
   Eina_Bool shift;

   if ((!rp) || (!ev)) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;
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

   _edje_entry_imf_context_reset(rp);

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
                  tc = evas_object_textblock_cursor_new(rp->object);
                  evas_textblock_cursor_copy(en->cursor, tc);
                  if (evas_textblock_cursor_compare(en->cursor, en->sel_start) < 0)
                    evas_textblock_cursor_line_char_first(en->cursor);
                  else
                    evas_textblock_cursor_line_char_last(en->cursor);
                  _sel_extend(en->ed, en->cursor, rp->object, en);
               }
             else
               {
                  en->have_selection = EINA_FALSE;
                  en->selecting = EINA_FALSE;
                  _sel_clear(en->ed, en->cursor, rp->object, en);
                  tc = evas_object_textblock_cursor_new(rp->object);
                  evas_textblock_cursor_copy(en->cursor, tc);
                  evas_textblock_cursor_line_char_first(en->cursor);
                  _sel_start(en->cursor, rp->object, en);
                  evas_textblock_cursor_line_char_last(en->cursor);
                  _sel_extend(en->ed, en->cursor, rp->object, en);
               }
             goto end;
          }
        else if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
          {
             if (shift)
               {
                  tc = evas_object_textblock_cursor_new(rp->object);
                  evas_textblock_cursor_copy(en->cursor, tc);
                  if (evas_textblock_cursor_compare(en->cursor, en->sel_start) < 0)
                    evas_textblock_cursor_word_start(en->cursor);
                  else
                    {
                       evas_textblock_cursor_word_end(en->cursor);
                       evas_textblock_cursor_cluster_next(en->cursor);
                    }
                  _sel_extend(en->ed, en->cursor, rp->object, en);
               }
             else
               {
                  en->have_selection = EINA_FALSE;
                  en->selecting = EINA_FALSE;
                  _sel_clear(en->ed, en->cursor, rp->object, en);
                  tc = evas_object_textblock_cursor_new(rp->object);
                  evas_textblock_cursor_copy(en->cursor, tc);
                  evas_textblock_cursor_word_start(en->cursor);
                  _sel_start(en->cursor, rp->object, en);
                  evas_textblock_cursor_word_end(en->cursor);
                  evas_textblock_cursor_cluster_next(en->cursor);
                  _sel_extend(en->ed, en->cursor, rp->object, en);
               }
             goto end;
          }
     }
   tc = _edje_cursor_cluster_coord_set(rp, ev->canvas.x, ev->canvas.y, &cx, &cy);

   if (dosel)
     {
        if ((en->have_selection) &&
            (rp->part->select_mode == EDJE_ENTRY_SELECTION_MODE_EXPLICIT))
          {
             if (shift)
               _sel_extend(en->ed, en->cursor, rp->object, en);
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
        else
          {
             if (shift)
               {
                  _sel_extend(en->ed, en->cursor, rp->object, en);
               }
             else
               {
                  en->selecting = EINA_TRUE;
                  _sel_clear(en->ed, en->cursor, rp->object, en);
                  _sel_start(en->cursor, rp->object, en);
               }
          }
     }
end:
   if (evas_textblock_cursor_compare(tc, en->cursor))
     {
        _edje_emit(en->ed, "cursor,changed", rp->part->name);
        _edje_emit(en->ed, "cursor,changed,manual", rp->part->name);
     }
   evas_textblock_cursor_free(tc);

   _edje_entry_real_part_configure(en->ed, rp);
   if (ev->button == 2)
     {
        _edje_emit(en->ed, "entry,paste,request", rp->part->name);
        _edje_emit(en->ed, "entry,paste,request,1", rp->part->name);
     }
}

static void
_edje_part_mouse_up_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Coord cx, cy;
   Edje_Real_Part *rp = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Entry *en;
   Evas_Textblock_Cursor *tc;

   if ((!ev) || (ev->button != 1)) return;
   if (!rp) return;
   if (ev->flags & EVAS_BUTTON_TRIPLE_CLICK) return;
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;

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
   tc = _edje_cursor_cluster_coord_set(rp, ev->canvas.x, ev->canvas.y, &cx, &cy);

   if (en->select_allow)
     {
        if (rp->part->select_mode == EDJE_ENTRY_SELECTION_MODE_EXPLICIT)
          {
             if (en->had_sel)
               {
                  if (en->select_mod_end)
                    _sel_extend(en->ed, en->cursor, rp->object, en);
                  else if (en->select_mod_start)
                    _sel_preextend(en->ed, en->cursor, rp->object, en);
               }
             else
               _sel_extend(en->ed, en->cursor, rp->object, en);
             //evas_textblock_cursor_copy(en->cursor, en->sel_end);
          }
        else
          {
             evas_textblock_cursor_copy(en->cursor, en->sel_end);
          }
     }
   if (en->selecting)
     {
        if (en->have_selection)
          en->had_sel = EINA_TRUE;
        en->selecting = EINA_FALSE;
     }
   if (evas_textblock_cursor_compare(tc, en->cursor))
     {
        _edje_emit(en->ed, "cursor,changed", rp->part->name);
        _edje_emit(en->ed, "cursor,changed,manual", rp->part->name);
     }

   _edje_entry_imf_cursor_info_set(en);

   evas_textblock_cursor_free(tc);

   _edje_entry_real_part_configure(en->ed, rp);
}

static void
_edje_part_mouse_move_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Coord cx, cy;
   Edje_Real_Part *rp = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Entry *en;
   Evas_Coord x, y, w, h;
   Evas_Textblock_Cursor *tc;

   if ((!rp) || (!ev)) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;

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
        tc = evas_object_textblock_cursor_new(rp->object);
        evas_textblock_cursor_copy(en->cursor, tc);
        evas_object_geometry_get(rp->object, &x, &y, &w, &h);
        cx = ev->cur.canvas.x - x;
        cy = ev->cur.canvas.y - y;
        if (!evas_textblock_cursor_cluster_coord_set(en->cursor, cx, cy))
          {
             Evas_Coord lx, ly, lw, lh;

             if (evas_textblock_cursor_line_coord_set(en->cursor, cy) < 0)
               {
                  if (rp->part->multiline)
                    _curs_end(en->cursor, rp->object, en);
                  else
                    {
                       evas_textblock_cursor_paragraph_first(en->cursor);
                       evas_textblock_cursor_line_geometry_get(en->cursor, &lx, &ly, &lw, &lh);
                       if (!evas_textblock_cursor_cluster_coord_set(en->cursor, cx, ly + (lh / 2)))
                         _curs_end(en->cursor, rp->object, en);
                    }
               }
             else
               {
                  evas_textblock_cursor_line_geometry_get(en->cursor, &lx, &ly, &lw, &lh);
                  if (cx <= lx)
                    _curs_lin_start(en->cursor, rp->object, en);
                  else
                    _curs_lin_end(en->cursor, rp->object, en);
               }
          }
        if (en->select_allow)
          {
             if (rp->part->select_mode == EDJE_ENTRY_SELECTION_MODE_EXPLICIT)
               {
                  if (en->had_sel)
                    {
                       if (en->select_mod_end)
                         _sel_extend(en->ed, en->cursor, rp->object, en);
                       else if (en->select_mod_start)
                         _sel_preextend(en->ed, en->cursor, rp->object, en);
                    }
                  else
                    _sel_extend(en->ed, en->cursor, rp->object, en);
               }
             else
               {
                  _sel_extend(en->ed, en->cursor, rp->object, en);
               }

             if (evas_textblock_cursor_compare(en->sel_start, en->sel_end) != 0)
               _sel_enable(en->ed, en->cursor, rp->object, en);
             if (en->have_selection)
               _sel_update(en->ed, en->cursor, rp->object, en);
          }
        if (evas_textblock_cursor_compare(tc, en->cursor))
          {
             _edje_emit(en->ed, "cursor,changed", rp->part->name);
             _edje_emit(en->ed, "cursor,changed,manual", rp->part->name);
          }
        evas_textblock_cursor_free(tc);

        _edje_entry_real_part_configure(en->ed, rp);
     }
}

static void
_canvas_viewport_resize_cb(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Edje_Real_Part *rp = data;
   Entry *en;
   if ((!rp)) return;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
                       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;

   _anchors_need_update(rp);
}

static void
_evas_focus_in_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Focus *ev = event->info;
   Evas *e = event->object;
   Efl_Input_Device *seat;
   Edje *ed = data;

   if (!ed) return;

   seat = efl_input_device_get(ev);
   if (evas_canvas_seat_focus_get(e, seat) == ed->obj)
     {
        _edje_focus_in(data, seat);
     }
}

static void
_evas_focus_out_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Focus *ev = event->info;
   Evas *e = event->object;
   Efl_Input_Device *seat;
   Edje *ed = data;

   if (!ed) return;

   seat = efl_input_device_get(ev);
   if (evas_canvas_seat_focus_get(e, seat) == ed->obj)
     {
        _edje_focus_out(data, seat);
     }
}

/***************************************************************/
void
_edje_entry_init(Edje *ed)
{
   if (!ed->has_entries)
     return;
   if (ed->entries_inited)
     return;
   ed->entries_inited = EINA_TRUE;

   efl_event_callback_add(ed->obj, EFL_EVENT_FOCUS_IN, _edje_focus_in_cb, ed);
   efl_event_callback_add(ed->obj, EFL_EVENT_FOCUS_OUT, _edje_focus_out_cb, ed);
   evas_object_event_callback_add(ed->obj, EVAS_CALLBACK_KEY_DOWN,
                                  _edje_key_down_cb, ed);
   evas_object_event_callback_add(ed->obj, EVAS_CALLBACK_KEY_UP,
                                  _edje_key_up_cb, ed);
   efl_event_callback_add(ed->base.evas, EFL_CANVAS_SCENE_EVENT_FOCUS_IN,
                          _evas_focus_in_cb, ed);
   efl_event_callback_add(ed->base.evas, EFL_CANVAS_SCENE_EVENT_FOCUS_OUT,
                          _evas_focus_out_cb, ed);
}

void
_edje_entry_shutdown(Edje *ed)
{
   if ((!ed) || (!ed->has_entries))
     return;
   if (!ed->entries_inited)
     return;
   ed->entries_inited = EINA_FALSE;

   efl_event_callback_del(ed->obj, EFL_EVENT_FOCUS_IN, _edje_focus_in_cb, ed);
   efl_event_callback_del(ed->obj, EFL_EVENT_FOCUS_OUT, _edje_focus_out_cb, ed);
   evas_object_event_callback_del(ed->obj, EVAS_CALLBACK_KEY_DOWN,
                                  _edje_key_down_cb);
   evas_object_event_callback_del(ed->obj, EVAS_CALLBACK_KEY_UP,
                                  _edje_key_up_cb);
   efl_event_callback_del(ed->base.evas, EFL_CANVAS_SCENE_EVENT_FOCUS_IN,
                          _evas_focus_in_cb, ed);
   efl_event_callback_del(ed->base.evas, EFL_CANVAS_SCENE_EVENT_FOCUS_OUT,
                          _evas_focus_out_cb, ed);
}

void
_edje_entry_real_part_init(Edje *ed, Edje_Real_Part *rp)
{
   Entry *en;
#ifdef HAVE_ECORE_IMF
   const char *ctx_id;
   const Ecore_IMF_Context_Info *ctx_info;
#endif

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = calloc(1, sizeof(Entry));
   if (!en)
     {
        ERR("Running very low on memory");
        return;
     }
   rp->typedata.text->entry_data = en;
   en->rp = rp;
   en->ed = ed;

   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOVE, _edje_part_move_cb, rp);

   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOUSE_DOWN, _edje_part_mouse_down_cb, rp);
   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOUSE_UP, _edje_part_mouse_up_cb, rp);
   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOUSE_MOVE, _edje_part_mouse_move_cb, rp);
   evas_event_callback_add(ed->base.evas, EVAS_CALLBACK_CANVAS_VIEWPORT_RESIZE, _canvas_viewport_resize_cb, rp);

   if (rp->part->select_mode == EDJE_ENTRY_SELECTION_MODE_DEFAULT)
     en->select_allow = EINA_TRUE;

   if (rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD)
     {
        Edje_Part_Description_Text *txt;

        txt = (Edje_Part_Description_Text *)rp->chosen_description;

        en->select_allow = EINA_FALSE;
        if (txt && edje_string_get(&txt->text.repch))
          evas_object_textblock_replace_char_set(rp->object, edje_string_get(&txt->text.repch));
        else
          evas_object_textblock_replace_char_set(rp->object, "*");
     }

   if (rp->part->source3)
     {
        en->cursor_bg = edje_object_add(ed->base.evas);
        edje_object_file_set(en->cursor_bg, ed->path, rp->part->source3);
        evas_object_smart_member_add(en->cursor_bg, ed->obj);
        evas_object_stack_below(en->cursor_bg, rp->object);
        evas_object_clip_set(en->cursor_bg, evas_object_clip_get(rp->object));
        evas_object_pass_events_set(en->cursor_bg, EINA_TRUE);
        _edje_subobj_register(ed, en->cursor_bg);
     }
   if (rp->part->source4)
     {
        en->cursor_fg = edje_object_add(ed->base.evas);
        edje_object_file_set(en->cursor_fg, ed->path, rp->part->source4);
        evas_object_smart_member_add(en->cursor_fg, ed->obj);
        evas_object_stack_above(en->cursor_fg, rp->object);
        evas_object_clip_set(en->cursor_fg, evas_object_clip_get(rp->object));
        evas_object_pass_events_set(en->cursor_fg, EINA_TRUE);
        _edje_subobj_register(ed, en->cursor_fg);

        /* A proxy to the main cursor. */
        if (rp->part->cursor_mode == EDJE_ENTRY_CURSOR_MODE_BEFORE)
          {
             en->cursor_fg2 = edje_object_add(ed->base.evas);
             edje_object_file_set(en->cursor_fg2, ed->path, rp->part->source4);
             evas_object_smart_member_add(en->cursor_fg2, ed->obj);
             evas_object_stack_above(en->cursor_fg2, rp->object);
             evas_object_clip_set(en->cursor_fg2, evas_object_clip_get(rp->object));
             evas_object_pass_events_set(en->cursor_fg2, EINA_TRUE);
             _edje_subobj_register(ed, en->cursor_fg2);
          }
     }

   evas_object_textblock_legacy_newline_set(rp->object, EINA_TRUE);

   if (rp->part->entry_mode >= EDJE_ENTRY_EDIT_MODE_EDITABLE)
     {
        evas_object_show(en->cursor_bg);
        evas_object_show(en->cursor_fg);
        evas_object_show(en->cursor_fg2);
        en->input_panel_enable = EINA_TRUE;

#ifdef HAVE_ECORE_IMF
        _edje_need_imf();

        en->commit_cancel = EINA_FALSE;

        edje_object_signal_callback_add(ed->obj, "focus,part,in,*",
                                        rp->part->name,
                                        _edje_entry_focus_in_cb, rp);
        edje_object_signal_callback_add(ed->obj, "focus,part,out,*",
                                        rp->part->name,
                                        _edje_entry_focus_out_cb, rp);

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
            (ecore_evas_ecore_evas_get(ed->base.evas)));
        ecore_imf_context_client_canvas_set(en->imf_context, ed->base.evas);

        ecore_imf_context_retrieve_surrounding_callback_set(en->imf_context,
                                                            _edje_entry_imf_retrieve_surrounding_cb, ed);
        ecore_imf_context_retrieve_selection_callback_set(en->imf_context, _edje_entry_imf_retrieve_selection_cb, ed);
        ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_COMMIT, _edje_entry_imf_event_commit_cb, ed);
        ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_DELETE_SURROUNDING, _edje_entry_imf_event_delete_surrounding_cb, ed);
        ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, _edje_entry_imf_event_preedit_changed_cb, ed);
        ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_SELECTION_SET, _edje_entry_imf_event_selection_set_cb, ed);
        ecore_imf_context_input_mode_set(en->imf_context,
                                         rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD ?
                                         ECORE_IMF_INPUT_MODE_INVISIBLE : ECORE_IMF_INPUT_MODE_FULL);

        if (rp->part->multiline)
          ecore_imf_context_input_hint_set(en->imf_context,
                                           ecore_imf_context_input_hint_get(en->imf_context) | ECORE_IMF_INPUT_HINT_MULTILINE);

        if (rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD)
          ecore_imf_context_input_panel_language_set(en->imf_context, ECORE_IMF_INPUT_PANEL_LANG_ALPHABET);
#endif
     }
#ifdef HAVE_ECORE_IMF
done:
#endif
   en->cursor = (Evas_Textblock_Cursor *)evas_object_textblock_cursor_get(rp->object);
}

void
_edje_entry_real_part_shutdown(Edje *ed, Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   rp->typedata.text->entry_data = NULL;
   _sel_clear(ed, en->cursor, rp->object, en);
   _anchors_clear(en->cursor, rp->object, en);
   _unused_item_objs_free(en);
#ifdef HAVE_ECORE_IMF
   _preedit_clear(en);
#endif
   evas_object_del(en->cursor_bg);
   evas_object_del(en->cursor_fg);
   evas_object_del(en->cursor_fg2);

   if (en->cursor_user)
     evas_textblock_cursor_free(en->cursor_user);

   if (en->cursor_user_extra)
     evas_textblock_cursor_free(en->cursor_user_extra);

   if (en->pw_timer)
     {
        ecore_timer_del(en->pw_timer);
        en->pw_timer = NULL;
     }

   evas_event_callback_del_full(ed->base.evas, EVAS_CALLBACK_CANVAS_VIEWPORT_RESIZE, _canvas_viewport_resize_cb, rp);

#ifdef HAVE_ECORE_IMF
   if (rp->part->entry_mode >= EDJE_ENTRY_EDIT_MODE_EDITABLE)
     {
        if (en->imf_context)
          {
             ecore_imf_context_event_callback_del(en->imf_context, ECORE_IMF_CALLBACK_COMMIT, _edje_entry_imf_event_commit_cb);
             ecore_imf_context_event_callback_del(en->imf_context, ECORE_IMF_CALLBACK_DELETE_SURROUNDING, _edje_entry_imf_event_delete_surrounding_cb);
             ecore_imf_context_event_callback_del(en->imf_context, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, _edje_entry_imf_event_preedit_changed_cb);
             ecore_imf_context_event_callback_del(en->imf_context, ECORE_IMF_CALLBACK_SELECTION_SET, _edje_entry_imf_event_selection_set_cb);

             ecore_imf_context_del(en->imf_context);
             en->imf_context = NULL;
          }

        edje_object_signal_callback_del(ed->obj, "focus,part,in,*",
                                        rp->part->name,
                                        _edje_entry_focus_in_cb);
        edje_object_signal_callback_del(ed->obj, "focus,part,out,*",
                                        rp->part->name,
                                        _edje_entry_focus_out_cb);
     }
#endif
   _compose_seq_reset(en);

   free(en);
}

void
_edje_entry_real_part_configure(Edje *ed, Edje_Real_Part *rp)
{
   Evas_Coord x, y, w, h, xx, yy, ww, hh, xx2, yy2;
   Entry *en;
   Evas_Textblock_Cursor_Type cur_type;
   Eina_Bool bidi_cursor = EINA_FALSE;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;

   _sel_update(ed, en->cursor, rp->object, en);
   _anchors_update_check(ed, rp);
   if (rp->part->entry_mode >= EDJE_ENTRY_EDIT_MODE_EDITABLE)
     {
        switch (rp->part->cursor_mode)
         {
           case EDJE_ENTRY_CURSOR_MODE_BEFORE:
              cur_type = EVAS_TEXTBLOCK_CURSOR_BEFORE;
            break;

           case EDJE_ENTRY_CURSOR_MODE_UNDER:
          /* no break for a reason */
           default:
             cur_type = EVAS_TEXTBLOCK_CURSOR_UNDER;
         }
        x = y = w = h = -1;
        xx = yy = ww = hh = -1;
        evas_object_geometry_get(rp->object, &x, &y, &w, &h);
        bidi_cursor = evas_textblock_cursor_geometry_bidi_get(en->cursor, &xx, &yy, &ww, &hh, &xx2, &yy2, NULL, NULL, cur_type);
        if (ww < 1) ww = 1;
        if (hh < 1) hh = 1;
        if (en->cursor_bg)
          {
             int bg_w = ww;

             if (rp->part->cursor_mode == EDJE_ENTRY_CURSOR_MODE_BEFORE)
               edje_object_size_min_restricted_calc(en->cursor_bg, &bg_w, NULL, ww, 0);

             evas_object_move(en->cursor_bg, x + xx, y + yy);
             evas_object_resize(en->cursor_bg, bg_w, hh);
          }
        if (en->cursor_fg)
          {
             int fg_w = ww;

             if (rp->part->cursor_mode == EDJE_ENTRY_CURSOR_MODE_BEFORE)
               edje_object_size_min_restricted_calc(en->cursor_fg, &fg_w, NULL, ww, 0);

             evas_object_move(en->cursor_fg, x + xx, y + yy);

             if (bidi_cursor)
               {
                  if (en->cursor_fg2)
                    {
                       evas_object_move(en->cursor_fg2, x + xx2, y + yy2 + (hh / 2));
                       evas_object_resize(en->cursor_fg, fg_w, hh / 2);
                       evas_object_resize(en->cursor_fg2, fg_w, hh / 2);
                       evas_object_show(en->cursor_fg2);
                    }
               }
             else
               {
                  evas_object_resize(en->cursor_fg, fg_w, hh);
                  if (en->cursor_fg2)
                    evas_object_hide(en->cursor_fg2);
               }
          }
     }
}

const char *
_edje_entry_selection_get(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return NULL;
   en = rp->typedata.text->entry_data;
   if (!en) return NULL;
   // get selection - convert to markup
   if ((!en->selection) && (en->have_selection))
     en->selection = evas_textblock_cursor_range_text_get
         (en->sel_start, en->sel_end, EVAS_TEXTBLOCK_TEXT_MARKUP);
   return en->selection;
}

const char *
_edje_entry_text_get(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return NULL;
   en = rp->typedata.text->entry_data;
   if (!en) return NULL;
   // get text - convert to markup
   return evas_object_textblock_text_markup_get(rp->object);
}

void
_edje_entry_text_markup_set(Edje_Real_Part *rp, const char *text)
{
   Entry *en;
   const char *ptext;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   ptext = evas_object_textblock_text_markup_get(rp->object);
   // some simple "do nothing if the text is the same" logic
   if (ptext == text) return;
   // if prev and cur is empty
   if (!ptext) ptext = "";
   if (!text) text = "";
   if ((!ptext[0]) && (!text[0])) return;
   // same content
   if (!strcmp(ptext, text)) return;

   _edje_entry_imf_context_reset(rp);
   // set text as markup
   _sel_clear(en->ed, en->cursor, rp->object, en);
   evas_object_textblock_text_markup_set(rp->object, text);
   _edje_entry_set_cursor_start(rp);

   _anchors_get(en->cursor, rp->object, en);
   _edje_emit(en->ed, "entry,changed", rp->part->name);
   _edje_entry_imf_cursor_info_set(en);

   _edje_entry_real_part_configure(en->ed, rp);
#if 0
   /* Don't emit cursor changed cause it didn't. It's just init to 0. */
   _edje_emit(en->ed, "cursor,changed", rp->part->name);
#endif
}

void
_edje_entry_text_markup_append(Edje_Real_Part *rp, const char *text)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   Evas_Textblock_Cursor *end_cur;
   if (!en) return;
   end_cur = evas_object_textblock_cursor_new(rp->object);
   evas_textblock_cursor_paragraph_last(end_cur);

   _text_filter_markup_prepend(en->ed, en, end_cur, text, NULL, NULL,
                               EINA_TRUE, EINA_FALSE);
   evas_textblock_cursor_free(end_cur);

   /* We are updating according to the real cursor on purpose */
   _anchors_get(en->cursor, rp->object, en);
   _edje_emit(en->ed, "entry,changed", rp->part->name);

   _edje_entry_real_part_configure(en->ed, rp);
}

void
_edje_entry_text_markup_insert(Edje_Real_Part *rp, const char *text)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   _edje_entry_imf_context_reset(rp);

   _text_filter_markup_prepend(en->ed, en, en->cursor, text, NULL, NULL,
                               EINA_TRUE, EINA_FALSE);
   _anchors_get(en->cursor, rp->object, en);
   _edje_emit(en->ed, "entry,changed", rp->part->name);
   _edje_emit(en->ed, "cursor,changed", rp->part->name);

   _edje_entry_imf_cursor_info_set(en);
   _edje_entry_real_part_configure(en->ed, rp);
}

void
_edje_entry_set_cursor_start(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   _curs_start(en->cursor, rp->object, en);

   _edje_entry_imf_cursor_info_set(en);
}

void
_edje_entry_set_cursor_end(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   _curs_end(en->cursor, rp->object, en);

   _edje_entry_imf_cursor_info_set(en);
}

void
_edje_entry_select_none(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   _sel_clear(en->ed, en->cursor, rp->object, en);
}

void
_edje_entry_select_all(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;

   _edje_entry_imf_context_reset(rp);

   _sel_clear(en->ed, en->cursor, rp->object, en);
   _curs_start(en->cursor, rp->object, en);
   _edje_entry_imf_context_reset(en->rp);
   _sel_start(en->cursor, rp->object, en);
   _curs_end(en->cursor, rp->object, en);
   _sel_extend(en->ed, en->cursor, rp->object, en);

   _edje_entry_real_part_configure(en->ed, rp);
}

void
_edje_entry_select_begin(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;

   _sel_clear(en->ed, en->cursor, rp->object, en);
   _sel_enable(en->ed, en->cursor, rp->object, en);
   _sel_start(en->cursor, rp->object, en);
   _sel_extend(en->ed, en->cursor, rp->object, en);

   _edje_entry_real_part_configure(en->ed, rp);
}

void
_edje_entry_select_extend(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   _sel_extend(en->ed, en->cursor, rp->object, en);

   _edje_entry_real_part_configure(en->ed, rp);
}

const Eina_List *
_edje_entry_anchor_geometry_get(Edje_Real_Part *rp, const char *anchor)
{
   Entry *en;
   Eina_List *l;
   Anchor *an;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return NULL;
   en = rp->typedata.text->entry_data;
   if (!en) return NULL;
   /* Update the anchors first in case entry is not inside the canvas
    * viewport */
   _anchors_need_update(rp);
   EINA_LIST_FOREACH(en->anchors, l, an)
     {
        const char *n = an->name;
        if ((an->item) || (!n)) continue;
        if (!strcmp(anchor, n))
          return an->sel;
     }
   return NULL;
}

const Eina_List *
_edje_entry_anchors_list(Edje_Real_Part *rp)
{
   Entry *en;
   Eina_List *l, *anchors = NULL;
   Anchor *an;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return NULL;
   en = rp->typedata.text->entry_data;
   if (!en) return NULL;
   /* Update the anchors first in case entry is not inside the canvas
    * viewport */
   _anchors_need_update(rp);
   if (!en->anchorlist)
     {
        EINA_LIST_FOREACH(en->anchors, l, an)
          {
             const char *n = an->name;
             if ((an->item) || (!n)) continue;
             anchors = eina_list_append(anchors, strdup(n));
          }
        en->anchorlist = anchors;
     }
   return en->anchorlist;
}

Eina_Bool
_edje_entry_item_geometry_get(Edje_Real_Part *rp, const char *item, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Entry *en;
   Eina_List *l;
   Anchor *an;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   en = rp->typedata.text->entry_data;
   if (!en) return EINA_FALSE;
   EINA_LIST_FOREACH(en->anchors, l, an)
     {
        const char *n = an->name;
        if (!an->item) continue;
        if (!n) n = "";
        if (!strcmp(item, n))
          {
             evas_textblock_cursor_format_item_geometry_get(an->start, cx, cy, cw, ch);
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

const Eina_List *
_edje_entry_items_list(Edje_Real_Part *rp)
{
   Entry *en;
   Eina_List *l, *items = NULL;
   Anchor *an;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return NULL;
   en = rp->typedata.text->entry_data;
   if (!en) return NULL;
   /* Update the anchors first in case entry is not inside the canvas
    * viewport */
   _anchors_need_update(rp);
   if (!en->itemlist)
     {
        EINA_LIST_FOREACH(en->anchors, l, an)
          {
             const char *n = an->name;
             if (!an->item) continue;
             if (!n) n = "";
             items = eina_list_append(items, strdup(n));
          }
        en->itemlist = items;
     }
   return en->itemlist;
}

void
_edje_entry_cursor_geometry_get(Edje_Real_Part *rp, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_BiDi_Direction *cdir)
{
   Evas_Coord x, y, w, h, xx, yy, ww, hh;
   Entry *en;
   Evas_Textblock_Cursor_Type cur_type;
   Evas_BiDi_Direction dir;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   switch (rp->part->cursor_mode)
     {
      case EDJE_ENTRY_CURSOR_MODE_BEFORE:
        cur_type = EVAS_TEXTBLOCK_CURSOR_BEFORE;
        break;

      case EDJE_ENTRY_CURSOR_MODE_UNDER:
      /* no break for a reason */
      default:
        cur_type = EVAS_TEXTBLOCK_CURSOR_UNDER;
     }

   x = y = w = h = -1;
   xx = yy = ww = hh = -1;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   evas_textblock_cursor_geometry_get(en->cursor, &xx, &yy, &ww, &hh, &dir, cur_type);
   if (ww < 1) ww = 1;
   if (rp->part->cursor_mode == EDJE_ENTRY_CURSOR_MODE_BEFORE)
     edje_object_size_min_restricted_calc(en->cursor_fg, &ww, NULL, ww, 0);
   if (hh < 1) hh = 1;
   if (cx) *cx = x + xx;
   if (cy) *cy = y + yy;
   if (cw) *cw = ww;
   if (ch) *ch = hh;
   if (cdir) *cdir = dir;
}

void
_edje_entry_user_insert(Edje_Real_Part *rp, const char *text)
{
   Entry *en;
   Edje_Entry_Change_Info *info;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   _edje_entry_imf_context_reset(rp);
   info = _text_filter_markup_prepend(en->ed, en, en->cursor, text, NULL, NULL,
                                      EINA_TRUE, EINA_TRUE);
   _anchors_get(en->cursor, rp->object, en);
   if (info)
     {
        _edje_emit(en->ed, "entry,changed", rp->part->name);
        _edje_emit_full(en->ed, "entry,changed,user", rp->part->name,
                        info, _free_entry_change_info);
        _edje_emit(en->ed, "cursor,changed", rp->part->name);
     }

   _edje_entry_imf_cursor_info_set(en);
   _edje_entry_real_part_configure(en->ed, rp);
}

void
_edje_entry_select_allow_set(Edje_Real_Part *rp, Eina_Bool allow)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;

   en->select_allow = allow;
}

Eina_Bool
_edje_entry_select_allow_get(const Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   en = rp->typedata.text->entry_data;
   if (!en) return EINA_FALSE;
   return en->select_allow;
}

void
_edje_entry_select_abort(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (en->selecting)
     {
        en->selecting = EINA_FALSE;

        _edje_entry_imf_context_reset(rp);
        _edje_entry_imf_cursor_info_set(en);
        _edje_entry_real_part_configure(en->ed, rp);
     }
}

void *
_edje_entry_imf_context_get(Edje_Real_Part *rp)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return NULL;
   en = rp->typedata.text->entry_data;
   if (!en) return NULL;

   return en->imf_context;
#else
   return NULL;
   (void)rp;
#endif
}

void
_edje_entry_autocapital_type_set(Edje_Real_Part *rp, Edje_Text_Autocapital_Type autocapital_type)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;

   if (rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD)
     autocapital_type = EDJE_TEXT_AUTOCAPITAL_TYPE_NONE;

   if (en->imf_context)
     ecore_imf_context_autocapital_type_set(en->imf_context, (Ecore_IMF_Autocapital_Type)autocapital_type);
#else
   (void)rp;
   (void)autocapital_type;
#endif
}

Edje_Text_Autocapital_Type
_edje_entry_autocapital_type_get(Edje_Real_Part *rp)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EDJE_TEXT_AUTOCAPITAL_TYPE_NONE;
   en = rp->typedata.text->entry_data;
   if (!en) return EDJE_TEXT_AUTOCAPITAL_TYPE_NONE;

   if (en->imf_context)
     return (Edje_Text_Autocapital_Type)ecore_imf_context_autocapital_type_get(en->imf_context);
   return EDJE_TEXT_AUTOCAPITAL_TYPE_NONE;
#else
   return EDJE_TEXT_AUTOCAPITAL_TYPE_NONE;
   (void)rp;
#endif
}

void
_edje_entry_prediction_allow_set(Edje_Real_Part *rp, Eina_Bool prediction)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   en->prediction_allow = prediction;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_prediction_allow_set(en->imf_context, prediction);
#endif
}

Eina_Bool
_edje_entry_prediction_allow_get(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   en = rp->typedata.text->entry_data;
   if (!en) return EINA_FALSE;
   return en->prediction_allow;
}

void
_edje_entry_input_hint_set(Edje_Real_Part *rp, Edje_Input_Hints input_hints)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_hint_set(en->imf_context, (Ecore_IMF_Input_Hints)input_hints);
#else
   (void)input_hints;
#endif
}

Edje_Input_Hints
_edje_entry_input_hint_get(const Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EDJE_INPUT_HINT_NONE;
   en = rp->typedata.text->entry_data;
   if (!en) return EDJE_INPUT_HINT_NONE;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return (Edje_Input_Hints)ecore_imf_context_input_hint_get(en->imf_context);
#endif

   return EDJE_INPUT_HINT_NONE;
}

void
_edje_entry_input_panel_enabled_set(Edje_Real_Part *rp, Eina_Bool enabled)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   en->input_panel_enable = enabled;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_enabled_set(en->imf_context, enabled);
#endif
}

Eina_Bool
_edje_entry_input_panel_enabled_get(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   en = rp->typedata.text->entry_data;
   if (!en) return EINA_FALSE;
   return en->input_panel_enable;
}

void
_edje_entry_input_panel_show(Edje_Real_Part *rp)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (en->imf_context)
     ecore_imf_context_input_panel_show(en->imf_context);
#else
   (void)rp;
#endif
}

void
_edje_entry_input_panel_hide(Edje_Real_Part *rp)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (en->imf_context)
     ecore_imf_context_input_panel_hide(en->imf_context);
#else
   (void)rp;
#endif
}

void
_edje_entry_input_panel_language_set(Edje_Real_Part *rp, Edje_Input_Panel_Lang lang)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   en->input_panel_lang = lang;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_language_set(en->imf_context, (Ecore_IMF_Input_Panel_Lang)lang);
#endif
}

Edje_Input_Panel_Lang
_edje_entry_input_panel_language_get(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EDJE_INPUT_PANEL_LANG_AUTOMATIC;
   en = rp->typedata.text->entry_data;
   if (!en) return EDJE_INPUT_PANEL_LANG_AUTOMATIC;
   return en->input_panel_lang;
}

void
_edje_entry_input_panel_imdata_set(Edje_Real_Part *rp, const void *data, int len)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (en->imf_context)
     ecore_imf_context_input_panel_imdata_set(en->imf_context, data, len);
#else
   (void)rp;
   (void)data;
   (void)len;
#endif
}

void
_edje_entry_input_panel_imdata_get(Edje_Real_Part *rp, void *data, int *len)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (en->imf_context)
     ecore_imf_context_input_panel_imdata_get(en->imf_context, data, len);
#else
   (void)rp;
   (void)data;
   (void)len;
#endif
}

void
_edje_entry_input_panel_return_key_type_set(Edje_Real_Part *rp, Edje_Input_Panel_Return_Key_Type return_key_type)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (en->imf_context)
     ecore_imf_context_input_panel_return_key_type_set(en->imf_context, (Ecore_IMF_Input_Panel_Return_Key_Type)return_key_type);
#else
   (void)rp;
   (void)return_key_type;
#endif
}

Edje_Input_Panel_Return_Key_Type
_edje_entry_input_panel_return_key_type_get(Edje_Real_Part *rp)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EDJE_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;
   en = rp->typedata.text->entry_data;
   if (!en) return EDJE_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;
   if (en->imf_context)
     return (Edje_Input_Panel_Return_Key_Type)ecore_imf_context_input_panel_return_key_type_get(en->imf_context);
   return EDJE_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;
#else
   return EDJE_INPUT_PANEL_RETURN_KEY_TYPE_DEFAULT;
   (void)rp;
#endif
}

void
_edje_entry_input_panel_return_key_disabled_set(Edje_Real_Part *rp, Eina_Bool disabled)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (en->imf_context)
     ecore_imf_context_input_panel_return_key_disabled_set(en->imf_context, disabled);
#else
   (void)rp;
   (void)disabled;
#endif
}

Eina_Bool
_edje_entry_input_panel_return_key_disabled_get(Edje_Real_Part *rp)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   en = rp->typedata.text->entry_data;
   if (!en) return EINA_FALSE;
   if (en->imf_context)
     return ecore_imf_context_input_panel_return_key_disabled_get(en->imf_context);
   return EINA_FALSE;
#else
   return EINA_FALSE;
   (void)rp;
#endif
}

#ifdef HAVE_ECORE_IMF
void
_edje_entry_input_panel_show_on_demand_set(Edje_Real_Part *rp, Eina_Bool ondemand)
#else
void
_edje_entry_input_panel_show_on_demand_set(Edje_Real_Part *rp, Eina_Bool ondemand EINA_UNUSED)
#endif
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_show_on_demand_set(en->imf_context, ondemand);
#endif
}

Eina_Bool
_edje_entry_input_panel_show_on_demand_get(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   en = rp->typedata.text->entry_data;
   if (!en) return EINA_FALSE;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     {
        Eina_Bool ret = ecore_imf_context_input_panel_show_on_demand_get(en->imf_context);
        return ret;
     }
#endif
   return EINA_FALSE;
}

static Evas_Textblock_Cursor *
_cursor_get(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return NULL;
   en = rp->typedata.text->entry_data;
   if (!en) return NULL;
   switch (cur)
     {
      case EDJE_CURSOR_MAIN:
        return en->cursor;

      case EDJE_CURSOR_SELECTION_BEGIN:
        return en->sel_start;

      case EDJE_CURSOR_SELECTION_END:
        return en->sel_end;

      case EDJE_CURSOR_PREEDIT_START:
        if (!en->preedit_start)
          en->preedit_start = evas_object_textblock_cursor_new(rp->object);
        return en->preedit_start;

      case EDJE_CURSOR_PREEDIT_END:
        if (!en->preedit_end)
          en->preedit_end = evas_object_textblock_cursor_new(rp->object);
        return en->preedit_end;

      case EDJE_CURSOR_USER:
        if (!en->cursor_user)
          en->cursor_user = evas_object_textblock_cursor_new(rp->object);
        return en->cursor_user;

      case EDJE_CURSOR_USER_EXTRA:
        if (!en->cursor_user_extra)
          en->cursor_user_extra = evas_object_textblock_cursor_new(rp->object);
        return en->cursor_user_extra;

      default:
        break;
     }
   return NULL;
}

Eina_Bool
_edje_text_cursor_next(Edje_Real_Part *rp, Efl_Text_Cursor_Cursor *c)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   en = rp->typedata.text->entry_data;
   if (!en) return EINA_FALSE;

   if (!c) return EINA_FALSE;

   _edje_entry_imf_context_reset(rp);

   if (!evas_textblock_cursor_cluster_next(c))
     {
        return EINA_FALSE;
     }
   _sel_update(en->ed, c, rp->object, rp->typedata.text->entry_data);
   _edje_entry_imf_cursor_info_set(en);

   _edje_emit(en->ed, "cursor,changed", rp->part->name);
   _edje_entry_real_part_configure(en->ed, rp);
   return EINA_TRUE;
}

Eina_Bool
_edje_entry_cursor_next(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);
   return _edje_text_cursor_next(rp, c);
}


Eina_Bool
_edje_text_cursor_prev(Edje_Real_Part *rp, Efl_Text_Cursor_Cursor *c)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   en = rp->typedata.text->entry_data;
   if (!en) return EINA_FALSE;
   if (!c) return EINA_FALSE;

   _edje_entry_imf_context_reset(rp);

   if (!evas_textblock_cursor_cluster_prev(c))
     {
        if (evas_textblock_cursor_paragraph_prev(c)) goto ok;
        else return EINA_FALSE;
     }
ok:
   _sel_update(en->ed, c, rp->object, rp->typedata.text->entry_data);

   _edje_entry_imf_cursor_info_set(en);

   _edje_emit(en->ed, "cursor,changed", rp->part->name);
   _edje_entry_real_part_configure(en->ed, rp);
   return EINA_TRUE;
}

Eina_Bool
_edje_entry_cursor_prev(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);
   return _edje_text_cursor_prev(rp, c);
}

Eina_Bool
_edje_text_cursor_up(Edje_Real_Part *rp, Efl_Text_Cursor_Cursor *c)
{
   Entry *en;
   Evas_Coord lx, ly, lw, lh, cx, cy, cw, ch;
   int ln;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   en = rp->typedata.text->entry_data;
   if (!en) return EINA_FALSE;
   if (!c) return EINA_FALSE;

   _edje_entry_imf_context_reset(rp);

   ln = evas_textblock_cursor_line_geometry_get(c, NULL, NULL, NULL, NULL);
   ln--;
   if (ln < 0) return EINA_FALSE;
   if (!evas_object_textblock_line_number_geometry_get(rp->object, ln,
                                                       &lx, &ly, &lw, &lh))
     return EINA_FALSE;
   evas_textblock_cursor_char_geometry_get(c, &cx, &cy, &cw, &ch);
   if (!evas_textblock_cursor_cluster_coord_set(c, cx, ly + (lh / 2)))
     evas_textblock_cursor_line_char_last(c);
   _sel_update(en->ed, c, rp->object, rp->typedata.text->entry_data);

   _edje_entry_imf_cursor_info_set(en);

   _edje_emit(en->ed, "cursor,changed", rp->part->name);
   _edje_entry_real_part_configure(en->ed, rp);
   return EINA_TRUE;
}

Eina_Bool
_edje_entry_cursor_up(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);
   return _edje_text_cursor_up(rp, c);
}

Eina_Bool
_edje_text_cursor_down(Edje_Real_Part *rp, Efl_Text_Cursor_Cursor *c)
{
   Entry *en;
   Evas_Coord lx, ly, lw, lh, cx, cy, cw, ch;
   int ln;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   en = rp->typedata.text->entry_data;
   if (!en) return EINA_FALSE;
   if (!c) return EINA_FALSE;

   _edje_entry_imf_context_reset(rp);

   ln = evas_textblock_cursor_line_geometry_get(c, NULL, NULL, NULL, NULL);
   ln++;
   if (!evas_object_textblock_line_number_geometry_get(rp->object, ln,
                                                       &lx, &ly, &lw, &lh))
     return EINA_FALSE;
   evas_textblock_cursor_char_geometry_get(c, &cx, &cy, &cw, &ch);
   if (!evas_textblock_cursor_cluster_coord_set(c, cx, ly + (lh / 2)))
     evas_textblock_cursor_line_char_last(c);

   _sel_update(en->ed, c, rp->object, rp->typedata.text->entry_data);

   _edje_entry_imf_cursor_info_set(en);
   _edje_emit(en->ed, "cursor,changed", rp->part->name);
   _edje_entry_real_part_configure(en->ed, rp);
   return EINA_TRUE;
}

Eina_Bool
_edje_entry_cursor_down(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);
   return _edje_text_cursor_down(rp, c);
}

void
_edje_text_cursor_begin(Edje_Real_Part *rp, Efl_Text_Cursor_Cursor *c)
{
   Entry *en;
   int old_cur_pos;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (!c) return;

   _edje_entry_imf_context_reset(rp);

   old_cur_pos = evas_textblock_cursor_pos_get(c);
   evas_textblock_cursor_paragraph_first(c);

   if (old_cur_pos == evas_textblock_cursor_pos_get(c))
     return;

   _sel_update(en->ed, c, rp->object, rp->typedata.text->entry_data);

   _edje_entry_imf_cursor_info_set(en);
   _edje_emit(en->ed, "cursor,changed", rp->part->name);
   _edje_entry_real_part_configure(en->ed, rp);
}

void
_edje_entry_cursor_begin(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);
   _edje_text_cursor_begin(rp, c);

}

void
_edje_text_cursor_end(Edje_Real_Part *rp, Efl_Text_Cursor_Cursor *c)
{
   Entry *en;
   int old_cur_pos;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (!c) return;

   _edje_entry_imf_context_reset(rp);

   old_cur_pos = evas_textblock_cursor_pos_get(c);
   _curs_end(c, rp->object, rp->typedata.text->entry_data);

   if (old_cur_pos == evas_textblock_cursor_pos_get(c))
     return;

   _sel_update(en->ed, c, rp->object, rp->typedata.text->entry_data);

   _edje_entry_imf_cursor_info_set(en);

   _edje_emit(en->ed, "cursor,changed", rp->part->name);
   _edje_entry_real_part_configure(en->ed, rp);
}
void
_edje_entry_cursor_end(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);
   _edje_text_cursor_end(rp, c);
}

void
_edje_text_cursor_copy(Edje_Real_Part *rp, Efl_Text_Cursor_Cursor *d, Efl_Text_Cursor_Cursor *c)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   evas_textblock_cursor_copy(c, d);
   _sel_update(en->ed, c, rp->object, rp->typedata.text->entry_data);

   _edje_entry_imf_context_reset(rp);
   _edje_entry_imf_cursor_info_set(en);
   _edje_emit(en->ed, "cursor,changed", rp->part->name);
   _edje_entry_real_part_configure(en->ed, rp);
}

void
_edje_entry_cursor_copy(Edje_Real_Part *rp, Edje_Cursor cur, Edje_Cursor dst)
{
   Evas_Textblock_Cursor *c;
   Evas_Textblock_Cursor *d;
   c = _cursor_get(rp, cur);
   if (!c) return;
   d = _cursor_get(rp, dst);
   if (!d) return;
   _edje_text_cursor_copy(rp, d, c);
}

void
_edje_text_cursor_line_begin(Edje_Real_Part *rp, Efl_Text_Cursor_Cursor *c)
{
   Entry *en;
   int old_cur_pos;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (!c) return;
   _edje_entry_imf_context_reset(rp);

   old_cur_pos = evas_textblock_cursor_pos_get(c);
   evas_textblock_cursor_line_char_first(c);

   if (old_cur_pos == evas_textblock_cursor_pos_get(c))
     return;

   _sel_update(en->ed, c, rp->object, rp->typedata.text->entry_data);

   _edje_entry_imf_cursor_info_set(en);

   _edje_emit(en->ed, "cursor,changed", rp->part->name);
   _edje_entry_real_part_configure(en->ed, rp);
}

void
_edje_entry_cursor_line_begin(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);
   _edje_text_cursor_line_begin(rp, c);
}

void
_edje_text_cursor_line_end(Edje_Real_Part *rp, Efl_Text_Cursor_Cursor *c)
{
   Entry *en;
   int old_cur_pos;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (!c) return;
   _edje_entry_imf_context_reset(rp);

   old_cur_pos = evas_textblock_cursor_pos_get(c);
   evas_textblock_cursor_line_char_last(c);

   if (old_cur_pos == evas_textblock_cursor_pos_get(c))
     return;

   _sel_update(en->ed, c, rp->object, rp->typedata.text->entry_data);

   _edje_entry_imf_cursor_info_set(en);
   _edje_emit(en->ed, "cursor,changed", rp->part->name);
   _edje_entry_real_part_configure(en->ed, rp);
}

void
_edje_entry_cursor_line_end(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);
   _edje_text_cursor_line_end(rp, c);
}

Eina_Bool
_edje_text_cursor_coord_set(Edje_Real_Part *rp, Efl_Text_Cursor_Cursor *c,
                             Evas_Coord x, Evas_Coord y)
{
   Entry *en = rp->typedata.text->entry_data;
   if ((c == _cursor_get(rp, EDJE_CURSOR_SELECTION_BEGIN)) ||
       (c == _cursor_get(rp, EDJE_CURSOR_SELECTION_END)))
     {
        if (en->have_selection)
          {
             if (en->selection)
               {
                  free(en->selection);
                  en->selection = NULL;
               }
             _edje_emit(en->ed, "selection,changed", rp->part->name);
          }
     }
   return evas_textblock_cursor_cluster_coord_set(c, x, y);
}

Eina_Bool
_edje_entry_cursor_coord_set(Edje_Real_Part *rp, Edje_Cursor cur,
                             Evas_Coord x, Evas_Coord y)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);
   if (!c) return EINA_FALSE;
   return _edje_text_cursor_coord_set(rp, c, x, y);
}

Eina_Bool
_edje_entry_cursor_is_format_get(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);
   if (!c) return EINA_FALSE;
   if (evas_textblock_cursor_is_format(c)) return EINA_TRUE;
   return EINA_FALSE;
}

Eina_Bool
_edje_entry_cursor_is_visible_format_get(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);
   if (!c) return EINA_FALSE;
   return evas_textblock_cursor_format_is_visible_get(c);
}

char *
_edje_text_cursor_content_get(Edje_Real_Part *rp EINA_UNUSED, Efl_Text_Cursor_Cursor *c)
{
   return evas_textblock_cursor_content_get(c);
}

char *
_edje_entry_cursor_content_get(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);

   if (!c) return NULL;

   return _edje_text_cursor_content_get(rp, c);
}

void
_edje_text_cursor_pos_set(Edje_Real_Part *rp, Efl_Text_Cursor_Cursor *c, int pos)
{
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   Entry *en = rp->typedata.text->entry_data;
   if (!en) return;
   if (!c) return;
   /* Abort if cursor position didn't really change */
   if (evas_textblock_cursor_pos_get(c) == pos)
     return;

   _edje_entry_imf_context_reset(rp);
   evas_textblock_cursor_pos_set(c, pos);
   _sel_update(en->ed, c, rp->object, rp->typedata.text->entry_data);

   _edje_entry_imf_cursor_info_set(en);
   _edje_emit(en->ed, "cursor,changed", rp->part->name);
   _edje_entry_real_part_configure(en->ed, rp);
}

void
_edje_entry_cursor_pos_set(Edje_Real_Part *rp, Edje_Cursor cur, int pos)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);

   _edje_text_cursor_pos_set(rp, c, pos);
}

int
_edje_text_cursor_pos_get(Edje_Real_Part *rp EINA_UNUSED, Efl_Text_Cursor_Cursor *c)
{
   return evas_textblock_cursor_pos_get(c);
}

int
_edje_entry_cursor_pos_get(Edje_Real_Part *rp, Edje_Cursor cur)
{
   Evas_Textblock_Cursor *c = _cursor_get(rp, cur);
   if (!c) return 0;
   return _edje_text_cursor_pos_get(rp, c);
}

void
_edje_entry_input_panel_layout_set(Edje_Real_Part *rp, Edje_Input_Panel_Layout layout)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (en->imf_context)
     ecore_imf_context_input_panel_layout_set(en->imf_context, (Ecore_IMF_Input_Panel_Layout)layout);
#else
   (void)rp;
   (void)layout;
#endif
}

Edje_Input_Panel_Layout
_edje_entry_input_panel_layout_get(Edje_Real_Part *rp)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EDJE_INPUT_PANEL_LAYOUT_INVALID;
   en = rp->typedata.text->entry_data;
   if (!en) return EDJE_INPUT_PANEL_LAYOUT_INVALID;
   if (en->imf_context)
     return (Edje_Input_Panel_Layout)ecore_imf_context_input_panel_layout_get(en->imf_context);
   return EDJE_INPUT_PANEL_LAYOUT_INVALID;
#else
   return EDJE_INPUT_PANEL_LAYOUT_INVALID;
   (void)rp;
#endif
}

void
_edje_entry_input_panel_layout_variation_set(Edje_Real_Part *rp, int variation)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_input_panel_layout_variation_set(en->imf_context, variation);
#else
   (void)variation;
#endif
}

int
_edje_entry_input_panel_layout_variation_get(Edje_Real_Part *rp)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return 0;
   en = rp->typedata.text->entry_data;
   if (!en) return 0;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return ecore_imf_context_input_panel_layout_variation_get(en->imf_context);
#endif

   return 0;
}

void
_edje_entry_imf_context_reset(Edje_Real_Part *rp)
{
#ifdef HAVE_ECORE_IMF
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
   if (en->imf_context)
     ecore_imf_context_reset(en->imf_context);
   if (en->commit_cancel)
     en->commit_cancel = EINA_FALSE;
#else
   (void)rp;
#endif
}

static void
_edje_entry_imf_cursor_location_set(Entry *en)
{
#ifdef HAVE_ECORE_IMF
   Evas_Coord cx = 0, cy = 0, cw = 0, ch = 0;
   Evas_BiDi_Direction dir = 0;
   if (!en || !en->rp || !en->imf_context) return;

   _edje_entry_cursor_geometry_get(en->rp, &cx, &cy, &cw, &ch, &dir);
   ecore_imf_context_cursor_location_set(en->imf_context, cx, cy, cw, ch);
   ecore_imf_context_bidi_direction_set(en->imf_context, (Ecore_IMF_BiDi_Direction)dir);
#else
   (void)en;
#endif
}

static void
_edje_entry_imf_cursor_info_set(Entry *en)
{
   int cursor_pos;

#ifdef HAVE_ECORE_IMF
   if (!en || !en->rp || !en->imf_context) return;

   if (en->have_selection)
     {
        if (evas_textblock_cursor_compare(en->sel_start, en->sel_end) < 0)
          cursor_pos = evas_textblock_cursor_pos_get(en->sel_start);
        else
          cursor_pos = evas_textblock_cursor_pos_get(en->sel_end);
     }
   else
     cursor_pos = evas_textblock_cursor_pos_get(en->cursor);

   ecore_imf_context_cursor_position_set(en->imf_context, cursor_pos);

   _edje_entry_imf_cursor_location_set(en);
#else
   (void)en;
#endif
}

void
_edje_entry_prediction_hint_set(Edje_Real_Part *rp, const char *prediction_hint)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   en = rp->typedata.text->entry_data;
   if (!en) return;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     ecore_imf_context_prediction_hint_set(en->imf_context, prediction_hint);
#else
   (void)prediction_hint;
#endif
}

Eina_Bool
_edje_entry_prediction_hint_hash_set(Edje_Real_Part *rp, const char *key, const char *value)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   en = rp->typedata.text->entry_data;
   if (!en) return EINA_FALSE;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return ecore_imf_context_prediction_hint_hash_set(en->imf_context, key, value);
#else
   (void)key;
   (void)value;
#endif

   return EINA_FALSE;
}

Eina_Bool
_edje_entry_prediction_hint_hash_del(Edje_Real_Part *rp, const char *key)
{
   Entry *en;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   en = rp->typedata.text->entry_data;
   if (!en) return EINA_FALSE;
#ifdef HAVE_ECORE_IMF
   if (en->imf_context)
     return ecore_imf_context_prediction_hint_hash_del(en->imf_context, key);
#else
   (void)key;
#endif

   return EINA_FALSE;
}

#ifdef HAVE_ECORE_IMF

static Edje_Real_Part *
_edje_entry_imf_default_focused_rp_get(Edje *ed)
{
   Eina_Stringshare *seat_name;
   Efl_Input_Device *seat;
   Evas *e;

   e = evas_object_evas_get(ed->obj);
   seat = evas_default_device_get(e, EFL_INPUT_DEVICE_TYPE_SEAT);
   seat_name = _edje_seat_name_get(ed, seat);

   return _edje_focused_part_get(ed, seat_name);
}

static Eina_Bool
_edje_entry_imf_retrieve_surrounding_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, char **text, int *cursor_pos)
{
   Edje *ed = data;
   Edje_Real_Part *rp;
   Entry *en = NULL;
   const char *str;
   char *plain_text;

   rp = _edje_entry_imf_default_focused_rp_get(ed);
   if (!rp) return EINA_FALSE;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   else
     en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return EINA_FALSE;

   if (text)
     {
        str = _edje_entry_text_get(rp);
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
          *cursor_pos = evas_textblock_cursor_pos_get(en->sel_start);
        else if (en->cursor)
          *cursor_pos = evas_textblock_cursor_pos_get(en->cursor);
        else
          *cursor_pos = 0;
     }

   return EINA_TRUE;
}

static void
_edje_entry_imf_event_commit_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info)
{
   Edje *ed = data;
   Edje_Real_Part *rp;
   Entry *en = NULL;
   char *commit_str = event_info;
   Edje_Entry_Change_Info *info = NULL;

   rp = _edje_entry_imf_default_focused_rp_get(ed);
   if ((!rp)) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   else
     en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;

   if (en->have_selection)
     {
        if (strcmp(commit_str, ""))
          {
             /* delete selected characters */
             _range_del_emit(ed, en->cursor, rp->object, en);
             _sel_clear(ed, en->cursor, rp->object, en);
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

   if ((rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD) &&
       _edje_password_show_last)
     _edje_entry_hide_visible_password(ed, en->rp);
   if ((rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD) &&
       _edje_password_show_last && (!en->preedit_start))
     {
        info = _text_filter_text_prepend(ed, en, en->cursor, commit_str,
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
                   _password_timer_cb, en);
          }
     }
   else
     {
        info = _text_filter_text_prepend(ed, en, en->cursor, commit_str,
                                         NULL, NULL,
                                         EINA_TRUE, EINA_TRUE);
     }

   _edje_entry_imf_cursor_info_set(en);
   _anchors_get(en->cursor, rp->object, en);
   if (info)
     {
        _edje_emit(ed, "entry,changed", rp->part->name);
        _edje_emit_full(ed, "entry,changed,user", rp->part->name,
                        info, _free_entry_change_info);
        _edje_emit(ed, "cursor,changed", rp->part->name);
     }
   _edje_entry_imf_cursor_info_set(en);
   _edje_entry_real_part_configure(ed, rp);
}

static void
_edje_entry_imf_event_preedit_changed_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Edje *ed = data;
   Edje_Real_Part *rp;
   Entry *en = NULL;
   Edje_Entry_Change_Info *info = NULL;
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

   rp = _edje_entry_imf_default_focused_rp_get(ed);
   if ((!rp)) return;

   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   else
     en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;

   if (!en->imf_context) return;

   ecore_imf_context_preedit_string_with_attributes_get(en->imf_context,
                                                        &preedit_string,
                                                        &attrs, &cursor_pos);
   if (!preedit_string) return;

   if (!strcmp(preedit_string, ""))
     preedit_end_state = EINA_TRUE;

   if (en->have_selection && !preedit_end_state)
     _range_del_emit(ed, en->cursor, rp->object, en);

   /* delete preedit characters */
   _preedit_del(en);

   preedit_start_pos = evas_textblock_cursor_pos_get(en->cursor);

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

        if ((rp->part->entry_mode == EDJE_ENTRY_EDIT_MODE_PASSWORD) &&
            _edje_password_show_last)
          {
             _edje_entry_hide_visible_password(ed, en->rp);
             info = _text_filter_markup_prepend(ed, en, en->cursor,
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
                        _password_timer_cb, en);
               }
          }
        else
          info = _text_filter_markup_prepend(ed, en, en->cursor,
                                      eina_strbuf_string_get(buf),
                                      NULL, NULL,
                                      EINA_TRUE, EINA_TRUE);
        eina_strbuf_free(buf);
     }

   if (!preedit_end_state)
     {
        /* set preedit start cursor */
        if (!en->preedit_start)
          en->preedit_start = evas_object_textblock_cursor_new(rp->object);
        evas_textblock_cursor_copy(en->cursor, en->preedit_start);

        /* set preedit end cursor */
        if (!en->preedit_end)
          en->preedit_end = evas_object_textblock_cursor_new(rp->object);
        evas_textblock_cursor_copy(en->cursor, en->preedit_end);

        preedit_end_pos = evas_textblock_cursor_pos_get(en->cursor);

        for (i = 0; i < (preedit_end_pos - preedit_start_pos); i++)
          {
             evas_textblock_cursor_char_prev(en->preedit_start);
          }

        en->have_preedit = EINA_TRUE;

        /* set cursor position */
        evas_textblock_cursor_pos_set(en->cursor, preedit_start_pos + cursor_pos);
     }

   _edje_entry_imf_cursor_info_set(en);
   _anchors_get(en->cursor, rp->object, en);
   _edje_emit_full(ed, "preedit,changed", rp->part->name, info,
                   _free_entry_change_info);
   _edje_emit(ed, "cursor,changed", rp->part->name);

   /* delete attribute list */
   if (attrs)
     {
        EINA_LIST_FREE(attrs, attr)
          free(attr);
     }

   free(preedit_string);
}

static void
_edje_entry_imf_event_delete_surrounding_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info)
{
   Edje *ed = data;
   Edje_Real_Part *rp;
   Entry *en = NULL;
   Ecore_IMF_Event_Delete_Surrounding *ev = event_info;
   Evas_Textblock_Cursor *del_start, *del_end;
   Edje_Entry_Change_Info *info;
   int cursor_pos;
   int start, end;
   char *tmp;

   rp = _edje_entry_imf_default_focused_rp_get(ed);
   if ((!rp)) return;
   if ((!rp) || (!ev)) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   else
     en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;

   cursor_pos = evas_textblock_cursor_pos_get(en->cursor);

   del_start = evas_object_textblock_cursor_new(en->rp->object);
   evas_textblock_cursor_pos_set(del_start, cursor_pos + ev->offset);

   del_end = evas_object_textblock_cursor_new(en->rp->object);
   evas_textblock_cursor_pos_set(del_end, cursor_pos + ev->offset + ev->n_chars);

   start = evas_textblock_cursor_pos_get(del_start);
   end = evas_textblock_cursor_pos_get(del_end);
   if (start == end) goto end;

   info = calloc(1, sizeof(*info));
   if  (!info)
     {
        ERR("Running very low on memory");
        return;
     }
   info->insert = EINA_FALSE;
   info->change.del.start = start;
   info->change.del.end = end;

   tmp = evas_textblock_cursor_range_text_get(del_start, del_end, EVAS_TEXTBLOCK_TEXT_MARKUP);
   info->change.del.content = eina_stringshare_add(tmp);
   if (tmp) free(tmp);

   evas_textblock_cursor_range_delete(del_start, del_end);
   _anchors_get(en->cursor, rp->object, en);
   _anchors_update_check(ed, rp);

   _edje_emit(ed, "entry,changed", en->rp->part->name);
   _edje_emit_full(ed, "entry,changed,user", en->rp->part->name, info,
                   _free_entry_change_info);
   _edje_emit(ed, "cursor,changed", en->rp->part->name);
   _edje_emit(ed, "cursor,changed,manual", en->rp->part->name);

   _edje_entry_imf_cursor_info_set(en);
   _edje_entry_real_part_configure(ed, rp);

end:
   evas_textblock_cursor_free(del_start);
   evas_textblock_cursor_free(del_end);
}

static void
_edje_entry_imf_event_selection_set_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info)
{
   Edje *ed = data;
   Edje_Real_Part *rp;
   Entry *en = NULL;
   Ecore_IMF_Event_Selection *ev = event_info;

   rp = _edje_entry_imf_default_focused_rp_get(ed);
   if ((!rp) || (!ev)) return;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return;
   else
     en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;

   if (ev->start == ev->end)
     {
        _edje_entry_cursor_pos_set(rp, EDJE_CURSOR_MAIN, ev->start);
     }
   else
     {
        _sel_clear(ed, en->cursor, rp->object, en);
        evas_textblock_cursor_pos_set(en->cursor, ev->start);
        _sel_enable(ed, en->cursor, rp->object, en);
        _sel_start(en->cursor, rp->object, en);
        evas_textblock_cursor_pos_set(en->cursor, ev->end);
        _sel_extend(ed, en->cursor, rp->object, en);
     }

   _edje_entry_real_part_configure(en->ed, rp);
}

static Eina_Bool
_edje_entry_imf_retrieve_selection_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, char **text)
{
   Edje *ed = data;
   Edje_Real_Part *rp;
   Entry *en = NULL;
   const char *selection_text = NULL;

   rp = _edje_entry_imf_default_focused_rp_get(ed);
   if (!rp) return EINA_FALSE;
   if ((rp->type != EDJE_RP_TYPE_TEXT) ||
       (!rp->typedata.text)) return EINA_FALSE;
   else
     en = rp->typedata.text->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return EINA_FALSE;

   if (en->have_selection)
     {
        selection_text = _edje_entry_selection_get(rp);

        if (text)
          *text = selection_text ? strdup(selection_text) : NULL;

        return selection_text ? EINA_TRUE : EINA_FALSE;
     }
   else
     return EINA_FALSE;
}

#endif

Evas_Textblock_Cursor *
_edje_text_cursor_get(Edje_Real_Part *rp, Edje_Cursor cur)
{
   return _cursor_get(rp, cur);
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
