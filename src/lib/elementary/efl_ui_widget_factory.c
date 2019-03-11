#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_UI_WIDGET_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Ui_Widget_Factory_Data Efl_Ui_Widget_Factory_Data;
typedef struct _Efl_Ui_Widget_Factory_Request Efl_Ui_Widget_Factory_Request;

struct _Efl_Ui_Widget_Factory_Data
{
   const Efl_Class *klass;

   Eina_Stringshare *style;
};

struct _Efl_Ui_Widget_Factory_Request
{
   Efl_Ui_Widget_Factory_Data *pd;
   Eo *parent;
   Efl_Model *model;
};

static void
_efl_ui_widget_factory_item_class_set(Eo *obj, Efl_Ui_Widget_Factory_Data *pd,
                                      const Efl_Class *klass)
{
   if (!efl_isa(klass, EFL_UI_VIEW_INTERFACE) ||
       !efl_isa(klass, EFL_UI_WIDGET_CLASS))
     {
        ERR("Provided class '%s' for factory '%s' doesn't implement '%s' and '%s' interfaces.",
            efl_class_name_get(klass),
            efl_class_name_get(obj),
            efl_class_name_get(EFL_UI_WIDGET_CLASS),
            efl_class_name_get(EFL_UI_VIEW_INTERFACE));
        return ;
     }
   pd->klass = klass;
}

static const Efl_Class *
_efl_ui_widget_factory_item_class_get(const Eo *obj EINA_UNUSED,
                                      Efl_Ui_Widget_Factory_Data *pd)
{
   return pd->klass;
}

static Eina_Value
_efl_ui_widget_factory_create_then(Eo *obj EINA_UNUSED, void *data, const Eina_Value v)
{
   Efl_Ui_Widget_Factory_Request *r = data;
   Efl_Ui_Widget *w;
   const char *string = NULL;

   if (!eina_value_string_get(&v, &string))
     return eina_value_error_init(EFL_MODEL_ERROR_NOT_SUPPORTED);

   w = efl_add(r->pd->klass, r->parent,
               efl_ui_widget_style_set(efl_added, string),
               efl_ui_view_model_set(efl_added, r->model));

   return eina_value_object_init(w);
}

static void
_efl_ui_widget_factory_create_cleanup(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Ui_Widget_Factory_Request *r = data;

   efl_unref(r->model);
   efl_unref(r->parent);
   free(r);
}

static Eina_Future *
_efl_ui_widget_factory_efl_ui_factory_create(Eo *obj, Efl_Ui_Widget_Factory_Data *pd,
                                             Efl_Model *model, Efl_Gfx_Entity *parent)
{
   Efl_Ui_Widget_Factory_Request *r;

   if (!pd->klass)
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

   if (!pd->style)
     {
        Efl_Ui_Widget *w;

        w = efl_add(pd->klass, parent,
                    efl_ui_view_model_set(efl_added, model));
        return efl_loop_future_resolved(obj, eina_value_object_init(w));
     }

   r = calloc(1, sizeof (Efl_Ui_Widget_Factory_Request));
   if (!r) return efl_loop_future_rejected(obj, ENOMEM);

   r->pd = pd;
   r->parent = efl_ref(parent);
   r->model = efl_ref(model);

   return efl_future_then(obj, efl_model_property_ready_get(obj, pd->style),
                          .success = _efl_ui_widget_factory_create_then,
                          .data = r,
                          .free = _efl_ui_widget_factory_create_cleanup);
}

static void
_efl_ui_widget_factory_efl_ui_factory_release(Eo *obj EINA_UNUSED,
                                              Efl_Ui_Widget_Factory_Data *pd EINA_UNUSED,
                                              Efl_Gfx_Entity *ui_view)
{
   // We do not cache or track this item, just get rid of them asap
   efl_del(ui_view);
}

static Eina_Error
_efl_ui_widget_factory_efl_ui_property_bind_property_bind(Eo *obj, Efl_Ui_Widget_Factory_Data *pd,
                                                          const char *target, const char *property)
{
   if (!strcmp(target, "style"))
     {
        eina_stringshare_replace(&pd->style, property);
        return 0;
     }

   return efl_ui_property_bind(efl_super(obj, EFL_UI_WIDGET_FACTORY_CLASS), target, property);
}

#include "efl_ui_widget_factory.eo.c"
