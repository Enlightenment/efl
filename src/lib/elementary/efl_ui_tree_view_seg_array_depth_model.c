#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "Eina.h"
#include <Efl.h>
#include <Ecore.h>
#include "Eo.h"
#include <assert.h>

#define MY_CLASS EFL_UI_TREE_VIEW_SEG_ARRAY_DEPTH_MODEL_CLASS
#define MY_CLASS_NAME "Efl.Ui.Tree_View_Seg_Array_Depth_Model"

#include "efl_ui_tree_view_seg_array_depth_model.eo.h"
#include "efl_model_accessor_view_private.h"
#include "efl_model_composite_private.h"

typedef struct _Efl_Ui_Tree_View_Seg_Array_Depth_Model_Data
{
   unsigned int depth;
   Eina_Bool readonly;

} Efl_Ui_Tree_View_Seg_Array_Depth_Model_Data;

EOLIAN static Efl_Object *
_efl_ui_tree_view_seg_array_depth_model_efl_object_constructor(Eo *obj, Efl_Ui_Tree_View_Seg_Array_Depth_Model_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->readonly = EINA_FALSE;

   return obj;
}

EOLIAN static Efl_Object *
_efl_ui_tree_view_seg_array_depth_model_efl_object_finalize(Eo *obj, Efl_Ui_Tree_View_Seg_Array_Depth_Model_Data *pd)
{
   pd->readonly = EINA_TRUE;

   return obj;
}

EOLIAN static Eina_Value *
_efl_ui_tree_view_seg_array_depth_model_efl_model_property_get(const Eo *obj,
                                                              Efl_Ui_Tree_View_Seg_Array_Depth_Model_Data *pd,
                                                              const char *property)
{
   if (strcmp("depth", property))
     return efl_model_property_get(efl_super(obj, MY_CLASS), property);

   return eina_value_uint_new(pd->depth);
}

EOLIAN static Eina_Future *
_efl_ui_tree_view_seg_array_depth_model_efl_model_property_set(Eo *obj,
                                                              Efl_Ui_Tree_View_Seg_Array_Depth_Model_Data *pd,
                                                              const char *property, Eina_Value *value)
{
   if (strcmp("depth", property))
     return efl_model_property_set(efl_super(obj, MY_CLASS), property, value);

   if (!pd->readonly && eina_value_uint_get(value, &pd->depth))
     return efl_loop_future_resolved(obj, eina_value_uint_init(pd->depth));
   else
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_READ_ONLY);
}

static Eina_Iterator *
_efl_ui_tree_view_seg_array_depth_model_efl_model_properties_get(const Eo *obj,
                                                              Efl_Ui_Tree_View_Seg_Array_Depth_Model_Data *pd EINA_UNUSED)
{
   EFL_MODEL_COMPOSITE_PROPERTIES_SUPER(props,
                                        obj, MY_CLASS,
                                        NULL,
                                        "depth");
   return props;
}

#include "efl_ui_tree_view_seg_array_depth_model.eo.c"
