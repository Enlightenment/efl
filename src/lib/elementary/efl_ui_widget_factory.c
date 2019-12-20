#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_UI_WIDGET_PROTECTED
#define EFL_PART_PROTECTED
#define EFL_UI_FACTORY_PROTECTED

#include <Efl_Ui.h>
#include "elm_priv.h"
#include "efl_ui_property_bind_part.eo.h"

typedef struct _Efl_Ui_Widget_Factory_Data Efl_Ui_Widget_Factory_Data;
typedef struct _Efl_Ui_Widget_Factory_Request Efl_Ui_Widget_Factory_Request;
typedef struct _Efl_Ui_Bind_Part_Data Efl_Ui_Bind_Part_Data;
typedef struct _Efl_Ui_Property_Bind_Data Efl_Ui_Property_Bind_Data;

struct _Efl_Ui_Property_Bind_Data
{
   Eina_Stringshare *part_property;
   Eina_Stringshare *model_property;
};

struct _Efl_Ui_Bind_Part_Data
{
   Eina_Stringshare *part;

   Eina_List *properties;
};

struct _Efl_Ui_Widget_Factory_Data
{
   const Efl_Class *klass;

   Efl_Ui_Widget *parenting_widget;

   Eina_Hash *parts;

   Eina_Stringshare *default_property;

   Eina_Stringshare *style;
};

struct _Efl_Ui_Widget_Factory_Request
{
   Efl_Ui_Widget_Factory_Data *pd;
   Efl_Ui_Factory *factory;
};

static Efl_Object *
_efl_ui_widget_factory_efl_object_finalize(Eo *obj, Efl_Ui_Widget_Factory_Data *pd)
{
   pd->parenting_widget = efl_provider_find(obj, EFL_UI_WIDGET_CLASS);
   if (!pd->parenting_widget)
     {
        ERR("Widget_Factory requires a Widget as parent.");
        return NULL;
     }

   return efl_finalize(efl_super(obj, EFL_UI_WIDGET_FACTORY_CLASS));
}

Efl_Ui_Win *
efl_ui_widget_factory_widget_get(Efl_Ui_Widget_Factory *factory)
{
   Efl_Ui_Widget_Factory_Data *pd = efl_data_scope_get(factory, EFL_UI_WIDGET_FACTORY_CLASS);

   return pd->parenting_widget;
}

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

static void
_efl_ui_widget_factory_constructing(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Gfx_Entity *ui_view = ev->info;
   const Efl_Model *model;
   Eina_Value *width, *height;

   model = efl_ui_view_model_get(ui_view);

   // Enable recalculate in case we do not know the size of the item
   efl_canvas_group_need_recalculate_set(ui_view, EINA_TRUE);

   // Fetch min size from model if available to avoid recalculcating it
   width = efl_model_property_get(model, "self.width");
   height = efl_model_property_get(model, "self.height");
   if (eina_value_type_get(width) != EINA_VALUE_TYPE_ERROR &&
       eina_value_type_get(height) != EINA_VALUE_TYPE_ERROR)
     {
        Eina_Size2D s;

        if (!eina_value_int_convert(width, &s.w)) s.w = 0;
        if (!eina_value_int_convert(height, &s.h)) s.h = 0;

        efl_gfx_hint_size_min_set(ui_view, s);
        efl_canvas_group_need_recalculate_set(ui_view, EINA_FALSE);
        if (efl_isa(ui_view, EFL_UI_ITEM_CLASS)) efl_ui_item_calc_locked_set(ui_view, EINA_TRUE);
     }
   eina_value_free(width);
   eina_value_free(height);

   efl_key_data_set(ui_view, "efl.ui.widget.factory.size_check", (void*)EINA_TRUE);
}


static void
_efl_ui_widget_factory_building(void *data, const Efl_Event *ev)
{
   Efl_Gfx_Entity *ui_view = ev->info;
   Efl_Ui_Widget_Factory_Data *pd = data;
   const Efl_Model *model;
   Eina_Value *property;
   Efl_Ui_Bind_Part_Data *bpd;
   Eina_Iterator *it;
   char *style;

   model = efl_ui_view_model_get(ui_view);

   // Check property size only if not checked yet
   if (!efl_key_data_get(ui_view, "efl.ui.widget.factory.size_check"))
     _efl_ui_widget_factory_constructing(data, ev);

   // Bind all property before the object is finalize
   it = eina_hash_iterator_data_new(pd->parts);
   EINA_ITERATOR_FOREACH(it, bpd)
     {
        Efl_Ui_Property_Bind_Data *bppd;
        Eina_List *l;

        EINA_LIST_FOREACH(bpd->properties, l, bppd)
          efl_ui_property_bind(efl_part(ui_view, bpd->part),
                               bppd->part_property,
                               bppd->model_property);
     }
   eina_iterator_free(it);

   // As we have already waited for the property to be ready, we should get the right style now
   if (!pd->style) return ;

   property = efl_model_property_get(model, pd->style);
   if (!property) return ;

   style = eina_value_to_string(property);
   if (style) efl_ui_widget_style_set(ui_view, style);
   free(style);

   eina_value_free(property);

   efl_key_data_set(ui_view, "efl.ui.widget.factory.cached", NULL);
}

static void
_efl_ui_widget_factory_releasing(void *data, const Efl_Event *ev)
{
   Efl_Ui_Widget_Factory_Data *pd = data;
   Efl_Gfx_Entity *ui_view = ev->info;
   Efl_Ui_Bind_Part_Data *bpd;
   Eina_Iterator *it;

   efl_key_data_set(ui_view, "efl.ui.widget.factory.size_set", NULL);
   efl_key_data_set(ui_view, "efl.ui.widget.factory.size_check", NULL);
   if (efl_isa(ui_view, EFL_UI_ITEM_CLASS)) efl_ui_item_calc_locked_set(ui_view, EINA_TRUE);

   // Bind default property
   if (pd->default_property) efl_ui_property_bind(ui_view, NULL, pd->default_property);

   // Bind all property before the object is finalize
   it = eina_hash_iterator_data_new(pd->parts);
   EINA_ITERATOR_FOREACH(it, bpd)
     {
        Efl_Ui_Property_Bind_Data *bppd;
        Eina_List *l;

        EINA_LIST_FOREACH(bpd->properties, l, bppd)
          efl_ui_property_bind(efl_part(ui_view, bpd->part),
                               bppd->part_property, NULL);
     }
   eina_iterator_free(it);

   efl_ui_view_model_set(ui_view, NULL);

   // Prevent any recalc to happen when an object is in the cache or during shutdown of the object
   efl_canvas_group_need_recalculate_set(ui_view, EINA_FALSE);
}

EFL_CALLBACKS_ARRAY_DEFINE(item_callbacks,
                           { EFL_UI_FACTORY_EVENT_ITEM_CONSTRUCTING, _efl_ui_widget_factory_constructing },
                           { EFL_UI_FACTORY_EVENT_ITEM_BUILDING, _efl_ui_widget_factory_building },
                           { EFL_UI_FACTORY_EVENT_ITEM_RELEASING, _efl_ui_widget_factory_releasing })

static Eo *
_efl_ui_widget_factory_efl_object_constructor(Efl_Ui_Widget_Factory *obj,
                                              Efl_Ui_Widget_Factory_Data *pd)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_WIDGET_FACTORY_CLASS));

   efl_event_callback_array_add(obj, item_callbacks(), pd);

   return obj;
}

static Efl_Ui_Widget *
_efl_ui_widget_create(const Efl_Ui_Factory *factory,
                      const Efl_Class *klass,
                      Efl_Ui_Widget *parent,
                      Efl_Model *model)
{
   Efl_Ui_Widget *w;

   w = efl_add(klass, parent,
               efl_ui_view_model_set(efl_added, model),
               efl_event_callback_call((Efl_Ui_Factory *) factory, EFL_UI_FACTORY_EVENT_ITEM_CONSTRUCTING, efl_added));
   efl_event_callback_call((Efl_Ui_Factory *) factory, EFL_UI_FACTORY_EVENT_ITEM_BUILDING, w);
   return w;
}

static Eina_Value
_efl_ui_widget_factory_create_then(Eo *model, void *data, const Eina_Value v EINA_UNUSED)
{
   Efl_Ui_Widget_Factory_Request *r = data;
   Efl_Ui_Widget *w;

   w = _efl_ui_widget_create(r->factory, r->pd->klass, r->pd->parenting_widget, model);
   if (!w) return eina_value_error_init(ENOMEM);
   return eina_value_object_init(w);
}

static void
_efl_ui_widget_factory_single_cleanup(Eo *model, void *data EINA_UNUSED, const Eina_Future *dead_future EINA_UNUSED)
{
   efl_unref(model);
}

static void
_efl_ui_widget_factory_create_cleanup(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Ui_Widget_Factory_Request *r = data;

   efl_unref(r->factory);
   free(r);
}

static Eina_Future *
_efl_ui_widget_factory_efl_ui_factory_create(Eo *obj, Efl_Ui_Widget_Factory_Data *pd,
                                             Eina_Iterator *models)
{
   Efl_Ui_Widget_Factory_Request *r = NULL;
   Eina_Future **f = NULL;
   Efl_Model *model = NULL;
   int count = 0;

   if (!pd->klass)
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

   if (!pd->style)
     {
        Efl_Ui_Widget *w = NULL;
        Eina_Value r;
        Evas *e;

        e = evas_object_evas_get(obj);
        evas_event_freeze(e);

        eina_value_array_setup(&r, EINA_VALUE_TYPE_OBJECT, 4);

        EINA_ITERATOR_FOREACH(models, model)
          {
             w = _efl_ui_widget_create(obj, pd->klass, pd->parenting_widget, model);

             if (!w)
               {
                  evas_event_thaw(e);
                  evas_event_thaw_eval(e);
                  return efl_loop_future_rejected(obj, ENOMEM);
               }
             eina_value_array_append(&r, w);
          }
        eina_iterator_free(models);

        evas_event_thaw(e);
        evas_event_thaw_eval(e);

        return efl_loop_future_resolved(obj, r);
     }

   r = calloc(1, sizeof (Efl_Ui_Widget_Factory_Request));
   if (!r) return efl_loop_future_rejected(obj, ENOMEM);

   r->pd = pd;
   r->factory = efl_ref(obj);

   f = calloc(count + 1, sizeof (Eina_Future *));
   if (!f) goto alloc_array_error;

   EINA_ITERATOR_FOREACH(models, model)
     {
        f[count++] = efl_future_then(efl_ref(model), efl_model_property_ready_get(model, pd->style),
                                     .success = _efl_ui_widget_factory_create_then,
                                     .free = _efl_ui_widget_factory_single_cleanup);

        Eina_Future** tmp = realloc(f, (count + 1) * sizeof (Eina_Future *));
        if (!tmp)
          {
            free(f);
            goto alloc_array_error;
          }
        f = tmp;
     }
   eina_iterator_free(models);

   f[count] = EINA_FUTURE_SENTINEL;

   return efl_future_then(obj, eina_future_all_array(f),
                          .data = r,
                          .free = _efl_ui_widget_factory_create_cleanup);

alloc_array_error:
   efl_unref(r->factory);
   free(r);
   eina_iterator_free(models);
   return efl_loop_future_rejected(obj, ENOMEM);
}

static void
_efl_ui_widget_factory_efl_ui_factory_release(Eo *obj,
                                              Efl_Ui_Widget_Factory_Data *pd EINA_UNUSED,
                                              Eina_Iterator *ui_views)
{
   Efl_Gfx_Entity *ui_view;

   EINA_ITERATOR_FOREACH(ui_views, ui_view)
     {
        // There might be multiple call to releasing on the same object as every factory in the
        // inheritance chain can decide to keep it for a time
        efl_event_callback_call(obj, EFL_UI_FACTORY_EVENT_ITEM_RELEASING, ui_view);
        // We do not cache or track this item, just get rid of them asap
        efl_del(ui_view);
     }
   eina_iterator_free(ui_views);
}

Eina_Stringshare *_property_style_ss = NULL;

static Eina_Error
_efl_ui_widget_factory_efl_ui_property_bind_property_bind(Eo *obj, Efl_Ui_Widget_Factory_Data *pd,
                                                          const char *target, const char *property)
{
   if (_property_style_ss == target || !strcmp(target, _property_style_ss))
     {
        eina_stringshare_replace(&pd->style, property);
        efl_event_callback_call(obj, EFL_UI_PROPERTY_BIND_EVENT_PROPERTY_BOUND, (void*) _property_style_ss);
        return 0;
     }

   return EINVAL;
}


typedef struct _Efl_Ui_Property_Bind_Part_Data Efl_Ui_Property_Bind_Part_Data;
struct _Efl_Ui_Property_Bind_Part_Data
{
   Efl_Ui_Widget_Factory_Data *pd;
   Eina_Stringshare *name;
};

static Efl_Object *
_efl_ui_widget_factory_efl_part_part_get(const Eo *obj,
                                         Efl_Ui_Widget_Factory_Data *pd,
                                         const char *name)
{
   Efl_Ui_Property_Bind_Part_Data *ppd;
   Efl_Object *part;

   part = efl_add(EFL_UI_PROPERTY_BIND_PART_CLASS, (Eo*) obj);
   if (!part) return NULL;

   ppd = efl_data_scope_get(part, EFL_UI_PROPERTY_BIND_PART_CLASS);
   ppd->name = eina_stringshare_add(name);
   ppd->pd = pd;

   return part;
}

static void
_efl_ui_property_bind_part_efl_object_destructor(Eo *obj, Efl_Ui_Property_Bind_Part_Data *pd)
{
   eina_stringshare_replace(&pd->name, NULL);

   efl_destructor(efl_super(obj, EFL_UI_PROPERTY_BIND_PART_CLASS));
}

static Eina_Error
_efl_ui_property_bind_part_efl_ui_property_bind_property_bind(Eo *obj EINA_UNUSED,
                                                              Efl_Ui_Property_Bind_Part_Data *pd,
                                                              const char *key,
                                                              const char *property)
{
   Efl_Ui_Bind_Part_Data *bpd;
   Efl_Ui_Property_Bind_Data *bppd;

   if (!pd->pd)
     {
        ERR("Trying to bind part property without specifying which part");
        return ENOENT;
     }

   if (!key)
     {
        eina_stringshare_replace(&pd->pd->default_property, property);
        return 0;
     }

   if (!pd->pd->parts)
     pd->pd->parts = eina_hash_stringshared_new(NULL);

   bpd = eina_hash_find(pd->pd->parts, pd->name);
   if (!bpd)
     {
        bpd = calloc(1, sizeof (Efl_Ui_Bind_Part_Data));
        if (!bpd) return ENOMEM;

        bpd->part = eina_stringshare_ref(pd->name);

        eina_hash_direct_add(pd->pd->parts, bpd->part, bpd);
     }

   bppd = calloc(1, sizeof (Efl_Ui_Property_Bind_Data));
   if (!bppd) return ENOMEM;

   bppd->part_property = eina_stringshare_add(key);
   bppd->model_property = eina_stringshare_add(property);

   bpd->properties = eina_list_append(bpd->properties, bppd);

   efl_event_callback_call(obj, EFL_UI_PROPERTY_BIND_EVENT_PROPERTY_BOUND, (void*) key);

   return 0;
}

static Eina_Error
_efl_ui_widget_factory_efl_ui_factory_bind_factory_bind(Eo *obj EINA_UNUSED,
                                                        Efl_Ui_Widget_Factory_Data *pd EINA_UNUSED,
                                                        const char *key EINA_UNUSED,
                                                        Efl_Ui_Factory *factory EINA_UNUSED)
{
   ERR("Efl.Ui.Widget_Factory doesn't support efl.ui.factory_bind.\n");
   return EINA_ERROR_NOT_IMPLEMENTED;
}

#include "efl_ui_property_bind_part.eo.c"
#include "efl_ui_widget_factory.eo.c"
