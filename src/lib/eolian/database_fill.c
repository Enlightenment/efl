#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eo_parser.h"

const Eolian_Class *
_get_impl_class(const Eolian_Class *cl, const char *cln)
{
   if (!cl || !strcmp(cl->full_name, cln))
     return cl;
   Eina_List *l;
   const char *s;
   EINA_LIST_FOREACH(cl->inherits, l, s)
     {
        /* we can do a depth first search, it's easier and doesn't matter
         * which part of the inheritance tree we find the class in
         */
        const Eolian_Class *fcl = _get_impl_class(eolian_class_get_by_name(s), cln);
        if (fcl)
          return fcl;
     }
   return NULL;
}

static void
_print_linecol(const Eolian_Object *base)
{
   fprintf(stderr, "eolian:%s:%d:%d: ", base->file, base->line, base->column);
}

static Eina_Bool
_get_impl_func(Eolian_Class *cl, Eolian_Implement *impl,
               Eolian_Function_Type ftype, Eolian_Function **foo_id)
{
   size_t imlen = strlen(impl->full_name);
   char *clbuf = alloca(imlen + 1);
   memcpy(clbuf, impl->full_name, imlen + 1);

   char *ldot = strrchr(clbuf, '.');
   if (!ldot)
     return EINA_FALSE; /* unreachable in practice, for static analysis */

   *ldot = '\0'; /* split between class name and func name */
   const char *clname = clbuf;
   const char *fnname = ldot + 1;

   const Eolian_Class *tcl = _get_impl_class(cl, clname);
   if (!tcl)
     {
        _print_linecol(&impl->base);
        fprintf(stderr, "class '%s' not found within the inheritance tree of '%s'\n",
                clname, cl->full_name);
        return EINA_FALSE;
     }

   impl->klass = tcl;

   const Eolian_Function *fid = eolian_class_function_get_by_name(tcl, fnname, ftype);
   if (!fid)
     {
        _print_linecol(&impl->base);
        fprintf(stderr, "function '%s' not known in class '%s'\n", fnname, clname);
        return EINA_FALSE;
     }

   if ((fid->klass == cl) && !impl->is_auto && !impl->is_empty)
     {
        /* only allow explicit implements from other classes, besides auto and
         * empty... also prevents pure virtuals from being implemented
         */
        _print_linecol(&impl->base);
        fprintf(stderr, "invalid implement '%s'\n", impl->full_name);
        return EINA_FALSE;
     }

   *foo_id = (Eolian_Function *)fid;
   impl->foo_id = fid;

   return EINA_TRUE;
}


static Eina_Bool
_db_fill_implement(Eolian_Class *cl, Eolian_Implement *impl)
{
   Eolian_Function *foo_id;
   Eolian_Function_Type ftype = EOLIAN_UNRESOLVED;

   if (impl->is_prop_get && impl->is_prop_set)
     ftype = EOLIAN_PROPERTY;
   else if (impl->is_prop_get)
     ftype = EOLIAN_PROP_GET;
   else if (impl->is_prop_set)
     ftype = EOLIAN_PROP_SET;

   if (!_get_impl_func(cl, impl, ftype, &foo_id))
     return EINA_FALSE;

   if (impl->is_auto)
     {
        if (ftype == EOLIAN_PROP_GET)
          {
             foo_id->get_impl = impl;
             foo_id->get_auto = EINA_TRUE;
          }
        else if (ftype == EOLIAN_PROP_SET)
          {
             foo_id->set_impl = impl;
             foo_id->set_auto = EINA_TRUE;
          }
        else
          {
             foo_id->get_impl = foo_id->set_impl = impl;
             foo_id->get_auto = foo_id->set_auto = EINA_TRUE;
          }
     }
   else if (impl->is_empty)
     {
        if (ftype == EOLIAN_PROP_GET)
          {
             foo_id->get_impl = impl;
             foo_id->get_empty = EINA_TRUE;
          }
        else if (ftype == EOLIAN_PROP_SET)
          {
             foo_id->set_impl = impl;
             foo_id->set_empty = EINA_TRUE;
          }
        else
          {
             foo_id->get_impl = foo_id->set_impl = impl;
             foo_id->get_empty = foo_id->set_empty = EINA_TRUE;
          }
     }

   return EINA_TRUE;
}

static void
_db_build_implement(Eolian_Class *cl, Eolian_Function *foo_id)
{
   if (foo_id->type == EOLIAN_PROP_SET)
     {
        if (foo_id->set_impl) return;
     }
   else if (foo_id->type == EOLIAN_PROP_GET)
     {
        if (foo_id->get_impl) return;
     }
   else if (foo_id->get_impl && foo_id->set_impl) return;

   Eolian_Implement *impl = calloc(1, sizeof(Eolian_Implement));

   if (foo_id->type == EOLIAN_PROP_SET)
     impl->base = foo_id->set_base;
   else
     impl->base = foo_id->base;
   eina_stringshare_ref(impl->base.file);

   impl->klass = cl;
   impl->foo_id = foo_id;
   impl->full_name = eina_stringshare_printf("%s.%s", cl->full_name,
                                             foo_id->name);

   if (foo_id->type == EOLIAN_PROPERTY)
     {
        /* FIXME fugly hack, ideally rework the whole implements api altogether */
        if (foo_id->get_virtual_pure && !foo_id->get_impl)
          {
             impl->is_virtual = EINA_TRUE;
             impl->is_prop_get = EINA_TRUE;
             foo_id->get_impl = impl;
             cl->implements = eina_list_append(cl->implements, impl);
             /* repeat for set */
             _db_build_implement(cl, foo_id);
             return;
          }
        else if (foo_id->set_virtual_pure && !foo_id->set_impl)
          {
             impl->is_virtual = EINA_TRUE;
             impl->is_prop_set = EINA_TRUE;
             foo_id->set_impl = impl;
             cl->implements = eina_list_append(cl->implements, impl);
             /* repeat for get */
             _db_build_implement(cl, foo_id);
             return;
          }
        if (foo_id->get_impl)
          {
             impl->is_prop_set = EINA_TRUE;
             impl->is_virtual = foo_id->set_virtual_pure;
             foo_id->set_impl = impl;
          }
        else if (foo_id->set_impl)
          {
             impl->is_prop_get = EINA_TRUE;
             foo_id->get_impl = impl;
          }
        else
          foo_id->get_impl = foo_id->set_impl = impl;
     }
   else if (foo_id->type == EOLIAN_PROP_SET)
     {
        impl->is_prop_set = EINA_TRUE;
        impl->is_virtual = foo_id->get_virtual_pure;
        foo_id->set_impl = impl;
     }
   else if (foo_id->type == EOLIAN_PROP_GET)
     {
        impl->is_prop_get = EINA_TRUE;
        impl->is_virtual = foo_id->set_virtual_pure;
        foo_id->get_impl = impl;
     }
   else
     {
        impl->is_virtual = foo_id->get_virtual_pure;
        foo_id->get_impl = foo_id->set_impl = impl;
     }

   cl->implements = eina_list_append(cl->implements, impl);
}

static Eina_Bool
_db_fill_implements(Eolian_Class *cl)
{
   Eolian_Implement *impl;
   Eolian_Function *foo_id;
   Eina_List *l;

   Eina_Hash *th = eina_hash_string_small_new(NULL);
   EINA_LIST_FOREACH(cl->implements, l, impl)
     {
        if (eina_hash_find(th, impl->full_name))
          {
             _print_linecol(&impl->base);
             fprintf(stderr, "duplicate implement '%s'\n", impl->full_name);
             return EINA_FALSE;
          }
        if (!_db_fill_implement(cl, impl))
          return EINA_FALSE;
        eina_hash_add(th, impl->full_name, impl->full_name);
     }
   eina_hash_free(th);

   EINA_LIST_FOREACH(cl->properties, l, foo_id)
     _db_build_implement(cl, foo_id);

   EINA_LIST_FOREACH(cl->methods, l, foo_id)
     _db_build_implement(cl, foo_id);

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_ctors(Eolian_Class *cl)
{
   Eolian_Constructor *ctor;
   Eina_List *l;

   Eina_Hash *th = eina_hash_string_small_new(NULL);
   EINA_LIST_FOREACH(cl->constructors, l, ctor)
     {
        if (eina_hash_find(th, ctor->full_name))
          {
             _print_linecol(&ctor->base);
             fprintf(stderr, "duplicate ctor '%s'\n", ctor->full_name);
             return EINA_FALSE;
          }
        const char *ldot = strrchr(ctor->full_name, '.');
        if (!ldot)
          return EINA_FALSE;
        char *cnbuf = alloca(ldot - ctor->full_name + 1);
        memcpy(cnbuf, ctor->full_name, ldot - ctor->full_name);
        cnbuf[ldot - ctor->full_name] = '\0';
        const Eolian_Class *tcl = _get_impl_class(cl, cnbuf);
        if (!tcl)
          {
             _print_linecol(&ctor->base);
             fprintf(stderr, "class '%s' not found within the inheritance "
                             "tree of '%s'\n", cnbuf, cl->full_name);
             return EINA_FALSE;
          }
        ctor->klass = tcl;
        eina_hash_add(th, ctor->full_name, ctor->full_name);
     }
   eina_hash_free(th);

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_class(Eolian_Class *cl)
{
   if (!_db_fill_implements(cl))
     return EINA_FALSE;

   if (!_db_fill_ctors(cl))
     return EINA_FALSE;

   eina_hash_set(_classes, cl->full_name, cl);
   eina_hash_set(_classesf, cl->base.file, cl);

   return EINA_TRUE;
}

Eina_Bool
eo_parser_database_fill(const char *filename, Eina_Bool eot)
{
   Eolian_Constructor *ctor;
   Eolian_Implement *impl;
   Eina_Iterator *itr;
   Eolian_Class *cl;
   Eo_Lexer *ls;

   if (eina_hash_find(_parsedeos, filename))
     return EINA_TRUE;

   eina_hash_set(_parsingeos, filename, (void *)EINA_TRUE);

   ls = eo_lexer_new(filename);
   if (!ls)
     {
        fprintf(stderr, "eolian: unable to create lexer for file '%s'\n", filename);
        goto error;
     }

   /* read first token */
   eo_lexer_get(ls);

   if (!eo_parser_walk(ls, eot))
     goto error;

   if (eot) goto done;

   if (!(cl = ls->tmp.kls))
     {
        fprintf(stderr, "eolian: no class for file '%s'\n", filename);
        goto error;
     }
   ls->tmp.kls = NULL;

   if (!_db_fill_class(cl))
     goto error;

   itr = eolian_class_implements_get(cl);
   EINA_ITERATOR_FOREACH(itr, impl)
     {
        Eolian_Function_Type impl_type = EOLIAN_UNRESOLVED;
        const Eolian_Function *impl_func = eolian_implement_function_get(impl, &impl_type);
        if (!impl_func)
          {
             fprintf(stderr, "eolian: unable to find function '%s'\n",
                     eolian_implement_full_name_get(impl));
             eina_iterator_free(itr);
             goto error;
          }
        else if (eolian_function_is_constructor(impl->foo_id, impl->klass))
          database_function_constructor_add((Eolian_Function*)impl->foo_id, cl);
     }
   eina_iterator_free(itr);
   itr = eolian_class_constructors_get(cl);
   EINA_ITERATOR_FOREACH(itr, ctor)
     {
        const Eolian_Function *ctor_func = eolian_constructor_function_get(ctor);
        if (!ctor_func)
          {
             fprintf(stderr, "eolian: unable to find function '%s'\n",
                     eolian_constructor_full_name_get(ctor));
             eina_iterator_free(itr);
             goto error;
          }
        else
          database_function_constructor_add((Eolian_Function*)ctor_func, ctor->klass);
     }
   eina_iterator_free(itr);

done:
   eina_hash_set(_parsedeos, filename, (void *)EINA_TRUE);
   eina_hash_set(_parsingeos, filename, (void *)EINA_FALSE);

   eo_lexer_free(ls);
   return EINA_TRUE;

error:
   eina_hash_set(_parsingeos, filename, (void *)EINA_FALSE);
   eo_lexer_free(ls);
   return EINA_FALSE;
}
