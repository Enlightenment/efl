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
_db_fill_implement(Eolian_Class *cl, Eolian_Implement *impl)
{
   Eolian_Function_Type ftype = EOLIAN_METHOD;

   if (impl->is_prop_get && impl->is_prop_set)
     ftype = EOLIAN_PROPERTY;
   else if (impl->is_prop_get)
     ftype = EOLIAN_PROP_GET;
   else if (impl->is_prop_set)
     ftype = EOLIAN_PROP_SET;

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

   const Eolian_Function *fid = eolian_class_function_get_by_name(tcl, fnname, EOLIAN_UNRESOLVED);
   if (!fid)
     {
        _print_linecol(&impl->base);
        fprintf(stderr, "function '%s' not known in class '%s'\n", fnname, clname);
        return EINA_FALSE;
     }

   Eolian_Function_Type aftype = eolian_function_type_get(fid);

   Eina_Bool auto_empty = (impl->get_auto || impl->get_empty);

   /* match implement type against function type */
   if (ftype == EOLIAN_PROPERTY)
     {
        /* property */
        if (aftype != EOLIAN_PROPERTY)
          {
             _print_linecol(&impl->base);
             fprintf(stderr, "function '%s' is not a complete property", fnname);
             return EINA_FALSE;
          }
        auto_empty = auto_empty && (impl->set_auto || impl->set_empty);
     }
   else if (ftype == EOLIAN_PROP_SET)
     {
        /* setter */
        if ((aftype != EOLIAN_PROP_SET) && (aftype != EOLIAN_PROPERTY))
          {
             _print_linecol(&impl->base);
             fprintf(stderr, "function '%s' doesn't have a setter\n", fnname);
             return EINA_FALSE;
          }
        auto_empty = (impl->set_auto || impl->set_empty);
     }
   else if (ftype == EOLIAN_PROP_GET)
     {
        /* getter */
        if ((aftype != EOLIAN_PROP_GET) && (aftype != EOLIAN_PROPERTY))
          {
             _print_linecol(&impl->base);
             fprintf(stderr, "function '%s' doesn't have a getter\n", fnname);
             return EINA_FALSE;
          }
     }
   else if (aftype != EOLIAN_METHOD)
     {
        _print_linecol(&impl->base);
        fprintf(stderr, "function '%s' is not a method\n", fnname);
        return EINA_FALSE;
     }

   if ((fid->klass == cl) && !auto_empty)
     {
        /* only allow explicit implements from other classes, besides auto and
         * empty... also prevents pure virtuals from being implemented
         */
        _print_linecol(&impl->base);
        fprintf(stderr, "invalid implement '%s'\n", impl->full_name);
        return EINA_FALSE;
     }

   impl->foo_id = fid;

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_implements(Eolian_Class *cl)
{
   Eolian_Implement *impl;
   Eina_List *l;

   Eina_Bool ret = EINA_TRUE;

   Eina_Hash *th = eina_hash_string_small_new(NULL),
             *pth = eina_hash_string_small_new(NULL);
   EINA_LIST_FOREACH(cl->implements, l, impl)
     {
        Eina_Bool prop = (impl->is_prop_get || impl->is_prop_set);
        if (eina_hash_find(prop ? pth : th, impl->full_name))
          {
             _print_linecol(&impl->base);
             fprintf(stderr, "duplicate implement '%s'\n", impl->full_name);
             ret = EINA_FALSE;
             goto end;
          }
        if (impl->klass != cl)
          {
             if (!_db_fill_implement(cl, impl))
               {
                  ret = EINA_FALSE;
                  goto end;
               }
             if (eolian_function_is_constructor(impl->foo_id, impl->klass))
               database_function_constructor_add((Eolian_Function *)impl->foo_id, cl);
          }
        if ((impl->klass != cl) && !_db_fill_implement(cl, impl))
          {
             ret = EINA_FALSE;
             goto end;
          }
        eina_hash_add(prop ? pth : th, impl->full_name, impl->full_name);
     }

end:
   eina_hash_free(th);
   eina_hash_free(pth);
   return ret;
}

static Eina_Bool
_db_fill_ctors(Eolian_Class *cl)
{
   Eolian_Constructor *ctor;
   Eina_List *l;

   Eina_Bool ret = EINA_TRUE;

   Eina_Hash *th = eina_hash_string_small_new(NULL);
   EINA_LIST_FOREACH(cl->constructors, l, ctor)
     {
        if (eina_hash_find(th, ctor->full_name))
          {
             _print_linecol(&ctor->base);
             fprintf(stderr, "duplicate ctor '%s'\n", ctor->full_name);
             ret = EINA_FALSE;
             goto end;
          }
        const char *ldot = strrchr(ctor->full_name, '.');
        if (!ldot)
          {
             ret = EINA_FALSE;
             goto end;
          }
        char *cnbuf = alloca(ldot - ctor->full_name + 1);
        memcpy(cnbuf, ctor->full_name, ldot - ctor->full_name);
        cnbuf[ldot - ctor->full_name] = '\0';
        const Eolian_Class *tcl = _get_impl_class(cl, cnbuf);
        if (!tcl)
          {
             _print_linecol(&ctor->base);
             fprintf(stderr, "class '%s' not found within the inheritance "
                             "tree of '%s'\n", cnbuf, cl->full_name);
             ret = EINA_FALSE;
             goto end;
          }
        ctor->klass = tcl;
        const Eolian_Function *cfunc = eolian_constructor_function_get(ctor);
        if (!cfunc)
          {
             _print_linecol(&ctor->base);
             fprintf(stderr, "unable to find function '%s'\n", ctor->full_name);
             ret = EINA_FALSE;
             goto end;
          }
        database_function_constructor_add((Eolian_Function *)cfunc, tcl);
        eina_hash_add(th, ctor->full_name, ctor->full_name);
     }

end:
   eina_hash_free(th);
   return ret;
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
   if (eina_hash_find(_parsedeos, filename))
     return EINA_TRUE;

   eina_hash_set(_parsingeos, filename, (void *)EINA_TRUE);

   Eo_Lexer *ls = eo_lexer_new(filename);
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

   Eolian_Class *cl;

   if (!(cl = ls->tmp.kls))
     {
        fprintf(stderr, "eolian: no class for file '%s'\n", filename);
        goto error;
     }
   ls->tmp.kls = NULL;

   if (!_db_fill_class(cl))
     goto error;

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
