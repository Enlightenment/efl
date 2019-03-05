EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_ITEM_FOCUSED =
   EFL_EVENT_DESCRIPTION("item,focused");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_ITEM_UNFOCUSED =
   EFL_EVENT_DESCRIPTION("item,unfocused");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_VBAR_DRAG =
   EFL_EVENT_DESCRIPTION("vbar,drag");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_VBAR_PRESS =
   EFL_EVENT_DESCRIPTION("vbar,press");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_VBAR_UNPRESS =
   EFL_EVENT_DESCRIPTION("vbar,unpress");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_HBAR_DRAG =
   EFL_EVENT_DESCRIPTION("hbar,drag");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_HBAR_PRESS =
   EFL_EVENT_DESCRIPTION("hbar,press");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_HBAR_UNPRESS =
   EFL_EVENT_DESCRIPTION("hbar,unpress");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_EDGE_TOP =
   EFL_EVENT_DESCRIPTION("edge,top");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_EDGE_BOTTOM =
   EFL_EVENT_DESCRIPTION("edge,bottom");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_EDGE_LEFT =
   EFL_EVENT_DESCRIPTION("edge,left");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_EDGE_RIGHT =
   EFL_EVENT_DESCRIPTION("edge,right");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_MOVED =
   EFL_EVENT_DESCRIPTION("moved");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_MOVED_BEFORE =
   EFL_EVENT_DESCRIPTION("moved,before");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_MOVED_AFTER =
   EFL_EVENT_DESCRIPTION("moved,after");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_SWIPE =
   EFL_EVENT_DESCRIPTION("swipe");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_MULTI_PINCH_IN =
   EFL_EVENT_DESCRIPTION("multi,pinch,in");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_MULTI_PINCH_OUT =
   EFL_EVENT_DESCRIPTION("multi,pinch,out");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_MULTI_SWIPE_DOWN =
   EFL_EVENT_DESCRIPTION("multi,swipe,down");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_MULTI_SWIPE_UP =
   EFL_EVENT_DESCRIPTION("multi,swipe,up");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_MULTI_SWIPE_RIGHT =
   EFL_EVENT_DESCRIPTION("multi,swipe,right");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_MULTI_SWIPE_LEFT =
   EFL_EVENT_DESCRIPTION("multi,swipe,left");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_RELEASED =
   EFL_EVENT_DESCRIPTION("released");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_ACTIVATED =
   EFL_EVENT_DESCRIPTION("activated");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_HIGHLIGHTED =
   EFL_EVENT_DESCRIPTION("highlighted");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_UNHIGHLIGHTED =
   EFL_EVENT_DESCRIPTION("unhighlighted");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_REALIZED =
   EFL_EVENT_DESCRIPTION("realized");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_UNREALIZED =
   EFL_EVENT_DESCRIPTION("unrealized");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_CONTRACT_REQUEST =
   EFL_EVENT_DESCRIPTION("contract,request");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_EXPAND_REQUEST =
   EFL_EVENT_DESCRIPTION("expand,request");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_CONTRACTED =
   EFL_EVENT_DESCRIPTION("contracted");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_EXPANDED =
   EFL_EVENT_DESCRIPTION("expanded");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_INDEX_UPDATE =
   EFL_EVENT_DESCRIPTION("index,update");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_TREE_EFFECT_FINISHED =
   EFL_EVENT_DESCRIPTION("tree,effect,finished");
EWAPI const Efl_Event_Description _ELM_GENLIST_EVENT_FILTER_DONE =
   EFL_EVENT_DESCRIPTION("filter,done");

void _elm_genlist_homogeneous_set(Eo *obj, Elm_Genlist_Data *pd, Eina_Bool homogeneous);


static Eina_Error
__eolian_elm_genlist_homogeneous_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_homogeneous_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_homogeneous_set, EFL_FUNC_CALL(homogeneous), Eina_Bool homogeneous);

Eina_Bool _elm_genlist_homogeneous_get(const Eo *obj, Elm_Genlist_Data *pd);


static Eina_Value
__eolian_elm_genlist_homogeneous_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_genlist_homogeneous_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_homogeneous_get, Eina_Bool, 0);

void _elm_genlist_select_mode_set(Eo *obj, Elm_Genlist_Data *pd, Elm_Object_Select_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_select_mode_set, EFL_FUNC_CALL(mode), Elm_Object_Select_Mode mode);

Elm_Object_Select_Mode _elm_genlist_select_mode_get(const Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_select_mode_get, Elm_Object_Select_Mode, 4 /* Elm.Object.Select_Mode.max */);

void _elm_genlist_focus_on_selection_set(Eo *obj, Elm_Genlist_Data *pd, Eina_Bool enabled);


static Eina_Error
__eolian_elm_genlist_focus_on_selection_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_focus_on_selection_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_focus_on_selection_set, EFL_FUNC_CALL(enabled), Eina_Bool enabled);

Eina_Bool _elm_genlist_focus_on_selection_get(const Eo *obj, Elm_Genlist_Data *pd);


static Eina_Value
__eolian_elm_genlist_focus_on_selection_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_genlist_focus_on_selection_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_focus_on_selection_get, Eina_Bool, 0);

void _elm_genlist_longpress_timeout_set(Eo *obj, Elm_Genlist_Data *pd, double timeout);


static Eina_Error
__eolian_elm_genlist_longpress_timeout_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_longpress_timeout_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_longpress_timeout_set, EFL_FUNC_CALL(timeout), double timeout);

double _elm_genlist_longpress_timeout_get(const Eo *obj, Elm_Genlist_Data *pd);


static Eina_Value
__eolian_elm_genlist_longpress_timeout_get_reflect(Eo *obj)
{
   double val = elm_obj_genlist_longpress_timeout_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_longpress_timeout_get, double, 0);

void _elm_genlist_multi_select_set(Eo *obj, Elm_Genlist_Data *pd, Eina_Bool multi);


static Eina_Error
__eolian_elm_genlist_multi_select_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_multi_select_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_multi_select_set, EFL_FUNC_CALL(multi), Eina_Bool multi);

Eina_Bool _elm_genlist_multi_select_get(const Eo *obj, Elm_Genlist_Data *pd);


static Eina_Value
__eolian_elm_genlist_multi_select_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_genlist_multi_select_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_multi_select_get, Eina_Bool, 0);

void _elm_genlist_reorder_mode_set(Eo *obj, Elm_Genlist_Data *pd, Eina_Bool reorder_mode);


static Eina_Error
__eolian_elm_genlist_reorder_mode_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_reorder_mode_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_reorder_mode_set, EFL_FUNC_CALL(reorder_mode), Eina_Bool reorder_mode);

Eina_Bool _elm_genlist_reorder_mode_get(const Eo *obj, Elm_Genlist_Data *pd);


static Eina_Value
__eolian_elm_genlist_reorder_mode_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_genlist_reorder_mode_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_reorder_mode_get, Eina_Bool, 0);

void _elm_genlist_decorate_mode_set(Eo *obj, Elm_Genlist_Data *pd, Eina_Bool decorated);


static Eina_Error
__eolian_elm_genlist_decorate_mode_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_decorate_mode_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_decorate_mode_set, EFL_FUNC_CALL(decorated), Eina_Bool decorated);

Eina_Bool _elm_genlist_decorate_mode_get(const Eo *obj, Elm_Genlist_Data *pd);


static Eina_Value
__eolian_elm_genlist_decorate_mode_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_genlist_decorate_mode_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_decorate_mode_get, Eina_Bool, 0);

void _elm_genlist_multi_select_mode_set(Eo *obj, Elm_Genlist_Data *pd, Elm_Object_Multi_Select_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_multi_select_mode_set, EFL_FUNC_CALL(mode), Elm_Object_Multi_Select_Mode mode);

Elm_Object_Multi_Select_Mode _elm_genlist_multi_select_mode_get(const Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_multi_select_mode_get, Elm_Object_Multi_Select_Mode, 2 /* Elm.Object.Multi_Select_Mode.max */);

void _elm_genlist_block_count_set(Eo *obj, Elm_Genlist_Data *pd, int count);


static Eina_Error
__eolian_elm_genlist_block_count_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_block_count_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_block_count_set, EFL_FUNC_CALL(count), int count);

int _elm_genlist_block_count_get(const Eo *obj, Elm_Genlist_Data *pd);


static Eina_Value
__eolian_elm_genlist_block_count_get_reflect(Eo *obj)
{
   int val = elm_obj_genlist_block_count_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_block_count_get, int, 0);

void _elm_genlist_tree_effect_enabled_set(Eo *obj, Elm_Genlist_Data *pd, Eina_Bool enabled);


static Eina_Error
__eolian_elm_genlist_tree_effect_enabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_tree_effect_enabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_tree_effect_enabled_set, EFL_FUNC_CALL(enabled), Eina_Bool enabled);

Eina_Bool _elm_genlist_tree_effect_enabled_get(const Eo *obj, Elm_Genlist_Data *pd);


static Eina_Value
__eolian_elm_genlist_tree_effect_enabled_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_genlist_tree_effect_enabled_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_tree_effect_enabled_get, Eina_Bool, 0);

void _elm_genlist_highlight_mode_set(Eo *obj, Elm_Genlist_Data *pd, Eina_Bool highlight);


static Eina_Error
__eolian_elm_genlist_highlight_mode_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_highlight_mode_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_highlight_mode_set, EFL_FUNC_CALL(highlight), Eina_Bool highlight);

Eina_Bool _elm_genlist_highlight_mode_get(const Eo *obj, Elm_Genlist_Data *pd);


static Eina_Value
__eolian_elm_genlist_highlight_mode_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_genlist_highlight_mode_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_highlight_mode_get, Eina_Bool, 0);

void _elm_genlist_mode_set(Eo *obj, Elm_Genlist_Data *pd, Elm_List_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_mode_set, EFL_FUNC_CALL(mode), Elm_List_Mode mode);

Elm_List_Mode _elm_genlist_mode_get(const Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_mode_get, Elm_List_Mode, 4 /* Elm.List.Mode.last */);

Elm_Widget_Item *_elm_genlist_decorated_item_get(const Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_decorated_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_genlist_selected_item_get(const Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_selected_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_genlist_first_item_get(const Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_first_item_get, Elm_Widget_Item *, NULL);

Eina_List *_elm_genlist_realized_items_get(const Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_realized_items_get, Eina_List *, NULL);

const Eina_List *_elm_genlist_selected_items_get(const Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_selected_items_get, const Eina_List *, NULL);

Elm_Widget_Item *_elm_genlist_last_item_get(const Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_last_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_genlist_item_insert_before(Eo *obj, Elm_Genlist_Data *pd, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Widget_Item *parent, Elm_Widget_Item *before_it, Elm_Genlist_Item_Type type, Evas_Smart_Cb func, const void *func_data);

EOAPI EFL_FUNC_BODYV(elm_obj_genlist_item_insert_before, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(itc, data, parent, before_it, type, func, func_data), const Elm_Genlist_Item_Class *itc, const void *data, Elm_Widget_Item *parent, Elm_Widget_Item *before_it, Elm_Genlist_Item_Type type, Evas_Smart_Cb func, const void *func_data);

void _elm_genlist_realized_items_update(Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_genlist_realized_items_update);

Elm_Widget_Item *_elm_genlist_item_insert_after(Eo *obj, Elm_Genlist_Data *pd, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Widget_Item *parent, Elm_Widget_Item *after_it, Elm_Genlist_Item_Type type, Evas_Smart_Cb func, const void *func_data);

EOAPI EFL_FUNC_BODYV(elm_obj_genlist_item_insert_after, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(itc, data, parent, after_it, type, func, func_data), const Elm_Genlist_Item_Class *itc, const void *data, Elm_Widget_Item *parent, Elm_Widget_Item *after_it, Elm_Genlist_Item_Type type, Evas_Smart_Cb func, const void *func_data);

Elm_Widget_Item *_elm_genlist_at_xy_item_get(const Eo *obj, Elm_Genlist_Data *pd, int x, int y, int *posret);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_genlist_at_xy_item_get, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(x, y, posret), int x, int y, int *posret);

void _elm_genlist_filter_set(Eo *obj, Elm_Genlist_Data *pd, void *key);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_filter_set, EFL_FUNC_CALL(key), void *key);

Eina_Iterator *_elm_genlist_filter_iterator_new(Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_FUNC_BODY(elm_obj_genlist_filter_iterator_new, Eina_Iterator *, NULL);

unsigned int _elm_genlist_filtered_items_count(const Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_filtered_items_count, unsigned int, 0);

unsigned int _elm_genlist_items_count(const Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_items_count, unsigned int, 0);

Elm_Widget_Item *_elm_genlist_item_prepend(Eo *obj, Elm_Genlist_Data *pd, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Widget_Item *parent, Elm_Genlist_Item_Type type, Evas_Smart_Cb func, const void *func_data);

EOAPI EFL_FUNC_BODYV(elm_obj_genlist_item_prepend, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(itc, data, parent, type, func, func_data), const Elm_Genlist_Item_Class *itc, const void *data, Elm_Widget_Item *parent, Elm_Genlist_Item_Type type, Evas_Smart_Cb func, const void *func_data);

void _elm_genlist_clear(Eo *obj, Elm_Genlist_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_genlist_clear);

Elm_Widget_Item *_elm_genlist_item_append(Eo *obj, Elm_Genlist_Data *pd, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Widget_Item *parent, Elm_Genlist_Item_Type type, Evas_Smart_Cb func, const void *func_data);

EOAPI EFL_FUNC_BODYV(elm_obj_genlist_item_append, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(itc, data, parent, type, func, func_data), const Elm_Genlist_Item_Class *itc, const void *data, Elm_Widget_Item *parent, Elm_Genlist_Item_Type type, Evas_Smart_Cb func, const void *func_data);

Elm_Widget_Item *_elm_genlist_item_sorted_insert(Eo *obj, Elm_Genlist_Data *pd, const Elm_Genlist_Item_Class *itc, const void *data, Elm_Widget_Item *parent, Elm_Genlist_Item_Type type, Eina_Compare_Cb comp, Evas_Smart_Cb func, const void *func_data);

EOAPI EFL_FUNC_BODYV(elm_obj_genlist_item_sorted_insert, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(itc, data, parent, type, comp, func, func_data), const Elm_Genlist_Item_Class *itc, const void *data, Elm_Widget_Item *parent, Elm_Genlist_Item_Type type, Eina_Compare_Cb comp, Evas_Smart_Cb func, const void *func_data);

Elm_Widget_Item *_elm_genlist_search_by_text_item_get(Eo *obj, Elm_Genlist_Data *pd, Elm_Widget_Item *item_to_search_from, const char *part_name, const char *pattern, Elm_Glob_Match_Flags flags);

EOAPI EFL_FUNC_BODYV(elm_obj_genlist_search_by_text_item_get, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(item_to_search_from, part_name, pattern, flags), Elm_Widget_Item *item_to_search_from, const char *part_name, const char *pattern, Elm_Glob_Match_Flags flags);

Efl_Object *_elm_genlist_efl_object_constructor(Eo *obj, Elm_Genlist_Data *pd);


void _elm_genlist_efl_gfx_entity_position_set(Eo *obj, Elm_Genlist_Data *pd, Eina_Position2D pos);


void _elm_genlist_efl_gfx_entity_size_set(Eo *obj, Elm_Genlist_Data *pd, Eina_Size2D size);


void _elm_genlist_efl_canvas_group_group_member_add(Eo *obj, Elm_Genlist_Data *pd, Efl_Canvas_Object *sub_obj);


Efl_Object *_elm_genlist_efl_object_provider_find(const Eo *obj, Elm_Genlist_Data *pd, const Efl_Class *klass);


Eina_Error _elm_genlist_efl_ui_widget_theme_apply(Eo *obj, Elm_Genlist_Data *pd);


Eina_Bool _elm_genlist_efl_ui_widget_widget_sub_object_add(Eo *obj, Elm_Genlist_Data *pd, Efl_Canvas_Object *sub_obj);


void _elm_genlist_efl_ui_widget_on_access_update(Eo *obj, Elm_Genlist_Data *pd, Eina_Bool enable);


Eina_Rect _elm_genlist_efl_ui_widget_focus_highlight_geometry_get(const Eo *obj, Elm_Genlist_Data *pd);


Eina_Bool _elm_genlist_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Genlist_Data *pd);


Eina_Bool _elm_genlist_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Genlist_Data *pd, Efl_Canvas_Object *sub_obj);


Eina_Bool _elm_genlist_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Genlist_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


Elm_Widget_Item *_elm_genlist_elm_widget_item_container_focused_item_get(const Eo *obj, Elm_Genlist_Data *pd);


void _elm_genlist_elm_interface_scrollable_item_loop_enabled_set(Eo *obj, Elm_Genlist_Data *pd, Eina_Bool enable);


Eina_Bool _elm_genlist_elm_interface_scrollable_item_loop_enabled_get(const Eo *obj, Elm_Genlist_Data *pd);


void _elm_genlist_elm_interface_scrollable_bounce_allow_set(Eo *obj, Elm_Genlist_Data *pd, Eina_Bool horiz, Eina_Bool vert);


void _elm_genlist_elm_interface_scrollable_bounce_allow_get(const Eo *obj, Elm_Genlist_Data *pd, Eina_Bool *horiz, Eina_Bool *vert);


void _elm_genlist_elm_interface_scrollable_policy_set(Eo *obj, Elm_Genlist_Data *pd, Elm_Scroller_Policy hbar, Elm_Scroller_Policy vbar);


void _elm_genlist_elm_interface_scrollable_policy_get(const Eo *obj, Elm_Genlist_Data *pd, Elm_Scroller_Policy *hbar, Elm_Scroller_Policy *vbar);


Eina_List *_elm_genlist_efl_access_object_access_children_get(const Eo *obj, Elm_Genlist_Data *pd);


Efl_Access_State_Set _elm_genlist_efl_access_object_state_set_get(const Eo *obj, Elm_Genlist_Data *pd);


const Efl_Access_Action_Data *_elm_genlist_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Genlist_Data *pd);


int _elm_genlist_efl_access_selection_selected_children_count_get(const Eo *obj, Elm_Genlist_Data *pd);


Efl_Object *_elm_genlist_efl_access_selection_selected_child_get(const Eo *obj, Elm_Genlist_Data *pd, int selected_child_index);


Eina_Bool _elm_genlist_efl_access_selection_selected_child_deselect(Eo *obj, Elm_Genlist_Data *pd, int child_index);


Eina_Bool _elm_genlist_efl_access_selection_child_select(Eo *obj, Elm_Genlist_Data *pd, int child_index);


Eina_Bool _elm_genlist_efl_access_selection_child_deselect(Eo *obj, Elm_Genlist_Data *pd, int child_index);


Eina_Bool _elm_genlist_efl_access_selection_is_child_selected(Eo *obj, Elm_Genlist_Data *pd, int child_index);


Eina_Bool _elm_genlist_efl_access_selection_all_children_select(Eo *obj, Elm_Genlist_Data *pd);


Eina_Bool _elm_genlist_efl_access_selection_access_selection_clear(Eo *obj, Elm_Genlist_Data *pd);


Eina_Bool _elm_genlist_efl_ui_widget_focus_state_apply(Eo *obj, Elm_Genlist_Data *pd, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect);


void _elm_genlist_efl_ui_focus_manager_setup_on_first_touch(Eo *obj, Elm_Genlist_Data *pd, Efl_Ui_Focus_Direction direction, Efl_Ui_Focus_Object *entry);


Efl_Ui_Focus_Object *_elm_genlist_efl_ui_focus_manager_manager_focus_get(const Eo *obj, Elm_Genlist_Data *pd);


Efl_Ui_Focus_Object *_elm_genlist_efl_ui_focus_manager_move(Eo *obj, Elm_Genlist_Data *pd, Efl_Ui_Focus_Direction direction);


static Eina_Bool
_elm_genlist_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_GENLIST_EXTRA_OPS
#define ELM_GENLIST_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_homogeneous_set, _elm_genlist_homogeneous_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_homogeneous_get, _elm_genlist_homogeneous_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_select_mode_set, _elm_genlist_select_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_select_mode_get, _elm_genlist_select_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_focus_on_selection_set, _elm_genlist_focus_on_selection_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_focus_on_selection_get, _elm_genlist_focus_on_selection_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_longpress_timeout_set, _elm_genlist_longpress_timeout_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_longpress_timeout_get, _elm_genlist_longpress_timeout_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_multi_select_set, _elm_genlist_multi_select_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_multi_select_get, _elm_genlist_multi_select_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_reorder_mode_set, _elm_genlist_reorder_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_reorder_mode_get, _elm_genlist_reorder_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_decorate_mode_set, _elm_genlist_decorate_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_decorate_mode_get, _elm_genlist_decorate_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_multi_select_mode_set, _elm_genlist_multi_select_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_multi_select_mode_get, _elm_genlist_multi_select_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_block_count_set, _elm_genlist_block_count_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_block_count_get, _elm_genlist_block_count_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_tree_effect_enabled_set, _elm_genlist_tree_effect_enabled_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_tree_effect_enabled_get, _elm_genlist_tree_effect_enabled_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_highlight_mode_set, _elm_genlist_highlight_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_highlight_mode_get, _elm_genlist_highlight_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_mode_set, _elm_genlist_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_mode_get, _elm_genlist_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_decorated_item_get, _elm_genlist_decorated_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_selected_item_get, _elm_genlist_selected_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_first_item_get, _elm_genlist_first_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_realized_items_get, _elm_genlist_realized_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_selected_items_get, _elm_genlist_selected_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_last_item_get, _elm_genlist_last_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_insert_before, _elm_genlist_item_insert_before),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_realized_items_update, _elm_genlist_realized_items_update),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_insert_after, _elm_genlist_item_insert_after),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_at_xy_item_get, _elm_genlist_at_xy_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_filter_set, _elm_genlist_filter_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_filter_iterator_new, _elm_genlist_filter_iterator_new),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_filtered_items_count, _elm_genlist_filtered_items_count),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_items_count, _elm_genlist_items_count),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_prepend, _elm_genlist_item_prepend),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_clear, _elm_genlist_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_append, _elm_genlist_item_append),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_sorted_insert, _elm_genlist_item_sorted_insert),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_search_by_text_item_get, _elm_genlist_search_by_text_item_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_genlist_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_position_set, _elm_genlist_efl_gfx_entity_position_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _elm_genlist_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_member_add, _elm_genlist_efl_canvas_group_group_member_add),
      EFL_OBJECT_OP_FUNC(efl_provider_find, _elm_genlist_efl_object_provider_find),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_genlist_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_add, _elm_genlist_efl_ui_widget_widget_sub_object_add),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_genlist_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_focus_highlight_geometry_get, _elm_genlist_efl_ui_widget_focus_highlight_geometry_get),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_on_focus_update, _elm_genlist_efl_ui_focus_object_on_focus_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_del, _elm_genlist_efl_ui_widget_widget_sub_object_del),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_genlist_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(elm_widget_item_container_focused_item_get, _elm_genlist_elm_widget_item_container_focused_item_get),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_item_loop_enabled_set, _elm_genlist_elm_interface_scrollable_item_loop_enabled_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_item_loop_enabled_get, _elm_genlist_elm_interface_scrollable_item_loop_enabled_get),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_bounce_allow_set, _elm_genlist_elm_interface_scrollable_bounce_allow_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_bounce_allow_get, _elm_genlist_elm_interface_scrollable_bounce_allow_get),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_policy_set, _elm_genlist_elm_interface_scrollable_policy_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_policy_get, _elm_genlist_elm_interface_scrollable_policy_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_access_children_get, _elm_genlist_efl_access_object_access_children_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_genlist_efl_access_object_state_set_get),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_genlist_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_children_count_get, _elm_genlist_efl_access_selection_selected_children_count_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_child_get, _elm_genlist_efl_access_selection_selected_child_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_child_deselect, _elm_genlist_efl_access_selection_selected_child_deselect),
      EFL_OBJECT_OP_FUNC(efl_access_selection_child_select, _elm_genlist_efl_access_selection_child_select),
      EFL_OBJECT_OP_FUNC(efl_access_selection_child_deselect, _elm_genlist_efl_access_selection_child_deselect),
      EFL_OBJECT_OP_FUNC(efl_access_selection_is_child_selected, _elm_genlist_efl_access_selection_is_child_selected),
      EFL_OBJECT_OP_FUNC(efl_access_selection_all_children_select, _elm_genlist_efl_access_selection_all_children_select),
      EFL_OBJECT_OP_FUNC(efl_access_selection_clear, _elm_genlist_efl_access_selection_access_selection_clear),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_focus_state_apply, _elm_genlist_efl_ui_widget_focus_state_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_manager_setup_on_first_touch, _elm_genlist_efl_ui_focus_manager_setup_on_first_touch),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_manager_focus_get, _elm_genlist_efl_ui_focus_manager_manager_focus_get),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_manager_move, _elm_genlist_efl_ui_focus_manager_move),
      ELM_GENLIST_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"homogeneous", __eolian_elm_genlist_homogeneous_set_reflect, __eolian_elm_genlist_homogeneous_get_reflect},
      {"focus_on_selection", __eolian_elm_genlist_focus_on_selection_set_reflect, __eolian_elm_genlist_focus_on_selection_get_reflect},
      {"longpress_timeout", __eolian_elm_genlist_longpress_timeout_set_reflect, __eolian_elm_genlist_longpress_timeout_get_reflect},
      {"multi_select", __eolian_elm_genlist_multi_select_set_reflect, __eolian_elm_genlist_multi_select_get_reflect},
      {"reorder_mode", __eolian_elm_genlist_reorder_mode_set_reflect, __eolian_elm_genlist_reorder_mode_get_reflect},
      {"decorate_mode", __eolian_elm_genlist_decorate_mode_set_reflect, __eolian_elm_genlist_decorate_mode_get_reflect},
      {"block_count", __eolian_elm_genlist_block_count_set_reflect, __eolian_elm_genlist_block_count_get_reflect},
      {"tree_effect_enabled", __eolian_elm_genlist_tree_effect_enabled_set_reflect, __eolian_elm_genlist_tree_effect_enabled_get_reflect},
      {"highlight_mode", __eolian_elm_genlist_highlight_mode_set_reflect, __eolian_elm_genlist_highlight_mode_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_genlist_class_desc = {
   EO_VERSION,
   "Elm.Genlist",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Genlist_Data),
   _elm_genlist_class_initializer,
   _elm_genlist_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_genlist_class_get, &_elm_genlist_class_desc, EFL_UI_LAYOUT_BASE_CLASS, ELM_INTERFACE_SCROLLABLE_MIXIN, EFL_UI_CLICKABLE_INTERFACE, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_ACCESS_SELECTION_INTERFACE, EFL_UI_SELECTABLE_INTERFACE, EFL_UI_LEGACY_INTERFACE, ELM_WIDGET_ITEM_CONTAINER_INTERFACE, NULL);

#include "elm_genlist_eo.legacy.c"
