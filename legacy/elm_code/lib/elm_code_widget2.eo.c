EOAPI const Eo_Event_Description _ELM_CODE_WIDGET2_EVENT_FOCUSED =
   EO_EVENT_DESCRIPTION("focused", "");
EOAPI const Eo_Event_Description _ELM_CODE_WIDGET2_EVENT_UNFOCUSED =
   EO_EVENT_DESCRIPTION("unfocused", "");

void _elm_code_widget2_font_size_set(Eo *obj, Elm_Code_Widget2_Data *pd, Evas_Font_Size font_size);

EOAPI EO_VOID_FUNC_BODYV(elm_code_widget2_font_size_set, EO_FUNC_CALL(font_size), Evas_Font_Size font_size);

Evas_Font_Size _elm_code_widget2_font_size_get(Eo *obj, Elm_Code_Widget2_Data *pd);

EOAPI EO_FUNC_BODY(elm_code_widget2_font_size_get, Evas_Font_Size, 0);

void _elm_code_widget2_eo_base_constructor(Eo *obj, Elm_Code_Widget2_Data *pd);


void _elm_code_widget2_evas_object_smart_add(Eo *obj, Elm_Code_Widget2_Data *pd);


void _elm_code_widget2_evas_object_smart_resize(Eo *obj, Elm_Code_Widget2_Data *pd, Evas_Coord w, Evas_Coord h);


void _elm_code_widget2_elm_interface_scrollable_content_pos_set(Eo *obj, Elm_Code_Widget2_Data *pd, Evas_Coord x, Evas_Coord y, Eina_Bool sig);


static Eo_Op_Description _elm_code_widget2_op_desc[] = {
     EO_OP_FUNC_OVERRIDE(eo_constructor, _elm_code_widget2_eo_base_constructor),
     EO_OP_FUNC_OVERRIDE(evas_obj_smart_add, _elm_code_widget2_evas_object_smart_add),
//     EO_OP_FUNC_OVERRIDE(evas_obj_smart_resize, _elm_code_widget2_evas_object_smart_resize),
     EO_OP_FUNC_OVERRIDE(elm_interface_scrollable_content_pos_set, _elm_code_widget2_elm_interface_scrollable_content_pos_set),
     EO_OP_FUNC(elm_code_widget2_font_size_set, _elm_code_widget2_font_size_set, ""),
     EO_OP_FUNC(elm_code_widget2_font_size_get, _elm_code_widget2_font_size_get, ""),
     EO_OP_SENTINEL
};

static const Eo_Event_Description *_elm_code_widget2_event_desc[] = {
     ELM_CODE_WIDGET2_EVENT_FOCUSED,
     ELM_CODE_WIDGET2_EVENT_UNFOCUSED,
     NULL
};

static const Eo_Class_Description _elm_code_widget2_class_desc = {
     EO_VERSION,
     "Elm_Code_Widget2",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(_elm_code_widget2_op_desc),
     _elm_code_widget2_event_desc,
     sizeof(Elm_Code_Widget2_Data),
     _elm_code_widget2_class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_code_widget2_class_get, &_elm_code_widget2_class_desc, ELM_BOX_CLASS, ELM_INTERFACE_SCROLLABLE_MIXIN, ELM_INTERFACE_ATSPI_TEXT_INTERFACE, NULL);
