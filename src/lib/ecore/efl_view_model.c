#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>
#include <Eina.h>
#include <Eo.h>
#include <Ecore.h>

#include "ecore_internal.h"

#include "efl_composite_model_private.h"

typedef struct _Efl_View_Model_Data Efl_View_Model_Data;
typedef struct _Efl_View_Model_Bind Efl_View_Model_Bind;
typedef struct _Efl_View_Model_Text Efl_View_Model_Text;
typedef struct _Efl_View_Model_Logic Efl_View_Model_Logic;
typedef struct _Efl_View_Model_Property_Ref Efl_View_Model_Property_Ref;

struct _Efl_View_Model_Data
{
   // FIXME: If parent is set, always access parent... recursively?
   Efl_View_Model_Data *parent;

   Eina_Hash *bound; // Stringhash of Efl_View_Model_Bind
   Eina_Hash *logics; // Stringhash of Efl_View_Model_Logic
   Eina_Hash *texts; // Stringhash of Efl_View_Model_Text

   Eina_Hash *deduplication; // Stringhash of Efl_View_Model_Property_Ref

   struct {
      Eina_Bool property_changed : 1;
      Eina_Bool child_added : 1;
      Eina_Bool child_removed : 1;
   } propagating; // Boolean to prevent reentrance event emission on the same object
   Eina_Bool finalized : 1;
   Eina_Bool children_bind : 1; // Define if child object should be automatically binded
};

struct _Efl_View_Model_Text
{
   Eina_Stringshare *name;
   Eina_Stringshare *definition;
   Eina_Stringshare *not_ready;
   Eina_Stringshare *on_error;
   Efl_Model *self;
};

struct _Efl_View_Model_Bind
{
   Eina_Stringshare *source;
   Eina_List *destinations;
};

struct _Efl_View_Model_Logic
{
   struct {
      EflViewModelPropertyGet fct;
      Eina_Free_Cb free_cb;
      void *data;
   } get;
   struct {
      EflViewModelPropertySet fct;
      Eina_Free_Cb free_cb;
      void *data;
   } set;

   Efl_Object *object;
   Eina_List *sources;
   Eina_Stringshare *property;
};

struct _Efl_View_Model_Property_Ref
{
   EINA_REFCOUNT;
   Eina_Stringshare *property;
};

static void
_ref_free(void *data)
{
   Efl_View_Model_Property_Ref *r = data;

   eina_stringshare_del(r->property);
   free(r);
}

static void
_ref_add(Efl_View_Model_Data *pd, Eina_Stringshare *property)
{
   Efl_View_Model_Property_Ref *r;

   r = eina_hash_find(pd->deduplication, property);
   if (!r)
     {
        r = calloc(1, sizeof (Efl_View_Model_Property_Ref));
        if (!r) return ;
        r->property = eina_stringshare_ref(property);

        eina_hash_direct_add(pd->deduplication, r->property, r);
     }

   EINA_REFCOUNT_REF(r);
}

static void
_ref_del(Efl_View_Model_Data *pd, Eina_Stringshare *property)
{
   Efl_View_Model_Property_Ref *r;

   r = eina_hash_find(pd->deduplication, property);
   if (!r) return ;

   EINA_REFCOUNT_UNREF(r)
     eina_hash_del(pd->deduplication, property, r);
}

static void
_logic_free(void *data)
{
   Efl_View_Model_Logic *logic = data;
   Eina_Stringshare *source;

   if (logic->get.free_cb) logic->get.free_cb(logic->get.data);
   if (logic->set.free_cb) logic->set.free_cb(logic->set.data);
   EINA_LIST_FREE(logic->sources, source)
     {
        efl_view_model_property_unbind(logic->object, source, logic->property);
        eina_stringshare_del(source);
     }
   eina_stringshare_del(logic->property);
   free(logic);
}

static Eina_Value *
_efl_view_model_property_dummy_get(void *data EINA_UNUSED,
                                   const Efl_View_Model *view_model EINA_UNUSED,
                                   Eina_Stringshare *property EINA_UNUSED)
{
   return eina_value_error_new(EFL_MODEL_ERROR_NOT_SUPPORTED);
}

static Eina_Future *
_efl_view_model_property_dummy_set(void *data EINA_UNUSED,
                                   Efl_View_Model *view_model,
                                   Eina_Stringshare *property EINA_UNUSED,
                                   Eina_Value *value EINA_UNUSED)
{
   return efl_loop_future_rejected(view_model, EFL_MODEL_ERROR_READ_ONLY);
}

static Eina_Error
_efl_view_model_property_logic_add(Eo *obj, Efl_View_Model_Data *pd,
                                   const char *property,
                                   void *get_data, EflViewModelPropertyGet get, Eina_Free_Cb get_free_cb,
                                   void *set_data, EflViewModelPropertySet set, Eina_Free_Cb set_free_cb,
                                   Eina_Iterator *bound)
{
   Efl_View_Model_Logic *logic;
   Eina_Stringshare *prop;
   const char *source;

   prop = eina_stringshare_add(property);

   if (eina_hash_find(pd->logics, prop))
     {
        eina_stringshare_del(prop);
        return EFL_MODEL_ERROR_INCORRECT_VALUE;
     }

   logic = calloc(1, sizeof (Efl_View_Model_Logic));
   if (!logic) return ENOMEM;

   logic->object = obj;
   logic->property = prop;
   logic->get.fct = get ? get : _efl_view_model_property_dummy_get;
   logic->get.free_cb = get_free_cb;
   logic->get.data = get_data;
   logic->set.fct = set ? set : _efl_view_model_property_dummy_set;
   logic->set.free_cb = set_free_cb;
   logic->set.data = set_data;

   eina_hash_direct_add(pd->logics, prop, logic);

   EINA_ITERATOR_FOREACH(bound, source)
     {
        logic->sources = eina_list_append(logic->sources, eina_stringshare_add(source));
        efl_view_model_property_bind(obj, source, property);
     }

   return 0;
}

static Eina_Error
_efl_view_model_property_logic_del(Eo *obj EINA_UNUSED, Efl_View_Model_Data *pd,
                                   const char *property)
{
   Efl_View_Model_Logic *logic;

   logic = eina_hash_find(pd->logics, property);
   if (!logic) return EFL_MODEL_ERROR_INCORRECT_VALUE;
   eina_hash_del(pd->logics, property, logic);
   return 0;
}

static int
_lookup_next_token(const char *definition,
                   Eina_Slstr **text,
                   Eina_Slstr **property)
{
   const char *lookup_text;
   const char *lookup_property;

   if (!definition) return 0;

   *text = NULL;
   *property = NULL;

   lookup_text = strstr(definition, "${");
   if (!lookup_text) goto on_error;
   lookup_text += 2;

   lookup_property = strchr(lookup_text, '}');
   if (!lookup_property) goto on_error;

   *text = eina_slstr_copy_new_length(definition, lookup_text - definition - 2);
   *property = eina_slstr_copy_new_length(lookup_text, lookup_property - lookup_text);

   return lookup_property + 1 - definition;

 on_error:
   if (strlen(definition) == 0) return 0;
   *text = eina_slstr_copy_new(definition);
   return strlen(definition);
}

static Eina_Error
_efl_view_model_property_string_add(Eo *obj, Efl_View_Model_Data *pd,
                                    const char *name,
                                    const char *definition,
                                    const char *not_ready,
                                    const char *on_error)
{
   Efl_View_Model_Text *text;
   Eina_Stringshare *sn;
   Eina_Slstr *st = NULL;
   Eina_Slstr *sp = NULL;
   int lookup;
   Eina_Error err = ENOMEM;

   if (!name || !definition) return EFL_MODEL_ERROR_INCORRECT_VALUE;
   if (!strlen(name)) return EFL_MODEL_ERROR_INCORRECT_VALUE;
   if (!strlen(definition)) return EFL_MODEL_ERROR_INCORRECT_VALUE;
   sn = eina_stringshare_add(name);

   // Lookup if there is an existing property defined and undo it first
   text = eina_hash_find(pd->texts, sn);
   if (text) efl_view_model_property_string_del(obj, sn);

   text = calloc(1, sizeof (Efl_View_Model_Text));
   if (!text) goto on_error;

   err = EFL_MODEL_ERROR_INCORRECT_VALUE;

   text->name = eina_stringshare_add(name);
   text->definition = eina_stringshare_add(definition);
   text->not_ready = not_ready ? eina_stringshare_add(not_ready) : NULL;
   text->on_error = on_error ? eina_stringshare_add(on_error) : NULL;
   text->self = obj;

   for (lookup = _lookup_next_token(definition, &st, &sp);
        lookup;
        definition += lookup, lookup = _lookup_next_token(definition, &st, &sp))
     {
        if (sp) efl_view_model_property_bind(obj, sp, name);
     }

   for (lookup = _lookup_next_token(not_ready, &st, &sp);
        lookup;
        not_ready += lookup, lookup = _lookup_next_token(not_ready, &st, &sp))
     {
        if (sp) efl_view_model_property_bind(obj, sp, name);
     }

   for (lookup = _lookup_next_token(on_error, &st, &sp);
        lookup;
        on_error += lookup, lookup = _lookup_next_token(on_error, &st, &sp))
     {
        if (sp) efl_view_model_property_bind(obj, sp, name);
     }

   eina_hash_direct_add(pd->texts, text->name, text);

   return 0;

 on_error:
   eina_stringshare_del(sn);
   free(text);
   return err;
}

static void
_text_free(void *data)
{
   Efl_View_Model_Text *text = data;
   Eina_Stringshare *st;
   Eina_Stringshare *sp;
   int lookup;
   const char *tmp;

   tmp = text->definition;
   for (lookup = _lookup_next_token(tmp, &st, &sp);
        lookup;
        tmp += lookup, lookup = _lookup_next_token(tmp, &st, &sp))
     {
        if (sp) efl_view_model_property_unbind(text->self, sp, text->name);
     }

   tmp = text->not_ready;
   for (lookup = _lookup_next_token(tmp, &st, &sp);
        lookup;
        tmp += lookup, lookup = _lookup_next_token(tmp, &st, &sp))
     {
        if (sp) efl_view_model_property_unbind(text->self, sp, text->name);
     }

   tmp = text->on_error;
   for (lookup = _lookup_next_token(tmp, &st, &sp);
        lookup;
        tmp += lookup, lookup = _lookup_next_token(tmp, &st, &sp))
     {
        if (sp) efl_view_model_property_unbind(text->self, sp, text->name);
     }

   eina_stringshare_del(text->name);
   eina_stringshare_del(text->not_ready);
   eina_stringshare_del(text->on_error);
   free(text);
}

static Eina_Error
_efl_view_model_property_string_del(Eo *obj EINA_UNUSED,
                                    Efl_View_Model_Data *pd,
                                    const char *name)
{
   Efl_View_Model_Text *text;
   Eina_Stringshare *sn;
   Eina_Error err = EFL_MODEL_ERROR_INCORRECT_VALUE;

   if (!name) return EFL_MODEL_ERROR_INCORRECT_VALUE;

   sn = eina_stringshare_add(name);
   text = eina_hash_find(pd->texts, sn);
   if (!text) goto on_error;
   eina_hash_del(pd->texts, sn, text);
   err = 0;

 on_error:
   eina_stringshare_del(sn);
   return err;
}

static void
_efl_view_model_property_bind(Eo *obj EINA_UNUSED, Efl_View_Model_Data *pd,
                              const char *source, const char *destination)
{
   Efl_View_Model_Bind *bind;
   Eina_Stringshare *src;
   Eina_Stringshare *dst;

   if (!source || !destination) return ;

   src = eina_stringshare_add(source);
   bind = eina_hash_find(pd->bound, src);
   if (!bind)
     {
        bind = calloc(1, sizeof (Efl_View_Model_Bind));
        if (!bind) goto on_error;
        bind->source = eina_stringshare_ref(src);

        eina_hash_direct_add(pd->bound, bind->source, bind);
     }

   dst = eina_stringshare_add(destination);
   bind->destinations = eina_list_append(bind->destinations, dst);
   _ref_add(pd, dst);

 on_error:
   eina_stringshare_del(src);
}

static void
_efl_view_model_property_unbind(Eo *obj EINA_UNUSED, Efl_View_Model_Data *pd,
                                const char *source, const char *destination)
{
   Efl_View_Model_Bind *bind;
   Eina_Stringshare *src;
   Eina_Stringshare *dst;
   Eina_Stringshare *cmp;
   Eina_List *l;

   if (!source || !destination) return ;
   src = eina_stringshare_add(source);
   bind = eina_hash_find(pd->bound, src);
   if (!bind) goto on_error;

   dst = eina_stringshare_add(destination);

   EINA_LIST_FOREACH(bind->destinations, l, cmp)
     if (cmp == dst)
       {
          bind->destinations = eina_list_remove_list(bind->destinations, l);
          break;
       }

   if (!bind->destinations)
     eina_hash_del(pd->bound, dst, bind);

   _ref_del(pd, dst);
   eina_stringshare_del(dst);

 on_error:
   eina_stringshare_del(src);
}

static void
_bind_free(void *data)
{
   Efl_View_Model_Bind *bind = data;
   Eina_Stringshare *dst;

   eina_stringshare_del(bind->source);

   EINA_LIST_FREE(bind->destinations, dst)
     eina_stringshare_del(dst);

   free(bind);
}

static void
_efl_view_model_property_bind_lookup(Eina_Array *changed_properties,
                                     Efl_View_Model_Data *pd,
                                     Eina_Stringshare *src)
{
   Efl_View_Model_Bind *bind;

   if (!pd) return ;
   bind = eina_hash_find(pd->bound, src);
   if (bind)
     {
        Eina_Stringshare *dest;
        Eina_List *l;

        EINA_LIST_FOREACH(bind->destinations, l, dest)
          {
             // Check for duplicated entry first to avoid infinite recursion
             Eina_Stringshare *dup = NULL;
             Eina_Array_Iterator iterator;
             unsigned int i;

             EINA_ARRAY_ITER_NEXT(changed_properties, i, dup, iterator)
               if (dup == dest) break;
             if (dup == dest) continue ;

             eina_array_push(changed_properties, dest);
             _efl_view_model_property_bind_lookup(changed_properties, pd, dest);
          }
     }
   _efl_view_model_property_bind_lookup(changed_properties, pd->parent, src);
}

static void
_efl_view_model_property_changed(void *data, const Efl_Event *event)
{
   Efl_View_Model_Data *pd = data;
   Efl_Model_Property_Event *ev = event->info;
   Efl_Model_Property_Event nev = { 0 };
   const char *property;
   Eina_Stringshare *src;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (pd->propagating.property_changed) return ;
   pd->propagating.property_changed = EINA_TRUE;

   // Our strategy is to rebuild a new Property_Event and cancel the current one.
   efl_event_callback_stop(event->object);

   nev.changed_properties = eina_array_new(1);

   EINA_ARRAY_ITER_NEXT(ev->changed_properties, i, property, iterator)
     {
        eina_array_push(nev.changed_properties, property);

        src = eina_stringshare_ref(property);
        _efl_view_model_property_bind_lookup(nev.changed_properties, pd, src);
     }

   efl_event_callback_call(event->object, EFL_MODEL_EVENT_PROPERTIES_CHANGED, &nev);

   eina_array_free(nev.changed_properties);

   pd->propagating.property_changed = EINA_FALSE;
}

static void
_efl_view_model_children_bind_set(Eo *obj EINA_UNUSED, Efl_View_Model_Data *pd, Eina_Bool enable)
{
   if (pd->finalized) return;

   pd->children_bind = enable;
}

static Eina_Bool
_efl_view_model_children_bind_get(const Eo *obj EINA_UNUSED, Efl_View_Model_Data *pd)
{
   return pd->children_bind;
}

static void
_efl_view_model_parent_data(Efl_View_Model *child, Efl_View_Model_Data *ppd)
{
   Efl_View_Model_Data *cpd;

   cpd = efl_data_scope_get(child, EFL_VIEW_MODEL_CLASS);
   cpd->parent = ppd;
   cpd->propagating = ppd->propagating;
}

static Efl_View_Model *
_efl_view_model_child_lookup(Efl_View_Model_Data *pd, Efl_Object *parent, Efl_Model *view)
{
   EFL_COMPOSITE_LOOKUP_RETURN(co, parent, view, "_efl.view_model");

   co = efl_add(EFL_VIEW_MODEL_CLASS, parent,
                efl_ui_view_model_set(efl_added, view),
                _efl_view_model_parent_data(efl_added, pd));
   if (!co) return NULL;

   EFL_COMPOSITE_REMEMBER_RETURN(co, view);
}

static void
_efl_view_model_child_added(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event *ev = event->info;
   Efl_Model_Children_Event nevt = { 0 };
   Efl_View_Model_Data *pd = data;
   Efl_View_Model *co;

   if (pd->propagating.child_added) return ;
   if (!pd->children_bind) return;
   if (!ev->child) return;

   pd->propagating.child_added = EINA_TRUE;

   // Our strategy is to rebuild a new Child_Add and cancel the current one.
   efl_event_callback_stop(event->object);

   co = _efl_view_model_child_lookup(pd, event->object, ev->child);
   if (!co) return;

   nevt.index = ev->index;
   nevt.child = co;

   efl_event_callback_call(event->object, EFL_MODEL_EVENT_CHILD_ADDED, &nevt);

   pd->propagating.child_added = EINA_FALSE;
}

static void
_efl_view_model_child_removed(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event *ev = event->info;
   Efl_Model_Children_Event nevt = { 0 };
   Efl_View_Model_Data *pd = data;
   Efl_View_Model *co;

   if (pd->propagating.child_removed) return ;
   if (!pd->children_bind) return;
   if (!ev->child) return;

   pd->propagating.child_removed = EINA_TRUE;

   // Our strategy is to rebuild a new Child_Add and cancel the current one.
   efl_event_callback_stop(event->object);

   co = _efl_view_model_child_lookup(pd, event->object, ev->child);
   if (!co) return;

   nevt.index = ev->index;
   nevt.child = co;

   efl_event_callback_call(event->object, EFL_MODEL_EVENT_CHILD_REMOVED, &nevt);

   // The object is being destroyed, there is no point in us keeping the ViewModel proxy alive.
   efl_del(co);

   pd->propagating.child_removed = EINA_FALSE;
}

EFL_CALLBACKS_ARRAY_DEFINE(efl_view_model_intercept,
                           { EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_view_model_property_changed },
                           { EFL_MODEL_EVENT_CHILD_ADDED, _efl_view_model_child_added },
                           { EFL_MODEL_EVENT_CHILD_REMOVED, _efl_view_model_child_removed })

static Efl_Object *
_efl_view_model_efl_object_constructor(Eo *obj, Efl_View_Model_Data *pd)
{
   obj = efl_constructor(efl_super(obj, EFL_VIEW_MODEL_CLASS));

   pd->children_bind = EINA_TRUE;
   pd->bound = eina_hash_stringshared_new(_bind_free);
   pd->logics = eina_hash_stringshared_new(_logic_free);
   pd->deduplication = eina_hash_stringshared_new(_ref_free);
   pd->texts = eina_hash_stringshared_new(_text_free);

   efl_event_callback_array_priority_add(obj, efl_view_model_intercept(), EFL_CALLBACK_PRIORITY_BEFORE, pd);

   return obj;
}

static Efl_Object *
_efl_view_model_efl_object_finalize(Eo *obj, Efl_View_Model_Data *pd)
{
   pd->finalized = EINA_TRUE;

   return efl_finalize(efl_super(obj, EFL_VIEW_MODEL_CLASS));
}

static void
_efl_view_model_efl_object_destructor(Eo *obj, Efl_View_Model_Data *pd)
{
   efl_event_callback_array_del(obj, efl_view_model_intercept(), pd);

   eina_hash_free(pd->bound);
   pd->bound = NULL;

   eina_hash_free(pd->logics);
   pd->logics = NULL;

   eina_hash_free(pd->texts);
   pd->texts = NULL;

   eina_hash_free(pd->deduplication);
   pd->deduplication = NULL;

   efl_destructor(efl_super(obj, EFL_VIEW_MODEL_CLASS));
}

static Efl_View_Model_Logic *
_efl_view_model_property_logic_lookup(Efl_View_Model_Data *pd, Eina_Stringshare *property)
{
   Efl_View_Model_Logic *logic;

   if (!pd) return NULL;
   logic = eina_hash_find(pd->logics, property);
   if (!logic) return _efl_view_model_property_logic_lookup(pd->parent, property);
   return logic;
}

static Eina_Future *
_efl_view_model_efl_model_property_set(Eo *obj, Efl_View_Model_Data *pd,
                                       const char *property, Eina_Value *value)
{
   Efl_View_Model_Logic *logic;
   Eina_Stringshare *prop;
   Eina_Future *f;

   prop = eina_stringshare_add(property);
   logic = _efl_view_model_property_logic_lookup(pd, prop);
   if (logic)
     f = logic->set.fct(logic->get.data, obj, prop, value);
   else
     {
        if (eina_hash_find(pd->texts, prop))
          f = efl_loop_future_rejected(obj, EFL_MODEL_ERROR_READ_ONLY);
        else
          f = efl_model_property_set(efl_super(obj, EFL_VIEW_MODEL_CLASS), property, value);
     }

   eina_stringshare_del(prop);
   return f;
}

static Eina_Value *
_efl_view_model_text_generate(const Eo *obj,
                              Eina_Strbuf *out,
                              Eina_Stringshare *pattern,
                              Eina_Bool stop_on_error)
{
   Eina_Stringshare *st;
   Eina_Stringshare *sp;
   int lookup;

   for (lookup = _lookup_next_token(pattern, &st, &sp);
        lookup;
        pattern += lookup, lookup = _lookup_next_token(pattern, &st, &sp))
     {
        Eina_Value *request;
        char *sr;

        eina_strbuf_append(out, st);

        if (!sp) continue;

        request = efl_model_property_get(obj, sp);
        if (!request)
          {
             if (stop_on_error)
               return eina_value_error_new(EFL_MODEL_ERROR_NOT_SUPPORTED);
             eina_strbuf_append(out, "Unknown property");
             continue;
          }
        if (eina_value_type_get(request) == EINA_VALUE_TYPE_ERROR && stop_on_error)
          return request;

        sr = eina_value_to_string(request);
        eina_strbuf_append(out, sr);

        free(sr);
        eina_value_free(request);
     }

   return eina_value_string_new(eina_strbuf_string_get(out));
}

static Eina_Value *
_efl_view_model_text_property_get(const Eo *obj, Efl_View_Model_Data *pd, Eina_Stringshare *prop)
{
   Efl_View_Model_Text *lookup;
   Eina_Strbuf *buf;
   Eina_Value *r;
   Eina_Error err = 0;

   if (!pd) return NULL;
   lookup = eina_hash_find(pd->texts, prop);
   // Lookup for property definition in the parent, but property value will be fetched on
   // the child object doing the request.
   if (!lookup) return _efl_view_model_text_property_get(obj, pd->parent, prop);

   buf = eina_strbuf_new();

   r = _efl_view_model_text_generate(obj, buf,
                                     lookup->definition,
                                     !!(lookup->on_error || lookup->not_ready));
   if (eina_value_type_get(r) != EINA_VALUE_TYPE_ERROR)
     goto done;
   if (eina_value_error_get(r, &err) && err == EAGAIN && lookup->not_ready)
     {
        eina_strbuf_reset(buf);
        eina_value_free(r);

        r = _efl_view_model_text_generate(obj, buf, lookup->not_ready, !!lookup->on_error);
        if (eina_value_type_get(r) != EINA_VALUE_TYPE_ERROR)
          goto done;
     }
   if (lookup->on_error)
     {
        eina_strbuf_reset(buf);
        eina_value_free(r);

        r = _efl_view_model_text_generate(obj, buf, lookup->on_error, 0);
     }

 done:
   eina_strbuf_free(buf);

   return r;
}

static Eina_Value *
_efl_view_model_efl_model_property_get(const Eo *obj, Efl_View_Model_Data *pd,
                                       const char *property)
{
   Efl_View_Model_Logic *logic;
   Eina_Stringshare *prop;
   Eina_Value *r;

   prop = eina_stringshare_add(property);
   logic = _efl_view_model_property_logic_lookup(pd, prop);
   if (logic)
     r = logic->get.fct(logic->get.data, obj, prop);
   else
     {
        r = _efl_view_model_text_property_get(obj, pd, prop);
        if (!r) r = efl_model_property_get(efl_super(obj, EFL_VIEW_MODEL_CLASS), property);
     }

   eina_stringshare_del(prop);
   return r;
}

static Eina_Iterator *
_efl_view_model_efl_model_properties_get(const Eo *obj, Efl_View_Model_Data *pd)
{
   EFL_COMPOSITE_MODEL_PROPERTIES_SUPER(props, obj, EFL_VIEW_MODEL_CLASS,
                                        eina_hash_iterator_key_new(pd->deduplication));

   return props;
}

typedef struct _Efl_View_Model_Slice_Request Efl_View_Model_Slice_Request;
struct _Efl_View_Model_Slice_Request
{
   Efl_View_Model_Data *pd;
   unsigned int start;
};

static Eina_Value
_efl_view_model_slice_then(Eo *o, void *data, const Eina_Value v)
{
   Efl_View_Model_Slice_Request *req = data;
   Eo *target;
   Eina_Value r = EINA_VALUE_EMPTY;
   unsigned int i, len;

   eina_value_array_setup(&r, EINA_VALUE_TYPE_OBJECT, 4);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, target)
     {
        Eo *composite;

        composite = _efl_view_model_child_lookup(req->pd, o, target);
        eina_value_array_append(&r, composite);
     }

   return r;
}

static void
_efl_view_model_slice_clean(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   free(data);
}

static Eina_Future *
_efl_view_model_efl_model_children_slice_get(Eo *obj, Efl_View_Model_Data *pd,
                                             unsigned int start, unsigned int count)
{
   Efl_View_Model_Slice_Request *req;
   Eina_Future *f;

   f = efl_model_children_slice_get(efl_super(obj, EFL_VIEW_MODEL_CLASS), start, count);

   req = malloc(sizeof (Efl_View_Model_Slice_Request));
   if (!req)
     {
        eina_future_cancel(f);
        return efl_loop_future_rejected(obj, ENOMEM);
     }

   req->pd = pd;
   req->start = start;

   return efl_future_then(obj, f, .success_type = EINA_VALUE_TYPE_ARRAY,
                          .success = _efl_view_model_slice_then,
                          .free = _efl_view_model_slice_clean,
                          .data = req);
}

#include "efl_view_model.eo.c"
