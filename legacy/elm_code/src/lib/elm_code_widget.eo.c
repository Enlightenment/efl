EOAPI const Eo_Event_Description _ELM_CODE_WIDGET_EVENT_LINE_CLICKED =
   EO_EVENT_DESCRIPTION("line,clicked", "");
EOAPI const Eo_Event_Description _ELM_CODE_WIDGET_EVENT_CURSOR_CHANGED =
   EO_EVENT_DESCRIPTION("cursor,changed", "");

void _elm_code_widget_code_set(Eo *obj, Elm_Code_Widget_Data *pd, Elm_Code *code);

EOAPI EO_VOID_FUNC_BODYV(elm_code_widget_code_set, EO_FUNC_CALL(code), Elm_Code *code);

Elm_Code * _elm_code_widget_code_get(Eo *obj, Elm_Code_Widget_Data *pd);

EOAPI EO_FUNC_BODY(elm_code_widget_code_get, Elm_Code *, 0);

void _elm_code_widget_font_size_set(Eo *obj, Elm_Code_Widget_Data *pd, Evas_Font_Size font_size);

EOAPI EO_VOID_FUNC_BODYV(elm_code_widget_font_size_set, EO_FUNC_CALL(font_size), Evas_Font_Size font_size);

Evas_Font_Size _elm_code_widget_font_size_get(Eo *obj, Elm_Code_Widget_Data *pd);

EOAPI EO_FUNC_BODY(elm_code_widget_font_size_get, Evas_Font_Size, 0);

void _elm_code_widget_gravity_set(Eo *obj, Elm_Code_Widget_Data *pd, double x, double y);

EOAPI EO_VOID_FUNC_BODYV(elm_code_widget_gravity_set, EO_FUNC_CALL(x, y), double x, double y);

void _elm_code_widget_gravity_get(Eo *obj, Elm_Code_Widget_Data *pd, double *x, double *y);

EOAPI EO_VOID_FUNC_BODYV(elm_code_widget_gravity_get, EO_FUNC_CALL(x, y), double *x, double *y);

void _elm_code_widget_editable_set(Eo *obj, Elm_Code_Widget_Data *pd, Eina_Bool editable);

EOAPI EO_VOID_FUNC_BODYV(elm_code_widget_editable_set, EO_FUNC_CALL(editable), Eina_Bool editable);

Eina_Bool _elm_code_widget_editable_get(Eo *obj, Elm_Code_Widget_Data *pd);

EOAPI EO_FUNC_BODY(elm_code_widget_editable_get, Eina_Bool, 0);

void _elm_code_widget_line_numbers_set(Eo *obj, Elm_Code_Widget_Data *pd, Eina_Bool line_numbers);

EOAPI EO_VOID_FUNC_BODYV(elm_code_widget_line_numbers_set, EO_FUNC_CALL(line_numbers), Eina_Bool line_numbers);

Eina_Bool _elm_code_widget_line_numbers_get(Eo *obj, Elm_Code_Widget_Data *pd);

EOAPI EO_FUNC_BODY(elm_code_widget_line_numbers_get, Eina_Bool, 0);

void _elm_code_widget_cursor_position_set(Eo *obj, Elm_Code_Widget_Data *pd, unsigned int col, unsigned int line);

EOAPI EO_VOID_FUNC_BODYV(elm_code_widget_cursor_position_set, EO_FUNC_CALL(col, line), unsigned int col, unsigned int line);

void _elm_code_widget_cursor_position_get(Eo *obj, Elm_Code_Widget_Data *pd, unsigned int *col, unsigned int *line);

EOAPI EO_VOID_FUNC_BODYV(elm_code_widget_cursor_position_get, EO_FUNC_CALL(col, line), unsigned int *col, unsigned int *line);

void _elm_code_widget_eo_base_constructor(Eo *obj, Elm_Code_Widget_Data *pd);


void _elm_code_widget_evas_object_smart_add(Eo *obj, Elm_Code_Widget_Data *pd);


Eina_Bool _elm_code_widget_elm_widget_focus_next_manager_is(Eo *obj, Elm_Code_Widget_Data *pd);


Eina_Bool _elm_code_widget_elm_widget_focus_direction_manager_is(Eo *obj, Elm_Code_Widget_Data *pd);


static Eo_Op_Description _elm_code_widget_op_desc[] = {
     EO_OP_FUNC_OVERRIDE(eo_constructor, _elm_code_widget_eo_base_constructor),
     EO_OP_FUNC_OVERRIDE(evas_obj_smart_add, _elm_code_widget_evas_object_smart_add),
     EO_OP_FUNC_OVERRIDE(elm_obj_widget_focus_next_manager_is, _elm_code_widget_elm_widget_focus_next_manager_is),
     EO_OP_FUNC_OVERRIDE(elm_obj_widget_focus_direction_manager_is, _elm_code_widget_elm_widget_focus_direction_manager_is),
     EO_OP_FUNC(elm_code_widget_code_set, _elm_code_widget_code_set, "Set the underlying code object that this widget renders"),
     EO_OP_FUNC(elm_code_widget_code_get, _elm_code_widget_code_get, "Get the underlying code object we are rendering"),
     EO_OP_FUNC(elm_code_widget_font_size_set, _elm_code_widget_font_size_set, "Set the font size that this widget uses, the font will always be a system monospaced font"),
     EO_OP_FUNC(elm_code_widget_font_size_get, _elm_code_widget_font_size_get, "Get the font size currently in use"),
     EO_OP_FUNC(elm_code_widget_gravity_set, _elm_code_widget_gravity_set, "Set how this widget's scroller should respond to new lines being added."),
     EO_OP_FUNC(elm_code_widget_gravity_get, _elm_code_widget_gravity_get, "Get the current x and y gravity of the widget's scroller"),
     EO_OP_FUNC(elm_code_widget_editable_set, _elm_code_widget_editable_set, "Set whether this widget allows editing"),
     EO_OP_FUNC(elm_code_widget_editable_get, _elm_code_widget_editable_get, "Get the current editable state of this widget"),
     EO_OP_FUNC(elm_code_widget_line_numbers_set, _elm_code_widget_line_numbers_set, "Set whether line numbers should be displayed in the left gutter."),
     EO_OP_FUNC(elm_code_widget_line_numbers_get, _elm_code_widget_line_numbers_get, "Get the status of line number display for this widget."),
     EO_OP_FUNC(elm_code_widget_cursor_position_set, _elm_code_widget_cursor_position_set, "Set the current location of the text cursor."),
     EO_OP_FUNC(elm_code_widget_cursor_position_get, _elm_code_widget_cursor_position_get, "Get the current x and y position of the widget's cursor"),
     EO_OP_SENTINEL
};

static const Eo_Event_Description *_elm_code_widget_event_desc[] = {
     ELM_CODE_WIDGET_EVENT_LINE_CLICKED,
     ELM_CODE_WIDGET_EVENT_CURSOR_CHANGED,
     NULL
};

static const Eo_Class_Description _elm_code_widget_class_desc = {
     EO_VERSION,
     "Elm_Code_Widget",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(_elm_code_widget_op_desc),
     _elm_code_widget_event_desc,
     sizeof(Elm_Code_Widget_Data),
     _elm_code_widget_class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_code_widget_class_get, &_elm_code_widget_class_desc, ELM_LAYOUT_CLASS, ELM_INTERFACE_ATSPI_TEXT_INTERFACE, NULL);