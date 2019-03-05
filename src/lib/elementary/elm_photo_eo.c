
Efl_Object *_elm_photo_efl_object_constructor(Eo *obj, Elm_Photo_Data *pd);


Efl_Object *_elm_photo_efl_object_finalize(Eo *obj, Elm_Photo_Data *pd);


Eina_Error _elm_photo_efl_file_file_set(Eo *obj, Elm_Photo_Data *pd, const char *file);


const char *_elm_photo_efl_file_file_get(const Eo *obj, Elm_Photo_Data *pd);


void _elm_photo_efl_file_key_set(Eo *obj, Elm_Photo_Data *pd, const char *key);


const char *_elm_photo_efl_file_key_get(const Eo *obj, Elm_Photo_Data *pd);


Eina_Error _elm_photo_efl_file_mmap_set(Eo *obj, Elm_Photo_Data *pd, const Eina_File *f);


const Eina_File *_elm_photo_efl_file_mmap_get(const Eo *obj, Elm_Photo_Data *pd);


Eina_Error _elm_photo_efl_file_load(Eo *obj, Elm_Photo_Data *pd);


Eina_Error _elm_photo_efl_ui_widget_theme_apply(Eo *obj, Elm_Photo_Data *pd);


void _elm_photo_efl_ui_draggable_drag_target_set(Eo *obj, Elm_Photo_Data *pd, Eina_Bool set);


Eina_Bool _elm_photo_efl_ui_draggable_drag_target_get(const Eo *obj, Elm_Photo_Data *pd);


static Eina_Bool
_elm_photo_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_PHOTO_EXTRA_OPS
#define ELM_PHOTO_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_photo_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_finalize, _elm_photo_efl_object_finalize),
      EFL_OBJECT_OP_FUNC(efl_file_set, _elm_photo_efl_file_file_set),
      EFL_OBJECT_OP_FUNC(efl_file_get, _elm_photo_efl_file_file_get),
      EFL_OBJECT_OP_FUNC(efl_file_key_set, _elm_photo_efl_file_key_set),
      EFL_OBJECT_OP_FUNC(efl_file_key_get, _elm_photo_efl_file_key_get),
      EFL_OBJECT_OP_FUNC(efl_file_mmap_set, _elm_photo_efl_file_mmap_set),
      EFL_OBJECT_OP_FUNC(efl_file_mmap_get, _elm_photo_efl_file_mmap_get),
      EFL_OBJECT_OP_FUNC(efl_file_load, _elm_photo_efl_file_load),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_photo_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_draggable_drag_target_set, _elm_photo_efl_ui_draggable_drag_target_set),
      EFL_OBJECT_OP_FUNC(efl_ui_draggable_drag_target_get, _elm_photo_efl_ui_draggable_drag_target_get),
      ELM_PHOTO_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_photo_class_desc = {
   EO_VERSION,
   "Elm.Photo",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Photo_Data),
   _elm_photo_class_initializer,
   _elm_photo_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_photo_class_get, &_elm_photo_class_desc, EFL_UI_WIDGET_CLASS, EFL_FILE_MIXIN, EFL_UI_CLICKABLE_INTERFACE, EFL_UI_DRAGGABLE_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);
