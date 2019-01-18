#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Eolian_Aux.h"

static void
_hashlist_free(void *ptr)
{
   eina_list_free((Eina_List *)ptr);
}

EAPI Eina_Hash *
eolian_aux_state_class_children_find(const Eolian_State *state)
{
   if (!state)
      return NULL;

   Eina_Hash *h = eina_hash_pointer_new(_hashlist_free);
   Eina_Iterator *itr = eolian_state_classes_get(state);
   if (!itr)
      return h;

   const Eolian_Class *cl;
   EINA_ITERATOR_FOREACH(itr, cl)
     {
        const Eolian_Class *icl = eolian_class_parent_get(cl);
        if (icl)
          eina_hash_set(h, &icl, eina_list_append(eina_hash_find(h, &icl), cl));
        Eina_Iterator *iitr = eolian_class_extensions_get(cl);
        EINA_ITERATOR_FOREACH(iitr, icl)
          {
             eina_hash_set(h, &icl,
               eina_list_append(eina_hash_find(h, &icl), cl));
          }
        eina_iterator_free(iitr);
     }
   eina_iterator_free(itr);

   return h;
}

static size_t
_callables_find_body(const Eolian_Class *pcl,
                     Eina_List **funcs, Eina_List **events,
                     Eina_Hash *written)
{
   Eina_Iterator *iitr;
   const Eolian_Implement *imp;
   const Eolian_Event *ev;
   size_t total = 0;

   if (!funcs)
     goto justevs;

   iitr = eolian_class_implements_get(pcl);
   EINA_ITERATOR_FOREACH(iitr, imp)
     {
        const Eolian_Function *ifn =
          eolian_implement_function_get(imp, NULL);
        if (eina_hash_find(written, &ifn))
          continue;
        *funcs = eina_list_append(*funcs, imp);
        ++total;
     }
   eina_iterator_free(iitr);

   if (!events)
     return total;
justevs:
   iitr = eolian_class_events_get(pcl);
   EINA_ITERATOR_FOREACH(iitr, ev)
     {
        /* events do not override */
        *events = eina_list_append(*events, ev);
        ++total;
     }
   eina_iterator_free(iitr);
   return total;
}

static size_t
_callables_find(const Eolian_Class *cl, Eina_List **funcs,
                Eina_List **events, Eina_Hash *written)
{
   size_t total = 0;
   if (!funcs && !events)
     return total;

   const Eolian_Class *pcl = eolian_class_parent_get(cl);
   if (pcl)
     total += _callables_find_body(pcl, funcs, events, written);

   Eina_Iterator *itr = eolian_class_extensions_get(cl);
   EINA_ITERATOR_FOREACH(itr, pcl)
     total += _callables_find_body(pcl, funcs, events, written);

   return total;
}

EAPI size_t
eolian_aux_class_callables_get(const Eolian_Class *klass,
                               Eina_List **funcs, Eina_List **events,
                               size_t *ownfuncs, size_t *ownevs)
{
   size_t of = 0, oe = 0, total = 0;
   if (!klass || (!funcs && !events))
     {
        if (ownfuncs) *ownfuncs = of;
        if (ownevs) *ownevs = oe;
        return total;
     }

   Eina_Hash *written = eina_hash_pointer_new(NULL);
   if (funcs)
     {
        const Eolian_Implement *imp;
        Eina_Iterator *itr = eolian_class_implements_get(klass);
        EINA_ITERATOR_FOREACH(itr, imp)
          {
             const Eolian_Function *ifn =
               eolian_implement_function_get(imp, NULL);
             eina_hash_set(written, &ifn, ifn);
             *funcs = eina_list_append(*funcs, imp);
             ++of;
          }
        eina_iterator_free(itr);
     }
   if (events)
     {
        const Eolian_Event *ev;
        Eina_Iterator *itr = eolian_class_events_get(klass);
        EINA_ITERATOR_FOREACH(itr, ev)
          {
             /* no need to mark in written, events do not override */
             *events = eina_list_append(*events, ev);
             ++oe;
          }
     }
   if (ownfuncs) *ownfuncs = of;
   if (ownevs) *ownevs = oe;
   total = of + oe;
   total += _callables_find(klass, funcs, events, written);
   eina_hash_free(written);
   return total;
}

static void
_all_impls_find(Eina_List **l, const Eolian_Class *cl,
                const Eolian_Function *func, Eina_Hash *got,
                Eina_Hash *children)
{
   if (eina_hash_find(got, &cl))
     return;
   eina_hash_add(got, &cl, cl);
   Eina_Iterator *itr = eolian_class_implements_get(cl);
   const Eolian_Implement *imp;
   EINA_ITERATOR_FOREACH(itr, imp)
     {
        const Eolian_Function *ofn = eolian_implement_function_get(imp, NULL);
        if (ofn == func)
          {
             *l = eina_list_append(*l, imp);
             break;
          }
     }
   eina_iterator_free(itr);
   Eina_List *tl;
   const Eolian_Class *icl;
   EINA_LIST_FOREACH(eina_hash_find(children, &cl), tl, icl)
     _all_impls_find(l, icl, func, got, children);
}

EAPI Eina_List *
eolian_aux_function_all_implements_get(const Eolian_Function *func,
                                       Eina_Hash *class_children)
{
   if (!class_children)
     return NULL;

   const Eolian_Class *cl = eolian_implement_class_get(
     eolian_function_implement_get(func));

   Eina_List *ret = NULL;
   Eina_Hash *got = eina_hash_pointer_new(NULL);
   _all_impls_find(&ret, cl, func, got, class_children);

   eina_hash_free(got);
   return ret;
}

static const Eolian_Implement * _parent_impl_find(
   const char *fulln, const Eolian_Class *cl);

static const Eolian_Implement *
_parent_impl_find_body(const Eolian_Class *icl, const char *fulln)
{
   Eina_Iterator *iitr = eolian_class_implements_get(icl);
   const Eolian_Implement *iimpl;
   EINA_ITERATOR_FOREACH(iitr, iimpl)
     {
        if (eolian_implement_name_get(iimpl) == fulln)
          {
             eina_iterator_free(iitr);
             return iimpl;
          }
     }
   eina_iterator_free(iitr);
   return _parent_impl_find(fulln, icl);
 }

static const Eolian_Implement *
_parent_impl_find(const char *fulln, const Eolian_Class *cl)
{
   const Eolian_Implement *iret = NULL;
   const Eolian_Class *icl = eolian_class_parent_get(cl);
   if (icl)
     iret = _parent_impl_find_body(icl, fulln);
   if (iret)
     return iret;
   Eina_Iterator *itr = eolian_class_extensions_get(cl);
   EINA_ITERATOR_FOREACH(itr, icl)
     {
        iret = _parent_impl_find_body(icl, fulln);
        if (iret)
          {
             eina_iterator_free(itr);
             return iret;
          }
     }
   eina_iterator_free(itr);
   return NULL;
}

EAPI const Eolian_Implement *
eolian_aux_implement_parent_get(const Eolian_Implement *impl)
{
   return _parent_impl_find(eolian_implement_name_get(impl),
                            eolian_implement_implementing_class_get(impl));
}

static const Eolian_Documentation *
_parent_documentation_find(const Eolian_Implement *impl,
                           Eolian_Function_Type ftype)
{
   const Eolian_Implement *pimpl = eolian_aux_implement_parent_get(impl);
   if (!pimpl)
     return NULL;

   const Eolian_Documentation *pdoc =
     eolian_implement_documentation_get(pimpl, ftype);
   if (!pdoc)
     return _parent_documentation_find(pimpl, ftype);

   return pdoc;
}

EAPI const Eolian_Documentation *
eolian_aux_implement_documentation_get(const Eolian_Implement *impl,
                                    Eolian_Function_Type ftype)
{
   const Eolian_Documentation *ret =
     eolian_implement_documentation_get(impl, ftype);

   if (ret)
     return ret;

   const Eolian_Class *icl = eolian_implement_implementing_class_get(impl);
   if (eolian_implement_class_get(impl) == icl)
     return NULL;

   const Eolian_Implement *oimp = eolian_function_implement_get(
     eolian_implement_function_get(impl, NULL));
   if ((ftype == EOLIAN_PROP_GET) && !eolian_implement_is_prop_get(oimp))
     return NULL;
   if ((ftype == EOLIAN_PROP_SET) && !eolian_implement_is_prop_set(oimp))
     return NULL;

   return _parent_documentation_find(impl, ftype);
}

EAPI const Eolian_Documentation *
eolian_aux_implement_documentation_fallback_get(const Eolian_Implement *impl)
{
   Eina_Bool ig = eolian_implement_is_prop_get(impl),
             is = eolian_implement_is_prop_set(impl);

   if (ig && !is)
     return eolian_implement_documentation_get(impl, EOLIAN_PROP_GET);
   else if (is && !ig)
     return eolian_implement_documentation_get(impl, EOLIAN_PROP_SET);

   return NULL;
}
