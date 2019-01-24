#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_TREE_FACTORY_CLASS
#define MY_CLASS_NAME "Efl.Ui.Tree_Factory"

typedef struct _Efl_Ui_Tree_Factory_Data
{
    Eina_Stringshare *exp_style;
} Efl_Ui_Tree_Factory_Data;

typedef struct _Efl_Ui_Tree_Factory_Request
{
  Efl_Ui_Tree_Factory_Data *pd;
  Efl_Model *model;
} Efl_Ui_Tree_Factory_Request;

EOLIAN static void
_efl_ui_tree_factory_efl_object_destructor(Eo *obj, Efl_Ui_Tree_Factory_Data *pd)
{
   eina_stringshare_del(pd->exp_style);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Value
_efl_ui_tree_factory_connect(Eo *obj, void *data, const Eina_Value value)
{
   Efl_Ui_Tree_Factory_Request *r = data;
   Efl_Ui_Tree_Factory_Data *pd = r->pd;
   Efl_Model *model = r->model;
   Efl_Ui_Tree_Factory_Item_Event evt;

   evt.model = model;
   evt.expandable = EINA_FALSE;

   if (efl_model_children_count_get(model) > 0)
     evt.expandable = EINA_TRUE;

   efl_event_callback_call(obj, EFL_UI_TREE_FACTORY_EVENT_ITEM_CREATE, &evt);

   if (pd->exp_style && evt.expandable)
     {
        Efl_Gfx_Entity *layout;
        eina_value_pget(&value, &layout);
        efl_ui_widget_style_set(layout, pd->exp_style);
     }

   return value;
}

static void
_efl_ui_tree_factory_create_cleanup(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Ui_Tree_Factory_Request *r = data;

   efl_unref(r->model);
   free(r);
}

EOLIAN static Eina_Future *
_efl_ui_tree_factory_efl_ui_factory_create(Eo *obj, Efl_Ui_Tree_Factory_Data *pd
                                                        , Efl_Model *model, Efl_Gfx_Entity *parent)
{
   Efl_Ui_Tree_Factory_Request *r;
   Eina_Future *f;

   r = calloc(1, sizeof (Efl_Ui_Tree_Factory_Request));
   if (!r) return efl_loop_future_rejected(obj, ENOMEM);

   r->pd = pd;
   r->model = efl_ref(model);

   f = efl_ui_factory_create(efl_super(obj, MY_CLASS), model, parent);

   return efl_future_then(obj, f,
                          .success_type = EINA_VALUE_TYPE_OBJECT,
                          .success = _efl_ui_tree_factory_connect,
                          .data = r,
                          .free = _efl_ui_tree_factory_create_cleanup);
}

EOLIAN static void
_efl_ui_tree_factory_expandable_style_set(Eo *obj EINA_UNUSED, Efl_Ui_Tree_Factory_Data *pd, const char *style)
{
   eina_stringshare_replace(&pd->exp_style, style);
}

#include "efl_ui_tree_factory.eo.c"
