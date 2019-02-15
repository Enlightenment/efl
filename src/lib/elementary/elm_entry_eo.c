EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_ACTIVATED =
   EFL_EVENT_DESCRIPTION("activated");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_CHANGED =
   EFL_EVENT_DESCRIPTION("changed");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_CHANGED_USER =
   EFL_EVENT_DESCRIPTION("changed,user");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_VALIDATE =
   EFL_EVENT_DESCRIPTION("validate");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_CONTEXT_OPEN =
   EFL_EVENT_DESCRIPTION("context,open");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_ANCHOR_CLICKED =
   EFL_EVENT_DESCRIPTION("anchor,clicked");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_REJECTED =
   EFL_EVENT_DESCRIPTION("rejected");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_MAXLENGTH_REACHED =
   EFL_EVENT_DESCRIPTION("maxlength,reached");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_PREEDIT_CHANGED =
   EFL_EVENT_DESCRIPTION("preedit,changed");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_PRESS =
   EFL_EVENT_DESCRIPTION("press");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_REDO_REQUEST =
   EFL_EVENT_DESCRIPTION("redo,request");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_UNDO_REQUEST =
   EFL_EVENT_DESCRIPTION("undo,request");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_TEXT_SET_DONE =
   EFL_EVENT_DESCRIPTION("text,set,done");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_ABORTED =
   EFL_EVENT_DESCRIPTION("aborted");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_ANCHOR_DOWN =
   EFL_EVENT_DESCRIPTION("anchor,down");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_ANCHOR_HOVER_OPENED =
   EFL_EVENT_DESCRIPTION("anchor,hover,opened");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_ANCHOR_IN =
   EFL_EVENT_DESCRIPTION("anchor,in");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_ANCHOR_OUT =
   EFL_EVENT_DESCRIPTION("anchor,out");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_ANCHOR_UP =
   EFL_EVENT_DESCRIPTION("anchor,up");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_CURSOR_CHANGED =
   EFL_EVENT_DESCRIPTION("cursor,changed");
EWAPI const Efl_Event_Description _ELM_ENTRY_EVENT_CURSOR_CHANGED_MANUAL =
   EFL_EVENT_DESCRIPTION("cursor,changed,manual");

void _elm_entry_scrollable_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool scroll);


static Eina_Error
__eolian_elm_entry_scrollable_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_scrollable_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_scrollable_set, EFL_FUNC_CALL(scroll), Eina_Bool scroll);

Eina_Bool _elm_entry_scrollable_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_scrollable_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_entry_scrollable_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_scrollable_get, Eina_Bool, 0);

void _elm_entry_input_panel_show_on_demand_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool ondemand);


static Eina_Error
__eolian_elm_entry_input_panel_show_on_demand_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_input_panel_show_on_demand_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_input_panel_show_on_demand_set, EFL_FUNC_CALL(ondemand), Eina_Bool ondemand);

Eina_Bool _elm_entry_input_panel_show_on_demand_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_input_panel_show_on_demand_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_entry_input_panel_show_on_demand_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_input_panel_show_on_demand_get, Eina_Bool, 0);

void _elm_entry_context_menu_disabled_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool disabled);


static Eina_Error
__eolian_elm_entry_context_menu_disabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_context_menu_disabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_context_menu_disabled_set, EFL_FUNC_CALL(disabled), Eina_Bool disabled);

Eina_Bool _elm_entry_context_menu_disabled_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_context_menu_disabled_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_entry_context_menu_disabled_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_context_menu_disabled_get, Eina_Bool, 0);

void _elm_entry_cnp_mode_set(Eo *obj, Elm_Entry_Data *pd, Elm_Cnp_Mode cnp_mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_cnp_mode_set, EFL_FUNC_CALL(cnp_mode), Elm_Cnp_Mode cnp_mode);

Elm_Cnp_Mode _elm_entry_cnp_mode_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_cnp_mode_get, Elm_Cnp_Mode, 0);

void _elm_entry_file_text_format_set(Eo *obj, Elm_Entry_Data *pd, Elm_Text_Format format);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_file_text_format_set, EFL_FUNC_CALL(format), Elm_Text_Format format);

void _elm_entry_input_panel_language_set(Eo *obj, Elm_Entry_Data *pd, Elm_Input_Panel_Lang lang);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_input_panel_language_set, EFL_FUNC_CALL(lang), Elm_Input_Panel_Lang lang);

Elm_Input_Panel_Lang _elm_entry_input_panel_language_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_input_panel_language_get, Elm_Input_Panel_Lang, 0);

void _elm_entry_selection_handler_disabled_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool disabled);


static Eina_Error
__eolian_elm_entry_selection_handler_disabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_selection_handler_disabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_selection_handler_disabled_set, EFL_FUNC_CALL(disabled), Eina_Bool disabled);

Eina_Bool _elm_entry_selection_handler_disabled_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_selection_handler_disabled_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_entry_selection_handler_disabled_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_selection_handler_disabled_get, Eina_Bool, 0);

void _elm_entry_input_panel_layout_variation_set(Eo *obj, Elm_Entry_Data *pd, int variation);


static Eina_Error
__eolian_elm_entry_input_panel_layout_variation_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_input_panel_layout_variation_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_input_panel_layout_variation_set, EFL_FUNC_CALL(variation), int variation);

int _elm_entry_input_panel_layout_variation_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_input_panel_layout_variation_get_reflect(const Eo *obj)
{
   int val = elm_obj_entry_input_panel_layout_variation_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_input_panel_layout_variation_get, int, 0);

void _elm_entry_autocapital_type_set(Eo *obj, Elm_Entry_Data *pd, Elm_Autocapital_Type autocapital_type);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_autocapital_type_set, EFL_FUNC_CALL(autocapital_type), Elm_Autocapital_Type autocapital_type);

Elm_Autocapital_Type _elm_entry_autocapital_type_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_autocapital_type_get, Elm_Autocapital_Type, 0);

void _elm_entry_editable_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool editable);


static Eina_Error
__eolian_elm_entry_editable_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_editable_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_editable_set, EFL_FUNC_CALL(editable), Eina_Bool editable);

Eina_Bool _elm_entry_editable_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_editable_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_entry_editable_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_editable_get, Eina_Bool, 0);

void _elm_entry_anchor_hover_style_set(Eo *obj, Elm_Entry_Data *pd, const char *style);


static Eina_Error
__eolian_elm_entry_anchor_hover_style_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_anchor_hover_style_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_anchor_hover_style_set, EFL_FUNC_CALL(style), const char *style);

const char *_elm_entry_anchor_hover_style_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_anchor_hover_style_get_reflect(const Eo *obj)
{
   const char *val = elm_obj_entry_anchor_hover_style_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_anchor_hover_style_get, const char *, NULL);

void _elm_entry_single_line_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool single_line);


static Eina_Error
__eolian_elm_entry_single_line_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_single_line_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_single_line_set, EFL_FUNC_CALL(single_line), Eina_Bool single_line);

Eina_Bool _elm_entry_single_line_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_single_line_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_entry_single_line_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_single_line_get, Eina_Bool, 0);

void _elm_entry_password_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool password);


static Eina_Error
__eolian_elm_entry_password_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_password_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_password_set, EFL_FUNC_CALL(password), Eina_Bool password);

Eina_Bool _elm_entry_password_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_password_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_entry_password_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_password_get, Eina_Bool, 0);

void _elm_entry_input_panel_return_key_disabled_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool disabled);


static Eina_Error
__eolian_elm_entry_input_panel_return_key_disabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_input_panel_return_key_disabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_input_panel_return_key_disabled_set, EFL_FUNC_CALL(disabled), Eina_Bool disabled);

Eina_Bool _elm_entry_input_panel_return_key_disabled_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_input_panel_return_key_disabled_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_entry_input_panel_return_key_disabled_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_input_panel_return_key_disabled_get, Eina_Bool, 0);

void _elm_entry_autosave_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool auto_save);


static Eina_Error
__eolian_elm_entry_autosave_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_autosave_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_autosave_set, EFL_FUNC_CALL(auto_save), Eina_Bool auto_save);

Eina_Bool _elm_entry_autosave_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_autosave_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_entry_autosave_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_autosave_get, Eina_Bool, 0);

void _elm_entry_anchor_hover_parent_set(Eo *obj, Elm_Entry_Data *pd, Efl_Canvas_Object *parent);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_anchor_hover_parent_set, EFL_FUNC_CALL(parent), Efl_Canvas_Object *parent);

Efl_Canvas_Object *_elm_entry_anchor_hover_parent_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_anchor_hover_parent_get, Efl_Canvas_Object *, NULL);

void _elm_entry_prediction_allow_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool prediction);


static Eina_Error
__eolian_elm_entry_prediction_allow_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_prediction_allow_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_prediction_allow_set, EFL_FUNC_CALL(prediction), Eina_Bool prediction);

Eina_Bool _elm_entry_prediction_allow_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_prediction_allow_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_entry_prediction_allow_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_prediction_allow_get, Eina_Bool, 0);

void _elm_entry_input_hint_set(Eo *obj, Elm_Entry_Data *pd, Elm_Input_Hints hints);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_input_hint_set, EFL_FUNC_CALL(hints), Elm_Input_Hints hints);

Elm_Input_Hints _elm_entry_input_hint_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_input_hint_get, Elm_Input_Hints, 0);

void _elm_entry_input_panel_layout_set(Eo *obj, Elm_Entry_Data *pd, Elm_Input_Panel_Layout layout);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_input_panel_layout_set, EFL_FUNC_CALL(layout), Elm_Input_Panel_Layout layout);

Elm_Input_Panel_Layout _elm_entry_input_panel_layout_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_input_panel_layout_get, Elm_Input_Panel_Layout, 8 /* Elm.Input.Panel.Layout.invalid */);

void _elm_entry_input_panel_return_key_type_set(Eo *obj, Elm_Entry_Data *pd, Elm_Input_Panel_Return_Key_Type return_key_type);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_input_panel_return_key_type_set, EFL_FUNC_CALL(return_key_type), Elm_Input_Panel_Return_Key_Type return_key_type);

Elm_Input_Panel_Return_Key_Type _elm_entry_input_panel_return_key_type_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_input_panel_return_key_type_get, Elm_Input_Panel_Return_Key_Type, 0);

void _elm_entry_input_panel_enabled_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool enabled);


static Eina_Error
__eolian_elm_entry_input_panel_enabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_input_panel_enabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_input_panel_enabled_set, EFL_FUNC_CALL(enabled), Eina_Bool enabled);

Eina_Bool _elm_entry_input_panel_enabled_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_input_panel_enabled_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_entry_input_panel_enabled_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_input_panel_enabled_get, Eina_Bool, 0);

void _elm_entry_line_wrap_set(Eo *obj, Elm_Entry_Data *pd, Elm_Wrap_Type wrap);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_line_wrap_set, EFL_FUNC_CALL(wrap), Elm_Wrap_Type wrap);

Elm_Wrap_Type _elm_entry_line_wrap_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_line_wrap_get, Elm_Wrap_Type, 0);

void _elm_entry_cursor_pos_set(Eo *obj, Elm_Entry_Data *pd, int pos);


static Eina_Error
__eolian_elm_entry_cursor_pos_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_cursor_pos_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_cursor_pos_set, EFL_FUNC_CALL(pos), int pos);

int _elm_entry_cursor_pos_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_cursor_pos_get_reflect(const Eo *obj)
{
   int val = elm_obj_entry_cursor_pos_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_cursor_pos_get, int, 0);

void _elm_entry_icon_visible_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool setting);


static Eina_Error
__eolian_elm_entry_icon_visible_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_icon_visible_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_icon_visible_set, EFL_FUNC_CALL(setting), Eina_Bool setting);

void _elm_entry_cursor_line_end_set(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_cursor_line_end_set);

void _elm_entry_select_region_set(Eo *obj, Elm_Entry_Data *pd, int start, int end);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_select_region_set, EFL_FUNC_CALL(start, end), int start, int end);

void _elm_entry_select_region_get(const Eo *obj, Elm_Entry_Data *pd, int *start, int *end);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_entry_select_region_get, EFL_FUNC_CALL(start, end), int *start, int *end);

void _elm_entry_input_panel_return_key_autoenabled_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool enabled);


static Eina_Error
__eolian_elm_entry_input_panel_return_key_autoenabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_input_panel_return_key_autoenabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_input_panel_return_key_autoenabled_set, EFL_FUNC_CALL(enabled), Eina_Bool enabled);

void _elm_entry_end_visible_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool setting);


static Eina_Error
__eolian_elm_entry_end_visible_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_end_visible_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_end_visible_set, EFL_FUNC_CALL(setting), Eina_Bool setting);

void _elm_entry_cursor_begin_set(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_cursor_begin_set);

void _elm_entry_cursor_line_begin_set(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_cursor_line_begin_set);

void _elm_entry_cursor_end_set(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_cursor_end_set);

Efl_Canvas_Object *_elm_entry_textblock_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_textblock_get, Efl_Canvas_Object *, NULL);

Eina_Bool _elm_entry_textblock_cursor_geometry_get(const Eo *obj, Elm_Entry_Data *pd, int *x, int *y, int *w, int *h);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_entry_textblock_cursor_geometry_get, Eina_Bool, 0, EFL_FUNC_CALL(x, y, w, h), int *x, int *y, int *w, int *h);

void *_elm_entry_imf_context_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_imf_context_get, void *, NULL);

Eina_Bool _elm_entry_cursor_is_format_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_cursor_is_format_get, Eina_Bool, 0);

char *_elm_entry_textblock_cursor_content_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_textblock_cursor_content_get, char *, NULL);

const char *_elm_entry_selection_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_selection_get, const char *, NULL);

Eina_Bool _elm_entry_cursor_is_visible_format_get(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_cursor_is_visible_format_get, Eina_Bool, 0);

void _elm_entry_select_allow_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool allow);


static Eina_Error
__eolian_elm_entry_select_allow_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_entry_select_allow_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_select_allow_set, EFL_FUNC_CALL(allow), Eina_Bool allow);

Eina_Bool _elm_entry_select_allow_get(const Eo *obj, Elm_Entry_Data *pd);


static Eina_Value
__eolian_elm_entry_select_allow_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_entry_select_allow_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_select_allow_get, Eina_Bool, 0);

Eina_Bool _elm_entry_cursor_prev(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY(elm_obj_entry_cursor_prev, Eina_Bool, 0);

void _elm_entry_text_style_user_pop(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_text_style_user_pop);

void _elm_entry_item_provider_prepend(Eo *obj, Elm_Entry_Data *pd, Elm_Entry_Item_Provider_Cb func, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_item_provider_prepend, EFL_FUNC_CALL(func, data), Elm_Entry_Item_Provider_Cb func, void *data);

void _elm_entry_input_panel_show(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_input_panel_show);

void _elm_entry_imf_context_reset(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_imf_context_reset);

void _elm_entry_anchor_hover_end(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_anchor_hover_end);

void _elm_entry_cursor_selection_begin(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_cursor_selection_begin);

Eina_Bool _elm_entry_cursor_down(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY(elm_obj_entry_cursor_down, Eina_Bool, 0);

void _elm_entry_file_save(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_file_save);

void _elm_entry_selection_copy(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_selection_copy);

void _elm_entry_text_style_user_push(Eo *obj, Elm_Entry_Data *pd, const char *style);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_text_style_user_push, EFL_FUNC_CALL(style), const char *style);

void _elm_entry_item_provider_remove(Eo *obj, Elm_Entry_Data *pd, Elm_Entry_Item_Provider_Cb func, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_item_provider_remove, EFL_FUNC_CALL(func, data), Elm_Entry_Item_Provider_Cb func, void *data);

const char *_elm_entry_text_style_user_peek(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_text_style_user_peek, const char *, NULL);

void _elm_entry_context_menu_clear(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_context_menu_clear);

Eina_Bool _elm_entry_cursor_up(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY(elm_obj_entry_cursor_up, Eina_Bool, 0);

void _elm_entry_entry_insert(Eo *obj, Elm_Entry_Data *pd, const char *entry);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_insert, EFL_FUNC_CALL(entry), const char *entry);

void _elm_entry_input_panel_imdata_set(Eo *obj, Elm_Entry_Data *pd, const void *data, int len);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_input_panel_imdata_set, EFL_FUNC_CALL(data, len), const void *data, int len);

void _elm_entry_input_panel_imdata_get(const Eo *obj, Elm_Entry_Data *pd, void *data, int *len);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_entry_input_panel_imdata_get, EFL_FUNC_CALL(data, len), void *data, int *len);

void _elm_entry_selection_paste(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_selection_paste);

Eina_Bool _elm_entry_cursor_next(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY(elm_obj_entry_cursor_next, Eina_Bool, 0);

void _elm_entry_select_none(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_select_none);

void _elm_entry_input_panel_hide(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_input_panel_hide);

void _elm_entry_select_all(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_select_all);

void _elm_entry_cursor_selection_end(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_cursor_selection_end);

void _elm_entry_selection_cut(Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_entry_selection_cut);

Eina_Bool _elm_entry_is_empty(const Eo *obj, Elm_Entry_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_entry_is_empty, Eina_Bool, EINA_TRUE /* true */);

void _elm_entry_markup_filter_remove(Eo *obj, Elm_Entry_Data *pd, Elm_Entry_Filter_Cb func, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_markup_filter_remove, EFL_FUNC_CALL(func, data), Elm_Entry_Filter_Cb func, void *data);

void _elm_entry_item_provider_append(Eo *obj, Elm_Entry_Data *pd, Elm_Entry_Item_Provider_Cb func, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_item_provider_append, EFL_FUNC_CALL(func, data), Elm_Entry_Item_Provider_Cb func, void *data);

void _elm_entry_markup_filter_append(Eo *obj, Elm_Entry_Data *pd, Elm_Entry_Filter_Cb func, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_markup_filter_append, EFL_FUNC_CALL(func, data), Elm_Entry_Filter_Cb func, void *data);

void _elm_entry_entry_append(Eo *obj, Elm_Entry_Data *pd, const char *str);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_append, EFL_FUNC_CALL(str), const char *str);

void _elm_entry_context_menu_item_add(Eo *obj, Elm_Entry_Data *pd, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_context_menu_item_add, EFL_FUNC_CALL(label, icon_file, icon_type, func, data), const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);

void _elm_entry_markup_filter_prepend(Eo *obj, Elm_Entry_Data *pd, Elm_Entry_Filter_Cb func, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_markup_filter_prepend, EFL_FUNC_CALL(func, data), Elm_Entry_Filter_Cb func, void *data);

void _elm_entry_prediction_hint_set(Eo *obj, Elm_Entry_Data *pd, const char *prediction_hint);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_entry_prediction_hint_set, EFL_FUNC_CALL(prediction_hint), const char *prediction_hint);

Eina_Bool _elm_entry_prediction_hint_hash_set(Eo *obj, Elm_Entry_Data *pd, const char *key, const char *value);

EOAPI EFL_FUNC_BODYV(elm_obj_entry_prediction_hint_hash_set, Eina_Bool, 0, EFL_FUNC_CALL(key, value), const char *key, const char *value);

Eina_Bool _elm_entry_prediction_hint_hash_del(Eo *obj, Elm_Entry_Data *pd, const char *key);

EOAPI EFL_FUNC_BODYV(elm_obj_entry_prediction_hint_hash_del, Eina_Bool, 0, EFL_FUNC_CALL(key), const char *key);

Efl_Object *_elm_entry_efl_object_constructor(Eo *obj, Elm_Entry_Data *pd);


void _elm_entry_efl_gfx_entity_visible_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool v);


void _elm_entry_efl_gfx_entity_position_set(Eo *obj, Elm_Entry_Data *pd, Eina_Position2D pos);


void _elm_entry_efl_gfx_entity_size_set(Eo *obj, Elm_Entry_Data *pd, Eina_Size2D size);


void _elm_entry_efl_canvas_group_group_member_add(Eo *obj, Elm_Entry_Data *pd, Efl_Canvas_Object *sub_obj);


Eina_Bool _elm_entry_efl_layout_signal_signal_callback_add(Eo *obj, Elm_Entry_Data *pd, const char *emission, const char *source, void *func_data, EflLayoutSignalCb func, Eina_Free_Cb func_free_cb);


Eina_Bool _elm_entry_efl_layout_signal_signal_callback_del(Eo *obj, Elm_Entry_Data *pd, const char *emission, const char *source, void *func_data, EflLayoutSignalCb func, Eina_Free_Cb func_free_cb);


void _elm_entry_efl_layout_signal_signal_emit(Eo *obj, Elm_Entry_Data *pd, const char *emission, const char *source);


void _elm_entry_efl_layout_calc_calc_force(Eo *obj, Elm_Entry_Data *pd);


Eina_Bool _elm_entry_efl_ui_widget_on_access_activate(Eo *obj, Elm_Entry_Data *pd, Efl_Ui_Activate act);


Eina_Error _elm_entry_efl_ui_widget_theme_apply(Eo *obj, Elm_Entry_Data *pd);


Efl_Ui_Focus_Manager *_elm_entry_efl_ui_widget_focus_manager_focus_manager_create(Eo *obj, Elm_Entry_Data *pd, Efl_Ui_Focus_Object *root);


Eina_Bool _elm_entry_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Entry_Data *pd);


Eina_Rect _elm_entry_efl_ui_widget_interest_region_get(const Eo *obj, Elm_Entry_Data *pd);


void _elm_entry_efl_ui_widget_disabled_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool disabled);


Eina_Bool _elm_entry_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Entry_Data *pd, Efl_Canvas_Object *sub_obj);


void _elm_entry_elm_interface_scrollable_policy_set(Eo *obj, Elm_Entry_Data *pd, Elm_Scroller_Policy hbar, Elm_Scroller_Policy vbar);


void _elm_entry_elm_interface_scrollable_bounce_allow_set(Eo *obj, Elm_Entry_Data *pd, Eina_Bool horiz, Eina_Bool vert);


Efl_Access_State_Set _elm_entry_efl_access_object_state_set_get(const Eo *obj, Elm_Entry_Data *pd);


const char *_elm_entry_efl_access_object_i18n_name_get(const Eo *obj, Elm_Entry_Data *pd);


char *_elm_entry_efl_access_text_access_text_get(const Eo *obj, Elm_Entry_Data *pd, int start_offset, int end_offset);


char *_elm_entry_efl_access_text_string_get(const Eo *obj, Elm_Entry_Data *pd, Efl_Access_Text_Granularity granularity, int *start_offset, int *end_offset);


Eina_Bool _elm_entry_efl_access_text_attribute_get(const Eo *obj, Elm_Entry_Data *pd, const char *name, int *start_offset, int *end_offset, char **value);


Eina_List *_elm_entry_efl_access_text_text_attributes_get(const Eo *obj, Elm_Entry_Data *pd, int *start_offset, int *end_offset);


Eina_List *_elm_entry_efl_access_text_default_attributes_get(const Eo *obj, Elm_Entry_Data *pd);


Eina_Bool _elm_entry_efl_access_text_caret_offset_set(Eo *obj, Elm_Entry_Data *pd, int offset);


int _elm_entry_efl_access_text_caret_offset_get(const Eo *obj, Elm_Entry_Data *pd);


Eina_Unicode _elm_entry_efl_access_text_character_get(const Eo *obj, Elm_Entry_Data *pd, int offset);


Eina_Bool _elm_entry_efl_access_text_character_extents_get(const Eo *obj, Elm_Entry_Data *pd, int offset, Eina_Bool screen_coords, Eina_Rect *rect);


int _elm_entry_efl_access_text_character_count_get(const Eo *obj, Elm_Entry_Data *pd);


int _elm_entry_efl_access_text_offset_at_point_get(const Eo *obj, Elm_Entry_Data *pd, Eina_Bool screen_coords, int x, int y);


Eina_List *_elm_entry_efl_access_text_bounded_ranges_get(const Eo *obj, Elm_Entry_Data *pd, Eina_Bool screen_coords, Eina_Rect rect, Efl_Access_Text_Clip_Type xclip, Efl_Access_Text_Clip_Type yclip);


Eina_Bool _elm_entry_efl_access_text_range_extents_get(const Eo *obj, Elm_Entry_Data *pd, Eina_Bool screen_coords, int start_offset, int end_offset, Eina_Rect *rect);


Eina_Bool _elm_entry_efl_access_text_access_selection_set(Eo *obj, Elm_Entry_Data *pd, int selection_number, int start_offset, int end_offset);


void _elm_entry_efl_access_text_access_selection_get(const Eo *obj, Elm_Entry_Data *pd, int selection_number, int *start_offset, int *end_offset);


int _elm_entry_efl_access_text_selections_count_get(const Eo *obj, Elm_Entry_Data *pd);


Eina_Bool _elm_entry_efl_access_text_selection_add(Eo *obj, Elm_Entry_Data *pd, int start_offset, int end_offset);


Eina_Bool _elm_entry_efl_access_text_selection_remove(Eo *obj, Elm_Entry_Data *pd, int selection_number);


Eina_Bool _elm_entry_efl_access_editable_text_text_content_set(Eo *obj, Elm_Entry_Data *pd, const char *string);


Eina_Bool _elm_entry_efl_access_editable_text_insert(Eo *obj, Elm_Entry_Data *pd, const char *string, int position);


Eina_Bool _elm_entry_efl_access_editable_text_copy(Eo *obj, Elm_Entry_Data *pd, int start, int end);


Eina_Bool _elm_entry_efl_access_editable_text_cut(Eo *obj, Elm_Entry_Data *pd, int start, int end);


Eina_Bool _elm_entry_efl_access_editable_text_delete(Eo *obj, Elm_Entry_Data *pd, int start, int end);


Eina_Bool _elm_entry_efl_access_editable_text_paste(Eo *obj, Elm_Entry_Data *pd, int position);


const Efl_Access_Action_Data *_elm_entry_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Entry_Data *pd);


Eina_Error _elm_entry_efl_file_file_set(Eo *obj, Elm_Entry_Data *pd, const char *file);


Eina_Error _elm_entry_efl_file_load(Eo *obj, Elm_Entry_Data *pd);


Efl_Object *_elm_entry_efl_part_part_get(const Eo *obj, Elm_Entry_Data *pd, const char *name);


static Eina_Bool
_elm_entry_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_ENTRY_EXTRA_OPS
#define ELM_ENTRY_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_entry_scrollable_set, _elm_entry_scrollable_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_scrollable_get, _elm_entry_scrollable_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_show_on_demand_set, _elm_entry_input_panel_show_on_demand_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_show_on_demand_get, _elm_entry_input_panel_show_on_demand_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_context_menu_disabled_set, _elm_entry_context_menu_disabled_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_context_menu_disabled_get, _elm_entry_context_menu_disabled_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cnp_mode_set, _elm_entry_cnp_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cnp_mode_get, _elm_entry_cnp_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_file_text_format_set, _elm_entry_file_text_format_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_language_set, _elm_entry_input_panel_language_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_language_get, _elm_entry_input_panel_language_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_selection_handler_disabled_set, _elm_entry_selection_handler_disabled_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_selection_handler_disabled_get, _elm_entry_selection_handler_disabled_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_layout_variation_set, _elm_entry_input_panel_layout_variation_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_layout_variation_get, _elm_entry_input_panel_layout_variation_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_autocapital_type_set, _elm_entry_autocapital_type_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_autocapital_type_get, _elm_entry_autocapital_type_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_editable_set, _elm_entry_editable_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_editable_get, _elm_entry_editable_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_anchor_hover_style_set, _elm_entry_anchor_hover_style_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_anchor_hover_style_get, _elm_entry_anchor_hover_style_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_single_line_set, _elm_entry_single_line_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_single_line_get, _elm_entry_single_line_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_password_set, _elm_entry_password_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_password_get, _elm_entry_password_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_return_key_disabled_set, _elm_entry_input_panel_return_key_disabled_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_return_key_disabled_get, _elm_entry_input_panel_return_key_disabled_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_autosave_set, _elm_entry_autosave_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_autosave_get, _elm_entry_autosave_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_anchor_hover_parent_set, _elm_entry_anchor_hover_parent_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_anchor_hover_parent_get, _elm_entry_anchor_hover_parent_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_prediction_allow_set, _elm_entry_prediction_allow_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_prediction_allow_get, _elm_entry_prediction_allow_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_hint_set, _elm_entry_input_hint_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_hint_get, _elm_entry_input_hint_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_layout_set, _elm_entry_input_panel_layout_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_layout_get, _elm_entry_input_panel_layout_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_return_key_type_set, _elm_entry_input_panel_return_key_type_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_return_key_type_get, _elm_entry_input_panel_return_key_type_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_enabled_set, _elm_entry_input_panel_enabled_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_enabled_get, _elm_entry_input_panel_enabled_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_line_wrap_set, _elm_entry_line_wrap_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_line_wrap_get, _elm_entry_line_wrap_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_pos_set, _elm_entry_cursor_pos_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_pos_get, _elm_entry_cursor_pos_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_icon_visible_set, _elm_entry_icon_visible_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_line_end_set, _elm_entry_cursor_line_end_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_select_region_set, _elm_entry_select_region_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_select_region_get, _elm_entry_select_region_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_return_key_autoenabled_set, _elm_entry_input_panel_return_key_autoenabled_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_end_visible_set, _elm_entry_end_visible_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_begin_set, _elm_entry_cursor_begin_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_line_begin_set, _elm_entry_cursor_line_begin_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_end_set, _elm_entry_cursor_end_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_textblock_get, _elm_entry_textblock_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_textblock_cursor_geometry_get, _elm_entry_textblock_cursor_geometry_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_imf_context_get, _elm_entry_imf_context_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_is_format_get, _elm_entry_cursor_is_format_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_textblock_cursor_content_get, _elm_entry_textblock_cursor_content_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_selection_get, _elm_entry_selection_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_is_visible_format_get, _elm_entry_cursor_is_visible_format_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_select_allow_set, _elm_entry_select_allow_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_select_allow_get, _elm_entry_select_allow_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_prev, _elm_entry_cursor_prev),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_text_style_user_pop, _elm_entry_text_style_user_pop),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_item_provider_prepend, _elm_entry_item_provider_prepend),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_show, _elm_entry_input_panel_show),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_imf_context_reset, _elm_entry_imf_context_reset),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_anchor_hover_end, _elm_entry_anchor_hover_end),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_selection_begin, _elm_entry_cursor_selection_begin),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_down, _elm_entry_cursor_down),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_file_save, _elm_entry_file_save),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_selection_copy, _elm_entry_selection_copy),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_text_style_user_push, _elm_entry_text_style_user_push),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_item_provider_remove, _elm_entry_item_provider_remove),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_text_style_user_peek, _elm_entry_text_style_user_peek),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_context_menu_clear, _elm_entry_context_menu_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_up, _elm_entry_cursor_up),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_insert, _elm_entry_entry_insert),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_imdata_set, _elm_entry_input_panel_imdata_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_imdata_get, _elm_entry_input_panel_imdata_get),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_selection_paste, _elm_entry_selection_paste),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_next, _elm_entry_cursor_next),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_select_none, _elm_entry_select_none),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_input_panel_hide, _elm_entry_input_panel_hide),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_select_all, _elm_entry_select_all),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_cursor_selection_end, _elm_entry_cursor_selection_end),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_selection_cut, _elm_entry_selection_cut),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_is_empty, _elm_entry_is_empty),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_markup_filter_remove, _elm_entry_markup_filter_remove),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_item_provider_append, _elm_entry_item_provider_append),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_markup_filter_append, _elm_entry_markup_filter_append),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_append, _elm_entry_entry_append),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_context_menu_item_add, _elm_entry_context_menu_item_add),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_markup_filter_prepend, _elm_entry_markup_filter_prepend),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_prediction_hint_set, _elm_entry_prediction_hint_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_prediction_hint_hash_set, _elm_entry_prediction_hint_hash_set),
      EFL_OBJECT_OP_FUNC(elm_obj_entry_prediction_hint_hash_del, _elm_entry_prediction_hint_hash_del),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_entry_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_visible_set, _elm_entry_efl_gfx_entity_visible_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_position_set, _elm_entry_efl_gfx_entity_position_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _elm_entry_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_member_add, _elm_entry_efl_canvas_group_group_member_add),
      EFL_OBJECT_OP_FUNC(efl_layout_signal_callback_add, _elm_entry_efl_layout_signal_signal_callback_add),
      EFL_OBJECT_OP_FUNC(efl_layout_signal_callback_del, _elm_entry_efl_layout_signal_signal_callback_del),
      EFL_OBJECT_OP_FUNC(efl_layout_signal_emit, _elm_entry_efl_layout_signal_signal_emit),
      EFL_OBJECT_OP_FUNC(efl_layout_calc_force, _elm_entry_efl_layout_calc_calc_force),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_activate, _elm_entry_efl_ui_widget_on_access_activate),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_entry_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_focus_manager_create, _elm_entry_efl_ui_widget_focus_manager_focus_manager_create),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_on_focus_update, _elm_entry_efl_ui_focus_object_on_focus_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_interest_region_get, _elm_entry_efl_ui_widget_interest_region_get),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_disabled_set, _elm_entry_efl_ui_widget_disabled_set),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_del, _elm_entry_efl_ui_widget_widget_sub_object_del),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_policy_set, _elm_entry_elm_interface_scrollable_policy_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_bounce_allow_set, _elm_entry_elm_interface_scrollable_bounce_allow_set),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_entry_efl_access_object_state_set_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_i18n_name_get, _elm_entry_efl_access_object_i18n_name_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_get, _elm_entry_efl_access_text_access_text_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_string_get, _elm_entry_efl_access_text_string_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_attribute_get, _elm_entry_efl_access_text_attribute_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_attributes_get, _elm_entry_efl_access_text_text_attributes_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_default_attributes_get, _elm_entry_efl_access_text_default_attributes_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_caret_offset_set, _elm_entry_efl_access_text_caret_offset_set),
      EFL_OBJECT_OP_FUNC(efl_access_text_caret_offset_get, _elm_entry_efl_access_text_caret_offset_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_character_get, _elm_entry_efl_access_text_character_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_character_extents_get, _elm_entry_efl_access_text_character_extents_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_character_count_get, _elm_entry_efl_access_text_character_count_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_offset_at_point_get, _elm_entry_efl_access_text_offset_at_point_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_bounded_ranges_get, _elm_entry_efl_access_text_bounded_ranges_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_range_extents_get, _elm_entry_efl_access_text_range_extents_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_access_selection_set, _elm_entry_efl_access_text_access_selection_set),
      EFL_OBJECT_OP_FUNC(efl_access_text_access_selection_get, _elm_entry_efl_access_text_access_selection_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_selections_count_get, _elm_entry_efl_access_text_selections_count_get),
      EFL_OBJECT_OP_FUNC(efl_access_text_selection_add, _elm_entry_efl_access_text_selection_add),
      EFL_OBJECT_OP_FUNC(efl_access_text_selection_remove, _elm_entry_efl_access_text_selection_remove),
      EFL_OBJECT_OP_FUNC(efl_access_editable_text_content_set, _elm_entry_efl_access_editable_text_text_content_set),
      EFL_OBJECT_OP_FUNC(efl_access_editable_text_insert, _elm_entry_efl_access_editable_text_insert),
      EFL_OBJECT_OP_FUNC(efl_access_editable_text_copy, _elm_entry_efl_access_editable_text_copy),
      EFL_OBJECT_OP_FUNC(efl_access_editable_text_cut, _elm_entry_efl_access_editable_text_cut),
      EFL_OBJECT_OP_FUNC(efl_access_editable_text_delete, _elm_entry_efl_access_editable_text_delete),
      EFL_OBJECT_OP_FUNC(efl_access_editable_text_paste, _elm_entry_efl_access_editable_text_paste),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_entry_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_file_set, _elm_entry_efl_file_file_set),
      EFL_OBJECT_OP_FUNC(efl_file_load, _elm_entry_efl_file_load),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_entry_efl_part_part_get),
      ELM_ENTRY_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"scrollable", __eolian_elm_entry_scrollable_set_reflect, __eolian_elm_entry_scrollable_get_reflect},
      {"input_panel_show_on_demand", __eolian_elm_entry_input_panel_show_on_demand_set_reflect, __eolian_elm_entry_input_panel_show_on_demand_get_reflect},
      {"context_menu_disabled", __eolian_elm_entry_context_menu_disabled_set_reflect, __eolian_elm_entry_context_menu_disabled_get_reflect},
      {"selection_handler_disabled", __eolian_elm_entry_selection_handler_disabled_set_reflect, __eolian_elm_entry_selection_handler_disabled_get_reflect},
      {"input_panel_layout_variation", __eolian_elm_entry_input_panel_layout_variation_set_reflect, __eolian_elm_entry_input_panel_layout_variation_get_reflect},
      {"editable", __eolian_elm_entry_editable_set_reflect, __eolian_elm_entry_editable_get_reflect},
      {"anchor_hover_style", __eolian_elm_entry_anchor_hover_style_set_reflect, __eolian_elm_entry_anchor_hover_style_get_reflect},
      {"single_line", __eolian_elm_entry_single_line_set_reflect, __eolian_elm_entry_single_line_get_reflect},
      {"password", __eolian_elm_entry_password_set_reflect, __eolian_elm_entry_password_get_reflect},
      {"input_panel_return_key_disabled", __eolian_elm_entry_input_panel_return_key_disabled_set_reflect, __eolian_elm_entry_input_panel_return_key_disabled_get_reflect},
      {"autosave", __eolian_elm_entry_autosave_set_reflect, __eolian_elm_entry_autosave_get_reflect},
      {"prediction_allow", __eolian_elm_entry_prediction_allow_set_reflect, __eolian_elm_entry_prediction_allow_get_reflect},
      {"input_panel_enabled", __eolian_elm_entry_input_panel_enabled_set_reflect, __eolian_elm_entry_input_panel_enabled_get_reflect},
      {"cursor_pos", __eolian_elm_entry_cursor_pos_set_reflect, __eolian_elm_entry_cursor_pos_get_reflect},
      {"icon_visible", __eolian_elm_entry_icon_visible_set_reflect, NULL},
      {"input_panel_return_key_autoenabled", __eolian_elm_entry_input_panel_return_key_autoenabled_set_reflect, NULL},
      {"end_visible", __eolian_elm_entry_end_visible_set_reflect, NULL},
      {"select_allow", __eolian_elm_entry_select_allow_set_reflect, __eolian_elm_entry_select_allow_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_entry_class_desc = {
   EO_VERSION,
   "Elm.Entry",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Entry_Data),
   _elm_entry_class_initializer,
   _elm_entry_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_entry_class_get, &_elm_entry_class_desc, EFL_UI_LAYOUT_BASE_CLASS, ELM_INTERFACE_SCROLLABLE_MIXIN, EFL_UI_CLICKABLE_INTERFACE, EFL_ACCESS_TEXT_INTERFACE, EFL_ACCESS_EDITABLE_TEXT_INTERFACE, EFL_FILE_MIXIN, EFL_UI_SELECTABLE_INTERFACE, EFL_UI_SCROLLABLE_INTERFACE, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_entry_eo.legacy.c"
