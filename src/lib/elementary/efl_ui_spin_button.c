#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_VALUE_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define EFL_UI_FOCUS_COMPOSITION_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_spin_button_private.h"
#include "efl_ui_spin_private.h"
#include "elm_entry.eo.h"

#define MY_CLASS EFL_UI_SPIN_BUTTON_CLASS

#define MY_CLASS_NAME "Efl.Ui.Spin_Button"

#define EFL_UI_SPIN_BUTTON_DELAY_CHANGE_TIME 0.2

static const char PART_NAME_ENTRY[] = "entry";
static const char PART_NAME_DEC_BUTTON[] = "dec_button";
static const char PART_NAME_TEXT_BUTTON[] = "text_button";
static const char PART_NAME_INC_BUTTON[] = "inc_button";

static void
_inc_dec_button_clicked_cb(void *data, const Efl_Event *event);
static void
_inc_dec_button_pressed_cb(void *data, const Efl_Event *event);
static void
_inc_dec_button_unpressed_cb(void *data, const Efl_Event *event);
static void
_inc_dec_button_mouse_move_cb(void *data, const Efl_Event *event);
static void
_entry_activated_cb(void *data, const Efl_Event *event);
static void
_entry_focus_changed_cb(void *data, const Efl_Event *event);
static void
_access_increment_decrement_info_say(Evas_Object *obj, Eina_Bool is_incremented);

EFL_CALLBACKS_ARRAY_DEFINE(_inc_dec_button_cb,
                           { EFL_UI_EVENT_CLICKED, _inc_dec_button_clicked_cb},
                           { EFL_UI_EVENT_PRESSED, _inc_dec_button_pressed_cb},
                           { EFL_UI_EVENT_UNPRESSED, _inc_dec_button_unpressed_cb},
                           { EFL_EVENT_POINTER_MOVE, _inc_dec_button_mouse_move_cb }
                          );

static void
_entry_show(Evas_Object *obj)
{
   Efl_Ui_Spin_Special_Value *sv;
   Eina_Array_Iterator iterator;
   unsigned int i;
   char buf[32], fmt[32] = "%0.f";

   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   Efl_Ui_Spin_Data *pd = efl_data_scope_get(obj, EFL_UI_SPIN_CLASS);

   EINA_ARRAY_ITER_NEXT(pd->special_values, i, sv, iterator)
     {
        if (sv->value == pd->val)
          {
             snprintf(buf, sizeof(buf), "%s", sv->label);
             elm_object_text_set(sd->ent, buf);
          }
     }

   /* try to construct just the format from given label
    * completely ignoring pre/post words
    */
   if (pd->templates)
     {
        const char *start = strchr(pd->templates, '%');
        while (start)
          {
             /* handle %% */
             if (start[1] != '%')
               break;
             else
               start = strchr(start + 2, '%');
          }

        if (start)
          {
             const char *itr, *end = NULL;
             for (itr = start + 1; *itr != '\0'; itr++)
               {
                  if ((*itr == 'd') || (*itr == 'u') || (*itr == 'i') || (*itr == 'o') ||
                      (*itr == 'x') || (*itr == 'X') || (*itr == 'f') || (*itr == 'F'))
                    {
                       end = itr + 1;
                       break;
                    }
               }

             if ((end) && ((size_t)(end - start + 1) < sizeof(fmt)))
               {
                  memcpy(fmt, start, end - start);
                  fmt[end - start] = '\0';
               }
          }
     }

   if (pd->format_type == SPIN_FORMAT_INT)
     snprintf(buf, sizeof(buf), fmt, (int)pd->val);
   else
     snprintf(buf, sizeof(buf), fmt, pd->val);

   elm_object_text_set(sd->ent, buf);
}

static void
_label_write(Evas_Object *obj)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(obj, MY_CLASS);

   Efl_Ui_Spin_Data *pd = efl_data_scope_get(obj, EFL_UI_SPIN_CLASS);

   if (pd->templates)
     {
        efl_text_set(sd->text_button, pd->templates);
     }
   else
     {
        char buf[1024];

        snprintf(buf, sizeof(buf), "%.0f", pd->val);
        efl_text_set(sd->text_button, buf);
     }
}

static Eina_Bool
_delay_change_timer_cb(void *data)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(data, MY_CLASS);

   sd->delay_change_timer = NULL;
   efl_event_callback_call(data, EFL_UI_SPIN_BUTTON_EVENT_DELAY_CHANGED, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_value_set(Evas_Object *obj,
           double new_val)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   Efl_Ui_Spin_Data *pd = efl_data_scope_get(obj, EFL_UI_SPIN_CLASS);

   if (sd->circulate)
     {
        if (new_val < pd->val_min)
          new_val = pd->val_max;
        else if (new_val > pd->val_max)
          new_val = pd->val_min;
     }

   efl_ui_range_value_set(obj, new_val);
   ecore_timer_del(sd->delay_change_timer);
   sd->delay_change_timer = ecore_timer_add(EFL_UI_SPIN_BUTTON_DELAY_CHANGE_TIME,
                                            _delay_change_timer_cb, obj);

   return EINA_TRUE;
}

static void
_entry_hide(Evas_Object *obj)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(obj, MY_CLASS);

   elm_layout_signal_emit(obj, "efl,state,button,active", "efl");
   elm_layout_signal_emit(obj, "efl,state,entry,inactive", "efl");

   if (sd->entry_visible && !evas_focus_state_get(evas_object_evas_get(obj)))
     sd->entry_reactivate = EINA_TRUE;

   sd->entry_visible = EINA_FALSE;
}

static void
_entry_value_apply(Evas_Object *obj)
{
   Efl_Ui_Spin_Special_Value *sv;
   Eina_Array_Iterator iterator;
   unsigned int i;
   const char *str;
   double val;
   char *end;

   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   Efl_Ui_Spin_Data *pd = efl_data_scope_get(obj, EFL_UI_SPIN_CLASS);

   if (!sd->entry_visible) return;

   efl_event_callback_del(sd->ent, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED,
                          _entry_focus_changed_cb, obj);
   _entry_hide(obj);
   str = elm_object_text_get(sd->ent);
   if (!str) return;

   EINA_ARRAY_ITER_NEXT(pd->special_values, i, sv, iterator)
      if (sv->value == pd->val)
        if (!strcmp(sv->label, str)) return;

   val = strtod(str, &end);
   if (((*end != '\0') && (!isspace(*end))) || (fabs(val - pd->val) < DBL_EPSILON)) return;
   efl_ui_range_value_set(obj, val);

   efl_event_callback_call(obj, EFL_UI_SPIN_EVENT_CHANGED, NULL);
   ecore_timer_del(sd->delay_change_timer);
   sd->delay_change_timer = ecore_timer_add(EFL_UI_SPIN_BUTTON_DELAY_CHANGE_TIME,
                                            _delay_change_timer_cb, obj);
}

static void
_invalid_input_validity_filter(void *data EINA_UNUSED, Evas_Object *obj, char **text)
{
   char *insert = NULL;
   const char *str = NULL;
   int cursor_pos = 0;
   int read_idx = 0, read_char, cmp_char;

   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(text);

   insert = *text;
   str = elm_object_text_get(obj);

   evas_string_char_next_get(*text, 0, &read_char);
   cursor_pos = elm_entry_cursor_pos_get(obj);
   if (read_char)
     {
       if (read_char == '-')
         {
            if (cursor_pos != 0)
              {
                 goto invalid_input;
              }
         }
       if (read_char == '.')
         {
            read_idx = evas_string_char_next_get(str, 0, &cmp_char);
            while (cmp_char)
              {
                 if (read_char == cmp_char)
                   {
                      goto invalid_input;
                   }
                 read_idx = evas_string_char_next_get(str, read_idx, &cmp_char);
               }
         }
       read_idx = evas_string_char_next_get(str, 0, &cmp_char);
       if ((cmp_char == '-') && (cursor_pos == 0))
         {
            goto invalid_input;
         }
     }
   return;

invalid_input:
   *insert = 0;
}

static void
_entry_accept_filter_add(Evas_Object *obj)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   Efl_Ui_Spin_Data *pd = efl_data_scope_get(obj, EFL_UI_SPIN_CLASS);
   static Elm_Entry_Filter_Accept_Set digits_filter_data;

   if (!sd->ent) return;

   elm_entry_markup_filter_remove(sd->ent, elm_entry_filter_accept_set, &digits_filter_data);

   if (pd->decimal_points > 0)
     digits_filter_data.accepted = "-.0123456789";
   else
     digits_filter_data.accepted = "-0123456789";

   elm_entry_markup_filter_prepend(sd->ent, elm_entry_filter_accept_set, &digits_filter_data);
}

static char *
_text_insert(const char *text, const char *input, int pos)
{
   char *result = NULL;
   int text_len, input_len;

   text_len = evas_string_char_len_get(text);
   input_len = evas_string_char_len_get(input);
   result = (char *)calloc(text_len + input_len + 1, sizeof(char));
   if (!result) return NULL;

   strncpy(result, text, pos);
   strcpy(result + pos, input);
   strcpy(result + pos + input_len, text + pos);

   return result;
}

static void
_min_max_validity_filter(void *data, Evas_Object *obj, char **text)
{
   const char *str, *point;
   char *insert, *new_str = NULL;
   double val;
   int max_len = 0, len;

   EINA_SAFETY_ON_NULL_RETURN(data);
   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(text);

   Efl_Ui_Spin_Data *pd = efl_data_scope_get(data, EFL_UI_SPIN_CLASS);

   str = elm_object_text_get(obj);
   if (!str) return;

   insert = *text;
   new_str = _text_insert(str, insert, elm_entry_cursor_pos_get(obj));
   if (!new_str) return;
   if (strchr(new_str, '-')) max_len++;

   if (pd->format_type == SPIN_FORMAT_FLOAT)
     {
        point = strchr(new_str, '.');
        if (point)
          {
             if ((int) strlen(point + 1) > pd->decimal_points)
               {
                  *insert = 0;
                  goto end;
               }
          }
     }

   max_len += (fabs(pd->val_max) > fabs(pd->val_min)) ?
              (log10(fabs(pd->val_max)) + 1) : (log10(fabs(pd->val_min)) + 1);
   len = strlen(new_str);
   if (len < max_len) goto end;

   val = strtod(new_str, NULL);
   if ((val < pd->val_min) || (val > pd->val_max))
     *insert = 0;

end:
   free(new_str);
}

static void
_entry_show_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj,
               void *event_info EINA_UNUSED)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(data, MY_CLASS);

   _entry_show(data);
   elm_object_focus_set(obj, EINA_TRUE);
   elm_entry_select_all(obj);
   sd->entry_visible = EINA_TRUE;
   elm_layout_signal_emit(data, "efl,state,button,inactive", "efl");
}

static void
_toggle_entry(Evas_Object *obj)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(obj, MY_CLASS);

   if (elm_widget_disabled_get(obj)) return;
   if (!sd->editable) return;
   if (sd->entry_visible) _entry_value_apply(obj);
   else
     {
        if (!sd->ent)
          {
             //FIXME: elm_entry will be changed to efl_ui_text after
             //       filter feature implemented.
             //       (Current efl_ui_text has missed filter feature.)
             sd->ent = elm_entry_add(obj);
             Eina_Strbuf *buf = eina_strbuf_new();
             eina_strbuf_append_printf(buf, "spinner/%s", elm_widget_style_get(obj));
             elm_widget_style_set(sd->ent, eina_strbuf_string_get(buf));
             eina_strbuf_free(buf);
             evas_object_event_callback_add
               (sd->ent, EVAS_CALLBACK_SHOW, _entry_show_cb, obj);
             elm_entry_single_line_set(sd->ent, EINA_TRUE);
             elm_layout_content_set(obj, "efl.entry", sd->ent);
             _entry_accept_filter_add(obj);
             elm_entry_markup_filter_append(sd->ent, _invalid_input_validity_filter, NULL);
             if (_elm_config->spinner_min_max_filter_enable)
               elm_entry_markup_filter_append(sd->ent, _min_max_validity_filter, obj);
             efl_event_callback_add(sd->ent, ELM_ENTRY_EVENT_ACTIVATED,
                                    _entry_activated_cb, obj);
          }

        efl_event_callback_add(sd->ent, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED,
                               _entry_focus_changed_cb, obj);
        sd->entry_visible = EINA_TRUE;
        elm_layout_signal_emit(obj, "efl,state,entry,active", "efl");
        {
           Eina_List *items = NULL;

           items = eina_list_append(items, sd->dec_button);
           items = eina_list_append(items, sd->text_button);
           items = eina_list_append(items, sd->ent);
           items = eina_list_append(items, sd->inc_button);

           efl_ui_focus_composition_elements_set(obj, items);
        }
     }
}

static void
_entry_toggle_cb(void *data EINA_UNUSED,
                 Evas_Object *obj,
                 const char *emission EINA_UNUSED,
                 const char *source EINA_UNUSED)
{
   _toggle_entry(obj);
}

static Eina_Bool
_spin_value(void *data)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(data, MY_CLASS);
   Efl_Ui_Spin_Data *pd = efl_data_scope_get(data, EFL_UI_SPIN_CLASS);

   if (_value_set(data, pd->val + (sd->inc_val ? pd->step : -pd->step)))
     _label_write(data);

   return ECORE_CALLBACK_RENEW;
}

static void
_spin_stop(Evas_Object *obj)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(obj, MY_CLASS);

   ELM_SAFE_FREE(sd->spin_timer, ecore_timer_del);

   elm_widget_scroll_freeze_pop(obj);
}

static Eina_Bool
_inc_dec_button_press_start(void *data)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(data, MY_CLASS);

   sd->interval = sd->first_interval;
   sd->longpress_timer = NULL;
   ecore_timer_del(sd->spin_timer);
   sd->spin_timer = ecore_timer_add(sd->interval, _spin_value, data);
   _spin_value(data);

   elm_widget_scroll_freeze_push(data);

   return ECORE_CALLBACK_CANCEL;
}

static void
_inc_dec_button_clicked_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(data, MY_CLASS);

   _spin_stop(data);
   sd->inc_val = sd->inc_button == event->object ? EINA_TRUE : EINA_FALSE;
   _spin_value(data);

   if (_elm_config->access_mode)
     _access_increment_decrement_info_say(data, EINA_TRUE);
}


static void
_inc_dec_button_pressed_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(data, MY_CLASS);

   sd->inc_val = sd->inc_button == event->object ? EINA_TRUE : EINA_FALSE;

   if (sd->longpress_timer) ecore_timer_del(sd->longpress_timer);

   sd->longpress_timer = ecore_timer_add
                           (_elm_config->longpress_timeout,
                            _inc_dec_button_press_start, data);

   if (sd->entry_visible) _entry_value_apply(data);
}

static void
_inc_dec_button_unpressed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(data, MY_CLASS);

   if (sd->longpress_timer)
     {
        ecore_timer_del(sd->longpress_timer);
        sd->longpress_timer = NULL;
     }

   _spin_stop(data);
}

static void
_inc_dec_button_mouse_move_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(data, MY_CLASS);

   if (efl_input_processed_get(ev) && sd->longpress_timer)
     {
        ecore_timer_del(sd->longpress_timer);
        sd->longpress_timer = NULL;
     }
}

static void
_text_button_focus_changed_cb(void *data, const Efl_Event *event)
{
   if (efl_ui_focus_object_focus_get(event->object))
     _toggle_entry(data);
}

static void
_entry_activated_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   _toggle_entry(data);
}

static void
_entry_focus_changed_cb(void *data, const Efl_Event *event)
{
   if (!efl_ui_focus_object_focus_get(event->object))
     _toggle_entry(data);
}

static void
_text_button_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(data, MY_CLASS);

   if (sd->entry_visible) return;
   _toggle_entry(data);
}

static Eina_Bool
_key_action_toggle(Evas_Object *obj, const char *params EINA_UNUSED)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(obj, MY_CLASS);

   if (sd->spin_timer) _spin_stop(obj);
   else if (sd->entry_visible) _entry_toggle_cb(NULL, obj, NULL, NULL);

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_spin_button_efl_ui_widget_widget_event(Eo *obj, Efl_Ui_Spin_Button_Data *sd EINA_UNUSED, const Efl_Event *eo_event, Evas_Object *src EINA_UNUSED)
{
   Eo *ev = eo_event->info;

   if (efl_input_processed_get(ev)) return EINA_FALSE;
   if (eo_event->desc == EFL_EVENT_KEY_DOWN)
     {
        if (sd->spin_timer) _spin_stop(obj);
        else return EINA_FALSE;
     }
   else if (eo_event->desc == EFL_EVENT_KEY_UP)
     {
        if (sd->spin_timer) _spin_stop(obj);
        else return EINA_FALSE;
     }
   else if (eo_event->desc == EFL_EVENT_POINTER_WHEEL)
     {
        sd->interval = sd->first_interval;
        if (efl_input_pointer_wheel_delta_get(ev) < 0)
          sd->inc_val = EINA_TRUE;
        else
          sd->inc_val = EINA_FALSE;

	    _spin_value(obj);
     }
   else return EINA_FALSE;

   efl_input_processed_set(ev, EINA_TRUE);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_spin_button_efl_ui_focus_object_on_focus_update(Eo *obj, Efl_Ui_Spin_Button_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   if (!efl_ui_focus_object_focus_get(obj))
     {
        ELM_SAFE_FREE(sd->delay_change_timer, ecore_timer_del);
        ELM_SAFE_FREE(sd->spin_timer, ecore_timer_del);
        ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);
     }
   else
     {
        if (sd->entry_reactivate)
          {
             _toggle_entry(obj);

             sd->entry_reactivate = EINA_FALSE;
          }
     }

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_spin_button_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Spin_Button_Data *_pd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   const char *txt = NULL;
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(data, MY_CLASS);

   if (sd->entry_visible)
     txt = elm_object_text_get(sd->ent);
   else
	 txt = efl_text_get(sd->text_button);

   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   if (elm_widget_disabled_get(data))
     return strdup(E_("State: Disabled"));

   return NULL;
}

static void
_access_activate_spin_button_cb(void *data,
                            Evas_Object *part_obj EINA_UNUSED,
                            Elm_Object_Item *item EINA_UNUSED)
{
   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(data, MY_CLASS);

   if (elm_widget_disabled_get(data)) return;
   if (!sd->entry_visible)
     _toggle_entry(data);
}

static void
_access_increment_decrement_info_say(Evas_Object *obj,
                                     Eina_Bool is_incremented)
{
   Eina_Strbuf *buf;

   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(obj, MY_CLASS);

   buf = eina_strbuf_new();
   if (is_incremented)
     {
        elm_object_signal_emit
           (sd->inc_button, "efl,action,anim,activate", "efl");
        eina_strbuf_append(buf, E_("incremented"));
     }
   else
     {
        elm_object_signal_emit
           (sd->dec_button, "efl,action,anim,activate", "efl");
        eina_strbuf_append(buf, E_("decremented"));
     }

   eina_strbuf_append_printf
      (buf, "%s", elm_object_text_get(sd->text_button));

   _elm_access_say(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
}

static void
_access_spinner_register(Evas_Object *obj, Eina_Bool is_access)
{
   Evas_Object *ao;
   Elm_Access_Info *ai;

   Efl_Ui_Spin_Button_Data *sd = efl_data_scope_get(obj, MY_CLASS);

   if (!is_access)
     {
        /* unregister access */
        _elm_access_edje_object_part_object_unregister
           (obj, elm_layout_edje_get(obj), "access");
        elm_layout_signal_emit(obj, "efl,state,access,inactive", "efl");
        return;
     }
   elm_layout_signal_emit(obj, "efl,state,access,active", "efl");
   ao = _elm_access_edje_object_part_object_register
      (obj, elm_layout_edje_get(obj), "access");

   ai = _elm_access_info_get(ao);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("spinner"));
   _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, obj);
   _elm_access_activate_callback_set(ai, _access_activate_spin_button_cb, obj);

   /*Do not register spinner buttons if widget is disabled*/
   if (!elm_widget_disabled_get(obj))
     {
        ai = _elm_access_info_get(sd->inc_button);
        _elm_access_text_set(ai, ELM_ACCESS_TYPE,
                             E_("spinner increment button"));
        ai = _elm_access_info_get(sd->dec_button);
        _elm_access_text_set(ai, ELM_ACCESS_TYPE,
                             E_("spinner decrement button"));
        ai = _elm_access_info_get(sd->text_button);
        _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("spinner text"));
        _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, obj);
     }
}

static const char *
_theme_group_modify_pos_get(const char *cur_group, const char *search, size_t len)
{
   const char *pos = NULL;
   const char *temp_str = NULL;

   temp_str = cur_group + len - strlen(search);
   if (temp_str >= cur_group)
     {
        if (!strcmp(temp_str, search))
          pos = temp_str;
     }

   return pos;
}

static char *
_efl_ui_spin_button_theme_group_get(Evas_Object *obj, Efl_Ui_Spin_Button_Data *sd)
{
   const char *pos = NULL;
   const char *cur_group = elm_widget_theme_element_get(obj);
   Eina_Strbuf *new_group = eina_strbuf_new();
   size_t len = 0;

   if (cur_group)
     {
        len = strlen(cur_group);
        pos = _theme_group_modify_pos_get(cur_group, "horizontal", len);
        if (!pos)
          pos = _theme_group_modify_pos_get(cur_group, "vertical", len);

        // TODO: change separator when it is decided.
        //       can skip when prev_group == cur_group
        if (!pos)
          {
             eina_strbuf_append(new_group, cur_group);
             eina_strbuf_append(new_group, "/");
          }
        else
          {
             eina_strbuf_append_length(new_group, cur_group, pos - cur_group);
          }
     }

   if (efl_ui_dir_is_horizontal(sd->dir, EINA_TRUE))
     eina_strbuf_append(new_group, "horizontal");
   else
     eina_strbuf_append(new_group, "vertical");

   return eina_strbuf_release(new_group);
}


EOLIAN static Efl_Ui_Theme_Apply_Result
_efl_ui_spin_button_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Spin_Button_Data *sd EINA_UNUSED)
{
   Efl_Ui_Theme_Apply_Result int_ret = EFL_UI_THEME_APPLY_RESULT_FAIL;
   char *group;

   group = _efl_ui_spin_button_theme_group_get(obj, sd);
   if (group)
     {
        elm_widget_theme_element_set(obj, group);
        free(group);
     }

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_RESULT_FAIL;

   if (sd->ent)
     elm_widget_element_update(obj, sd->ent, PART_NAME_ENTRY);

   if (sd->inc_button)
     elm_widget_element_update(obj, sd->inc_button, PART_NAME_INC_BUTTON);

   if (sd->text_button)
     elm_widget_element_update(obj, sd->text_button, PART_NAME_TEXT_BUTTON);

   if (sd->dec_button)
     elm_widget_element_update(obj, sd->dec_button, PART_NAME_DEC_BUTTON);

   if (_elm_config->access_mode)
     _access_spinner_register(obj, EINA_TRUE);

   _label_write(obj);
   elm_layout_sizing_eval(obj);
   return EFL_UI_THEME_APPLY_RESULT_SUCCESS;
}

EOLIAN static Eo *
_efl_ui_spin_button_efl_object_constructor(Eo *obj, Efl_Ui_Spin_Button_Data *sd)
{
   char *group;

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);
   elm_widget_theme_klass_set(obj, "spin_button");

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   group = _efl_ui_spin_button_theme_group_get(obj, sd);
   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                    elm_widget_theme_klass_get(obj),
                                    group,
                                    elm_widget_theme_style_get(obj)))
     CRI("Failed to set layout!");

   free(group);

   sd->first_interval = 0.85;

   sd->inc_button = efl_add(EFL_UI_BUTTON_CLASS, obj,
                            elm_widget_element_update(obj, efl_added, PART_NAME_INC_BUTTON),
                            efl_event_callback_array_add(efl_added, _inc_dec_button_cb(), obj),
                            efl_content_set(efl_part(obj, "efl.inc_button"), efl_added));

   sd->text_button = efl_add(EFL_UI_BUTTON_CLASS, obj,
                             elm_widget_element_update(obj, efl_added, PART_NAME_TEXT_BUTTON),
                             efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                                    _text_button_clicked_cb, obj),
                             efl_event_callback_add(efl_added, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED,
                                                    _text_button_focus_changed_cb, obj),
                             efl_content_set(efl_part(obj, "efl.text_button"), efl_added));

   sd->dec_button = efl_add(EFL_UI_BUTTON_CLASS, obj,
                            elm_widget_element_update(obj, efl_added, PART_NAME_DEC_BUTTON),
                            efl_event_callback_array_add(efl_added, _inc_dec_button_cb(), obj),
                            efl_content_set(efl_part(obj, "efl.dec_button"), efl_added));

     {
        Eina_List *items = NULL;

        items = eina_list_append(items, sd->dec_button);
        items = eina_list_append(items, sd->text_button);
        items = eina_list_append(items, sd->inc_button);

        efl_ui_focus_composition_elements_set(obj, items);
     }

   elm_layout_signal_callback_add
      (obj, "efl,action,entry,toggle", "*", _entry_toggle_cb, NULL);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_SPIN_BUTTON);

   return obj;
}

EOLIAN static void
_efl_ui_spin_button_efl_ui_direction_direction_set(Eo *obj, Efl_Ui_Spin_Button_Data *sd, Efl_Ui_Dir dir)
{
   if (sd->dir == dir) return;

   sd->dir = dir;

   efl_ui_widget_theme_apply(obj);
}

EOLIAN static Efl_Ui_Dir
_efl_ui_spin_button_efl_ui_direction_direction_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Button_Data *sd)
{
   return sd->dir;
}

EOLIAN static void
_efl_ui_spin_button_efl_ui_range_range_value_set(Eo *obj, Efl_Ui_Spin_Button_Data *sd EINA_UNUSED, double val)
{
   efl_ui_range_value_set(efl_super(obj, MY_CLASS), val);

   _label_write(obj);
}

EOLIAN static void
_efl_ui_spin_button_editable_set(Eo *obj EINA_UNUSED, Efl_Ui_Spin_Button_Data *sd, Eina_Bool editable)
{
   sd->editable = editable;
}

EOLIAN static Eina_Bool
_efl_ui_spin_button_editable_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Button_Data *sd)
{
   return sd->editable;
}

EOLIAN static void
_efl_ui_spin_button_circulate_set(Eo *obj EINA_UNUSED, Efl_Ui_Spin_Button_Data *sd, Eina_Bool circulate)
{
   sd->circulate = circulate;
}

EOLIAN static Eina_Bool
_efl_ui_spin_button_circulate_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Button_Data *sd)
{
   return sd->circulate;
}

EOLIAN static const Efl_Access_Action_Data *
_efl_ui_spin_button_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Button_Data *sd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "toggle", "toggle", NULL, _key_action_toggle},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

// A11Y Accessibility

EOLIAN static void
_efl_ui_spin_button_efl_access_value_value_and_text_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Button_Data *sd EINA_UNUSED, double *value, const char **text)
{
   Efl_Ui_Spin_Data *pd = efl_data_scope_get(obj, EFL_UI_SPIN_CLASS);

   if (value) *value = pd->val;
   if (text) *text = NULL;
}

EOLIAN static Eina_Bool
_efl_ui_spin_button_efl_access_value_value_and_text_set(Eo *obj, Efl_Ui_Spin_Button_Data *sd EINA_UNUSED, double value, const char *text EINA_UNUSED)
{
   Efl_Ui_Spin_Data *pd = efl_data_scope_get(obj, EFL_UI_SPIN_CLASS);

   if (pd->val_min > value) return EINA_FALSE;
   if (pd->val_max < value) return EINA_FALSE;

   pd->val = value;
   efl_ui_range_value_set(efl_super(obj, MY_CLASS), value);

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_spin_button_efl_access_value_range_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Button_Data *sd EINA_UNUSED, double *lower, double *upper, const char **descr)
{
   Efl_Ui_Spin_Data *pd = efl_data_scope_get(obj, EFL_UI_SPIN_CLASS);

   if (lower) *lower = pd->val_min;
   if (upper) *upper = pd->val_max;
   if (descr) *descr = NULL;
}

EOLIAN static double
_efl_ui_spin_button_efl_access_value_increment_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Button_Data *sd EINA_UNUSED)
{
   Efl_Ui_Spin_Data *pd = efl_data_scope_get(obj, EFL_UI_SPIN_CLASS);

   return pd->step;
}

EOLIAN static const char*
_efl_ui_spin_button_efl_access_object_i18n_name_get(const Eo *obj, Efl_Ui_Spin_Button_Data *sd EINA_UNUSED)
{
   const char *name;
   name = efl_access_object_i18n_name_get(efl_super(obj, EFL_UI_SPIN_BUTTON_CLASS));
   if (name) return name;
   const char *ret = elm_layout_text_get(obj, "efl.text");
   return ret;
}

// A11Y Accessibility - END

#define EFL_UI_SPIN_BUTTON_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_spin_button), \

#include "efl_ui_spin_button.eo.c"
