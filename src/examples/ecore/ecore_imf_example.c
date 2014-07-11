/**
 * Ecore example illustrating how to use ecore imf.
 *
 * @verbatim
 * gcc -o ecore_imf_example ecore_imf_example.c `pkg-config --cflags --libs ecore evas eina ecore-evas ecore-imf ecore-imf-evas`
 * @endverbatim
 */

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_IMF.h>
#include <Ecore_IMF_Evas.h>
#include <Evas.h>
#include <stdio.h>

#define WIDTH 480
#define HEIGHT 800

typedef struct _Entry Entry;

struct _Entry
{
   Evas_Object           *rect;
   Evas_Object           *txt_obj;
   Evas_Textblock_Style  *txt_style;
   Evas_Textblock_Cursor *cursor;
   Evas_Textblock_Cursor *preedit_start;
   Evas_Textblock_Cursor *preedit_end;
   Ecore_IMF_Context     *imf_context;
   Eina_Bool              have_preedit : 1;
};

static void _imf_cursor_info_set(Entry *en);

static void
_mouse_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info)
{
   Entry *en = data;
   Evas_Event_Mouse_Down *ev = event_info;
   if (!en) return;

   if (en->imf_context)
     {
        Ecore_IMF_Event_Mouse_Down ecore_ev;
        ecore_imf_evas_event_mouse_down_wrap(ev, &ecore_ev);
        if (ecore_imf_context_filter_event(en->imf_context,
                                           ECORE_IMF_EVENT_MOUSE_DOWN,
                                           (Ecore_IMF_Event *)&ecore_ev))
          return;

        // ecore_imf_context_reset should be called before calculating new cursor position
        ecore_imf_context_reset(en->imf_context);
     }

   // calculate new cursor position
}

static void
_mouse_up_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info)
{
   Entry *en = data;
   Evas_Event_Mouse_Up *ev = event_info;
   if (!en) return;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        _imf_cursor_info_set(en);
        return;
     }

   if (en->imf_context)
     {
        Ecore_IMF_Event_Mouse_Up ecore_ev;
        ecore_imf_evas_event_mouse_up_wrap(ev, &ecore_ev);
        if (ecore_imf_context_filter_event(en->imf_context,
                                           ECORE_IMF_EVENT_MOUSE_UP,
                                           (Ecore_IMF_Event *)&ecore_ev))
          return;
     }

   if (evas_object_focus_get(en->rect))
     {
        // notify cursor information
        _imf_cursor_info_set(en);
     }
   else
     evas_object_focus_set(en->rect, EINA_TRUE);
}

static void
_entry_focus_in_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Entry *en = data;
   if (!en) return;

   if (en->imf_context)
     ecore_imf_context_focus_in(en->imf_context);

   // notify the cursor information
   _imf_cursor_info_set(en);
}

static void
_entry_focus_out_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Entry *en = data;
   if (!en) return;

   if (en->imf_context)
     {
        // ecore_imf_context_reset should be called for flushing the preedit string in focus-out event handler
        ecore_imf_context_reset(en->imf_context);
        ecore_imf_context_focus_out(en->imf_context);
     }
}

static void
_canvas_focus_in_cb(void *data EINA_UNUSED, Evas *e, void *event_info EINA_UNUSED)
{
   Entry *en;
   Evas_Object *obj = evas_focus_get(e);
   if (!obj) return;

   en = evas_object_data_get(obj, "Entry");
   if (en)
     _entry_focus_in_cb(en, NULL, NULL, NULL);
}

static void
_canvas_focus_out_cb(void *data EINA_UNUSED, Evas *e, void *event_info EINA_UNUSED)
{
   Entry *en;
   Evas_Object *obj = evas_focus_get(e);
   if (!obj) return;

   en = evas_object_data_get(obj, "Entry");
   if (en)
     _entry_focus_out_cb(en, NULL, NULL, NULL);
}

static void
_imf_cursor_info_set(Entry *en)
{
   Evas_Coord x, y, w, h;
   Evas_Coord cx, cy, cw, ch; // cursor geometry
   int cursor_pos; // cursor position in chars (Not bytes)

   if (!en) return;

   // get cursor geometry
   evas_object_geometry_get(en->txt_obj, &x, &y, &w, &h);
   evas_textblock_cursor_geometry_get(en->cursor, &cx, &cy, &cw, &ch, NULL, EVAS_TEXTBLOCK_CURSOR_BEFORE);

   // get cursor position
   cursor_pos = evas_textblock_cursor_pos_get(en->cursor);

   ecore_imf_context_cursor_position_set(en->imf_context, cursor_pos);
   ecore_imf_context_cursor_location_set(en->imf_context, x + cx, y + cy, cw, ch);
}

static void
_preedit_del(Entry *en)
{
   if (!en || !en->have_preedit) return;
   if (!en->preedit_start || !en->preedit_end) return;
   if (!evas_textblock_cursor_compare(en->preedit_start, en->preedit_end)) return;

   // delete the preedit characters
   evas_textblock_cursor_range_delete(en->preedit_start, en->preedit_end);
}

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

static Eina_Bool
_ecore_imf_retrieve_surrounding_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, char **text, int *cursor_pos)
{
   // This callback will be called when the Input Method Context module requests the surrounding context.
   Entry *en = data;
   const char *str;

   if (!en) return EINA_FALSE;

   str = evas_object_textblock_text_markup_get(en->txt_obj);

   if (text)
     *text = str ? strdup(str) : strdup("");

   // get the current position of cursor
   if (cursor_pos)
     *cursor_pos = evas_textblock_cursor_pos_get(en->cursor);

   return EINA_TRUE;
}

static void
_ecore_imf_event_delete_surrounding_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info)
{
   // called when the input method needs to delete all or part of the context surrounding the cursor
   Entry *en = data;
   Ecore_IMF_Event_Delete_Surrounding *ev = event_info;
   Evas_Textblock_Cursor *del_start, *del_end;
   int cursor_pos;

   if ((!en) || (!ev)) return;

   // get the current cursor position
   cursor_pos = evas_textblock_cursor_pos_get(en->cursor);

   // start cursor position to be deleted
   del_start = evas_object_textblock_cursor_new(en->txt_obj);
   evas_textblock_cursor_pos_set(del_start, cursor_pos + ev->offset);

   // end cursor position to be deleted
   del_end = evas_object_textblock_cursor_new(en->txt_obj);
   evas_textblock_cursor_pos_set(del_end, cursor_pos + ev->offset + ev->n_chars);

   // implement function to delete character(s) from 'cursor_pos+ev->offset' cursor position to 'cursor_pos + ev->offset + ev->n_chars'
   evas_textblock_cursor_range_delete(del_start, del_end);

   evas_textblock_cursor_free(del_start);
   evas_textblock_cursor_free(del_end);
}

static void
_ecore_imf_event_commit_cb(void *data, Ecore_IMF_Context *ctx EINA_UNUSED, void *event_info)
{
   Entry *en = data;
   char *commit_str = (char *)event_info;
   if (!en) return;

   // delete preedit string
   _preedit_del(en);
   _preedit_clear(en);

   printf("commit string : %s\n", commit_str);

   // insert the commit string in the editor
   evas_object_textblock_text_markup_prepend(en->cursor, commit_str);

   // notify the cursor information
   _imf_cursor_info_set(en);

   return;
}

static void
_ecore_imf_event_preedit_changed_cb(void *data, Ecore_IMF_Context *ctx, void *event_info EINA_UNUSED)
{
   // example how to get preedit string
   Entry *en = data;
   char *preedit_string;
   int cursor_pos;
   Eina_List *attrs = NULL;
   Eina_List *l;
   Ecore_IMF_Preedit_Attr *attr;
   Ecore_IMF_Context *imf_context = ctx;
   int preedit_start_pos, preedit_end_pos;
   int i;
   Eina_Bool preedit_end_state = EINA_FALSE;

   if (!en) return;

   // get preedit string and attributes
   ecore_imf_context_preedit_string_with_attributes_get(imf_context, &preedit_string, &attrs, &cursor_pos);
   printf("preedit string : %s\n", preedit_string);

   if (!strcmp(preedit_string, ""))
     preedit_end_state = EINA_TRUE;

   // delete preedit
   _preedit_del(en);

   preedit_start_pos = evas_textblock_cursor_pos_get(en->cursor);

   // insert preedit character(s)
   if (strlen(preedit_string) > 0)
     {
        if (attrs)
          {
             EINA_LIST_FOREACH(attrs, l, attr)
               {
                  if (attr->preedit_type == ECORE_IMF_PREEDIT_TYPE_SUB1) // style type
                    {
                       // apply appropriate style such as underline
                    }
                  else if (attr->preedit_type == ECORE_IMF_PREEDIT_TYPE_SUB2 || attr->preedit_type == ECORE_IMF_PREEDIT_TYPE_SUB3)
                    {
                       // apply appropriate style such as underline
                    }
               }

             // insert code to display preedit string in your editor
             evas_object_textblock_text_markup_prepend(en->cursor, preedit_string);
          }
     }

   if (!preedit_end_state)
     {
        // set preedit start cursor
        if (!en->preedit_start)
          en->preedit_start = evas_object_textblock_cursor_new(en->txt_obj);
        evas_textblock_cursor_copy(en->cursor, en->preedit_start);

        // set preedit end cursor
        if (!en->preedit_end)
          en->preedit_end = evas_object_textblock_cursor_new(en->txt_obj);
        evas_textblock_cursor_copy(en->cursor, en->preedit_end);

        preedit_end_pos = evas_textblock_cursor_pos_get(en->cursor);

        for (i = 0; i < (preedit_end_pos - preedit_start_pos); i++)
          {
             evas_textblock_cursor_char_prev(en->preedit_start);
          }

        en->have_preedit = EINA_TRUE;

        // set cursor position
        evas_textblock_cursor_pos_set(en->cursor, preedit_start_pos + cursor_pos);
     }

   // notify the cursor information
   _imf_cursor_info_set(en);

   EINA_LIST_FREE(attrs, attr)
     free(attr);

   free(preedit_string);
}

static void
_key_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info)
{
   Entry *en = data;
   Evas_Event_Key_Down *ev = event_info;
   Eina_Bool control, alt, shift;
   if ((!en) || (!ev->key)) return;

   if (en->imf_context)
     {
        Ecore_IMF_Event_Key_Down ecore_ev;
        ecore_imf_evas_event_key_down_wrap(ev, &ecore_ev);
        if (ecore_imf_context_filter_event(en->imf_context,
                                           ECORE_IMF_EVENT_KEY_DOWN,
                                           (Ecore_IMF_Event *)&ecore_ev))
          return;
     }

   control = evas_key_modifier_is_set(ev->modifiers, "Control");
   alt = evas_key_modifier_is_set(ev->modifiers, "Alt");
   shift = evas_key_modifier_is_set(ev->modifiers, "Shift");
   (void)alt;
   (void)shift;

   if (!strcmp(ev->key, "BackSpace"))
     {
        if (evas_textblock_cursor_char_prev(en->cursor))
          {
             evas_textblock_cursor_char_delete(en->cursor);
             // notify the cursor information
             _imf_cursor_info_set(en);
          }
         return;
     }
   else if (!strcmp(ev->key, "Delete") ||
            (!strcmp(ev->key, "KP_Delete") && !ev->string))
     {
        // FILLME
     }
   else if ((control) && (!strcmp(ev->key, "v")))
     {
        // ctrl + v
        // FILLME
     }
   else if ((control) && (!strcmp(ev->key, "a")))
     {
        // ctrl + a
        // FILLME
     }
   else if ((control) && (!strcmp(ev->key, "A")))
     {
        // ctrl + A
        // FILLME
     }
   else if ((control) && ((!strcmp(ev->key, "c") || (!strcmp(ev->key, "Insert")))))
     {
        // ctrl + c
        // FILLME
     }
   else if ((control) && ((!strcmp(ev->key, "x") || (!strcmp(ev->key, "m")))))
     {
        // ctrl + x
        // FILLME
     }
   else if ((control) && (!strcmp(ev->key, "z")))
     {
        // ctrl + z (undo)
        // FILLME
     }
   else if ((control) && (!strcmp(ev->key, "y")))
     {
        // ctrl + y (redo)
        // FILLME
     }
   else if ((!strcmp(ev->key, "Return")) || (!strcmp(ev->key, "KP_Enter")))
     {
        // FILLME
     }
   else
     {
        if (ev->string)
          {
             printf("key down string : %s\n", ev->string);
             evas_object_textblock_text_markup_prepend(en->cursor, ev->string);
          }
     }

   // notify the cursor information
   _imf_cursor_info_set(en);
}

static void
_key_up_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info)
{
   Entry *en = data;
   Evas_Event_Key_Up *ev = event_info;

   if (!en) return;

   if (en->imf_context)
     {
        Ecore_IMF_Event_Key_Up ecore_ev;

        ecore_imf_evas_event_key_up_wrap(ev, &ecore_ev);
        if (ecore_imf_context_filter_event(en->imf_context,
                                           ECORE_IMF_EVENT_KEY_UP,
                                           (Ecore_IMF_Event *)&ecore_ev))
          return;
     }
}

static void
create_input_field(Evas *evas, Entry *en, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   if (!en) return;

   en->have_preedit = EINA_FALSE;
   en->preedit_start = NULL;
   en->preedit_end = NULL;

   // create the background for text input field
   en->rect = evas_object_rectangle_add(evas);
   evas_object_color_set(en->rect, 150, 150, 150, 255); // gray color
   evas_object_move(en->rect, x, y);
   evas_object_resize(en->rect, w, h);
   evas_object_show(en->rect);
   evas_object_data_set(en->rect, "Entry", en);

   // create text object for displaying text
   en->txt_obj = evas_object_textblock_add(evas);
   evas_object_color_set(en->txt_obj, 0, 0, 0, 255);
   evas_object_pass_events_set(en->txt_obj, EINA_TRUE);
   evas_object_move(en->txt_obj, x, y);
   evas_object_resize(en->txt_obj, w, h);
   evas_object_show(en->txt_obj);

   // set style on textblock
   static const char *style_buf =
      "DEFAULT='font=Sans font_size=30 color=#000 text_class=entry'"
      "newline='br'"
      "b='+ font=Sans:style=bold'";
   en->txt_style = evas_textblock_style_new();
   evas_textblock_style_set(en->txt_style, style_buf);
   evas_object_textblock_style_set(en->txt_obj, en->txt_style);

   // create cursor
   en->cursor = evas_object_textblock_cursor_new(en->txt_obj);

   // create input context
   const char *default_id = ecore_imf_context_default_id_get();
   if (!default_id)
     return;

   en->imf_context = ecore_imf_context_add(default_id);
   ecore_imf_context_client_window_set(en->imf_context, (void *)ecore_evas_window_get(ecore_evas_ecore_evas_get(evas)));
   ecore_imf_context_client_canvas_set(en->imf_context, evas);

   // register key event handler
   evas_object_event_callback_add(en->rect, EVAS_CALLBACK_KEY_DOWN, _key_down_cb, en);
   evas_object_event_callback_add(en->rect, EVAS_CALLBACK_KEY_UP, _key_up_cb, en);

   // register mouse event handler
   evas_object_event_callback_add(en->rect, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, en);
   evas_object_event_callback_add(en->rect, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, en);

   // register focus event handler
   evas_object_event_callback_add(en->rect, EVAS_CALLBACK_FOCUS_IN, _entry_focus_in_cb, en);
   evas_object_event_callback_add(en->rect, EVAS_CALLBACK_FOCUS_OUT, _entry_focus_out_cb, en);

   // register retrieve surrounding callback
   ecore_imf_context_retrieve_surrounding_callback_set(en->imf_context, _ecore_imf_retrieve_surrounding_cb, en);

   // register commit event callback
   ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_COMMIT, _ecore_imf_event_commit_cb, en);

   // register preedit changed event handler
   ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_PREEDIT_CHANGED, _ecore_imf_event_preedit_changed_cb, en);

   // register surrounding delete event callback
   ecore_imf_context_event_callback_add(en->imf_context, ECORE_IMF_CALLBACK_DELETE_SURROUNDING, _ecore_imf_event_delete_surrounding_cb, en);
}

static void
delete_input_field(Entry *en)
{
   if (!en) return;

   if (en->rect)
     {
        evas_object_del(en->rect);
        en->rect = NULL;
     }

   if (en->cursor)
     {
        evas_textblock_cursor_free(en->cursor);
        en->cursor = NULL;
     }

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

   if (en->txt_obj)
     {
        evas_object_del(en->txt_obj);
        en->txt_obj = NULL;
     }

   if (en->txt_style)
     {
        evas_textblock_style_free(en->txt_style);
        en->txt_style = NULL;
     }

   if (en->imf_context)
     {
        ecore_imf_context_del(en->imf_context);
        en->imf_context = NULL;
     }
}

int
main(void)
{
   Ecore_Evas *ee;
   Evas *evas;
   Entry en1, en2;

   if (!ecore_evas_init())
     {
        fprintf(stderr, "failed to call ecore_evas_init()\n");
        return EXIT_FAILURE;
     }

   ecore_imf_init();

   // create a new window, with size=WIDTHxHEIGHT and default engine
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

   if (!ee)
     {
        fprintf(stderr, "failed to call ecore_evas_new\n");
        return EXIT_FAILURE;
     }

   ecore_evas_show(ee);

   // get the canvas off just-created window
   evas = ecore_evas_get(ee);
   if (!evas)
     {
        fprintf(stderr, "failed to call ecore_evas_get\n");
        return EXIT_FAILURE;
     }

   // create input field rectangle
   Evas_Object *bg = evas_object_rectangle_add(evas);
   evas_object_move(bg, 0, 0);
   evas_object_resize(bg, WIDTH, HEIGHT);
   evas_object_color_set(bg, 255, 255, 255, 255);
   evas_object_show(bg);

   // register canvas focus in/out event handler
   evas_event_callback_add(evas, EVAS_CALLBACK_CANVAS_FOCUS_IN, _canvas_focus_in_cb, NULL);
   evas_event_callback_add(evas, EVAS_CALLBACK_CANVAS_FOCUS_OUT, _canvas_focus_out_cb, NULL);

   // create input field 1
   create_input_field(evas, &en1, 40, 60, 400, 80);

   // create input field 2
   create_input_field(evas, &en2, 40, 180, 400, 80);

   // give focus to input field 1
   evas_object_focus_set(en1.rect, EINA_TRUE);

   ecore_main_loop_begin(); // begin mainloop

   delete_input_field(&en1); // delete input field 1
   delete_input_field(&en2); // delete input field 2

   evas_event_callback_del_full(evas, EVAS_CALLBACK_CANVAS_FOCUS_IN, _canvas_focus_in_cb, NULL);
   evas_event_callback_del_full(evas, EVAS_CALLBACK_CANVAS_FOCUS_OUT, _canvas_focus_out_cb, NULL);

   ecore_evas_free(ee);

   ecore_imf_shutdown();
   ecore_evas_shutdown();

   return 0;
}

