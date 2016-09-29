#include "main.h"
#include "docs.h"

static const char *
_cl_type_str_get(const Eolian_Class *cl, Eina_Bool uc)
{
   switch (eolian_class_type_get(cl))
     {
      case EOLIAN_CLASS_REGULAR:
      case EOLIAN_CLASS_ABSTRACT:
        return uc ? "CLASS" : "class";
      case EOLIAN_CLASS_MIXIN:
        return uc ? "MIXIN" : "mixin";
      case EOLIAN_CLASS_INTERFACE:
        return uc ? "INTERFACE" : "interface";
      default:
        return NULL;
     }
}

static const char *
_get_add_star(Eolian_Function_Type ftype, Eolian_Parameter_Dir pdir)
{
   if (ftype == EOLIAN_PROP_GET)
     return "*";
   if ((pdir == EOLIAN_OUT_PARAM) || (pdir == EOLIAN_INOUT_PARAM))
     return "*";
   return "";
}

static void
_gen_func(const Eolian_Function *fid, Eolian_Function_Type ftype,
          Eina_Strbuf *buf, char *cname, char *cnameu, Eina_Bool legacy)
{
   Eina_Stringshare *fcn = eolian_function_full_c_name_get(fid, ftype, legacy);
   if (!fcn)
     return;

   Eina_Bool var_as_ret = EINA_FALSE;
   const Eolian_Type *rtp = eolian_function_return_type_get(fid, ftype);
   if (ftype == EOLIAN_PROP_GET && !rtp)
     {
        void *d1, *d2;
        Eina_Iterator *itr = eolian_property_values_get(fid, ftype);
        if (eina_iterator_next(itr, &d1) && !eina_iterator_next(itr, &d2))
          {
             rtp = eolian_parameter_type_get((Eolian_Function_Parameter *)d1);
             var_as_ret = EINA_TRUE;
          }
        eina_iterator_free(itr);
     }

   Eolian_Object_Scope fsc = eolian_function_scope_get(fid, ftype);

   /* this one will never be satisfied in legacy */
   if (eolian_function_is_beta(fid))
     eina_strbuf_append_printf(buf, "#ifdef %s_BETA\n", cnameu);
   /* XXX: is this right? we expose potentially internal stuff into legacy */
   if (!legacy && (fsc == EOLIAN_SCOPE_PROTECTED))
     eina_strbuf_append_printf(buf, "#ifdef %s_PROTECTED\n", cnameu);

   Eina_Bool hasdoc = eolian_function_documentation_get(fid, EOLIAN_UNRESOLVED) ||
                      eolian_function_documentation_get(fid, ftype);
   if (hasdoc)
     {
        Eina_Strbuf *dbuf = eo_gen_docs_func_gen(fid, ftype, 0, legacy);
        eina_strbuf_append(buf, eina_strbuf_string_get(dbuf));
        eina_strbuf_append_char(buf, '\n');
        eina_strbuf_free(dbuf);
     }
   eina_strbuf_append(buf, legacy ? "EAPI " : "EOAPI ");
   if (rtp)
     {
        Eina_Stringshare *rtps = eolian_type_c_type_get(rtp);
        eina_strbuf_append(buf, rtps);
        if (rtps[strlen(rtps) - 1] != '*')
          eina_strbuf_append_char(buf, ' ');
        eina_stringshare_del(rtps);
     }
   else
     eina_strbuf_append(buf, "void ");

   eina_strbuf_append(buf, fcn);
   eina_stringshare_del(fcn);

   Eina_Bool first = EINA_TRUE;
   Eina_Strbuf *flagbuf = NULL;
   int nidx = !legacy || !eolian_function_is_class(fid);

   eina_strbuf_append_char(buf, '(');
   if (nidx)
     {
        if ((ftype == EOLIAN_PROP_GET) || eolian_function_object_is_const(fid)
            || eolian_function_is_class(fid))
          {
             eina_strbuf_append(buf, "const ");
          }
        if (legacy)
          eina_strbuf_append_printf(buf, "%s *obj", cname);
        else
          eina_strbuf_append(buf, "Eo *obj");
        first = EINA_FALSE;
     }

   {
      Eolian_Function_Parameter *pr = NULL;
      Eina_Iterator *itr = eolian_property_keys_get(fid, ftype);
      EINA_ITERATOR_FOREACH(itr, pr)
        {
           const Eolian_Type *prt = eolian_parameter_type_get(pr);
           const char *prn = eolian_parameter_name_get(pr);
           Eina_Stringshare *prtn = eolian_type_c_type_get(prt);
           ++nidx;
           if (!first)
             eina_strbuf_append(buf, ", ");
           eina_strbuf_append_printf(buf, "%s %s", prtn, prn);
           eina_stringshare_del(prtn);
           first = EINA_FALSE;
           if (!eolian_parameter_is_nonull(pr))
             continue;
           if (!flagbuf)
             {
                flagbuf = eina_strbuf_new();
                eina_strbuf_append_printf(flagbuf, " EINA_ARG_NONNULL(%d", nidx);
             }
           else
             eina_strbuf_append_printf(flagbuf, ", %d", nidx);
        }
      eina_iterator_free(itr);
   }

   if (!var_as_ret)
     {
        Eina_Iterator *itr = NULL;
        if (ftype == EOLIAN_PROP_GET || ftype == EOLIAN_PROP_SET)
          itr = eolian_property_values_get(fid, ftype);
        else
          itr = eolian_function_parameters_get(fid);

        Eolian_Function_Parameter *pr = NULL;
        EINA_ITERATOR_FOREACH(itr, pr)
          {
             const Eolian_Type *prt = eolian_parameter_type_get(pr);
             const char *prn = eolian_parameter_name_get(pr);
             Eina_Stringshare *prtn = eolian_type_c_type_get(prt);

             ++nidx;
             if (!first)
               eina_strbuf_append(buf, ", ");
             eina_strbuf_append(buf, prtn);
             if (!strchr(prtn, '*'))
               eina_strbuf_append_char(buf, ' ');
             eina_strbuf_append(buf,
                 _get_add_star(ftype, eolian_parameter_direction_get(pr)));
             eina_strbuf_append(buf, prn);
             eina_stringshare_del(prtn);
             first = EINA_FALSE;
             if (!eolian_parameter_is_nonull(pr))
               continue;
             if (!flagbuf)
               {
                  flagbuf = eina_strbuf_new();
                  eina_strbuf_append_printf(flagbuf, " EINA_ARG_NONNULL(%d", nidx);
               }
             else
               eina_strbuf_append_printf(flagbuf, ", %d", nidx);
          }

        eina_iterator_free(itr);
     }

   if (flagbuf)
     eina_strbuf_append_char(flagbuf, ')');

   eina_strbuf_append(buf, ")");
   if (eolian_function_return_is_warn_unused(fid, ftype))
     {
        if (!flagbuf)
          flagbuf = eina_strbuf_new();
        eina_strbuf_prepend(flagbuf, " EINA_WARN_UNUSED_RESULT");
     }
   if (flagbuf)
     {
        eina_strbuf_append(buf, eina_strbuf_string_get(flagbuf));
        eina_strbuf_free(flagbuf);
     }
   eina_strbuf_append(buf, ";\n");

   if (eolian_function_is_beta(fid))
     eina_strbuf_append_printf(buf, "#endif\n");
   if (!legacy && (fsc == EOLIAN_SCOPE_PROTECTED))
     eina_strbuf_append_printf(buf, "#endif\n");
}

void
eo_gen_header_gen(const Eolian_Class *cl, Eina_Strbuf *buf, Eina_Bool legacy)
{
   if (!cl)
     return;

   char *cname = NULL, *cnameu = NULL, *cnamel = NULL;
   eo_gen_class_names_get(cl, &cname, &cnameu, &cnamel);

   /* class definition */

   if (!legacy)
     {
        const Eolian_Documentation *doc = eolian_class_documentation_get(cl);
        if (doc)
          {
             Eina_Strbuf *cdoc = eo_gen_docs_full_gen(doc,
                eolian_class_full_name_get(cl), 0, EINA_FALSE);
             if (cdoc)
               {
                  eina_strbuf_append(buf, eina_strbuf_string_get(cdoc));
                  eina_strbuf_append_char(buf, '\n');
                  eina_strbuf_free(cdoc);
               }
          }
        eina_strbuf_append_printf(buf, "#define %s_%s %s_%s_get()\n\n",
                                  cnameu, _cl_type_str_get(cl, EINA_TRUE),
                                  cnamel, _cl_type_str_get(cl, EINA_FALSE));

        eina_strbuf_append_printf(buf, "EWAPI const Efl_Class *%s_%s_get(void);\n",
                                  cnamel, _cl_type_str_get(cl, EINA_FALSE));
     }

   /* method section */
   {
      Eina_Iterator *itr = eolian_class_implements_get(cl);
      if (!itr)
        goto events;

      const Eolian_Implement *imp;
      EINA_ITERATOR_FOREACH(itr, imp)
        {
           if (eolian_implement_class_get(imp) != cl)
             continue;
           Eolian_Function_Type ftype = EOLIAN_UNRESOLVED;
           const Eolian_Function *fid = eolian_implement_function_get(imp, &ftype);
           /* beta can only exist for eo api */
           if (legacy && eolian_function_is_beta(fid))
             continue;
           eina_strbuf_append_char(buf, '\n');
           switch (ftype)
             {
              case EOLIAN_PROP_GET:
              case EOLIAN_PROP_SET:
                _gen_func(fid, ftype, buf, cname, cnameu, legacy);
                break;
              case EOLIAN_PROPERTY:
                _gen_func(fid, EOLIAN_PROP_SET, buf, cname, cnameu, legacy);
                _gen_func(fid, EOLIAN_PROP_GET, buf, cname, cnameu, legacy);
                break;
              default:
                _gen_func(fid, EOLIAN_UNRESOLVED, buf, cname, cnameu, legacy);
             }
        }
      eina_iterator_free(itr);
   }

events:
   /* event section */
   if (!legacy)
     {
        Eina_Iterator *itr = eolian_class_events_get(cl);
        Eolian_Event *ev;
        EINA_ITERATOR_FOREACH(itr, ev)
          {
             Eina_Stringshare *evn = eolian_event_c_name_get(ev);
             Eolian_Object_Scope evs = eolian_event_scope_get(ev);

             if (evs == EOLIAN_SCOPE_PRIVATE)
               continue;

             if (eolian_event_is_beta(ev))
               {
                  eina_strbuf_append_printf(buf, "\n#ifdef %s_BETA\n", cnameu);
               }
             if (evs == EOLIAN_SCOPE_PROTECTED)
               {
                  if (!eolian_event_is_beta(ev))
                    eina_strbuf_append_char(buf, '\n');
                  eina_strbuf_append_printf(buf, "#ifdef %s_PROTECTED\n", cnameu);
               }

             if (!eolian_event_is_beta(ev) && evs == EOLIAN_SCOPE_PUBLIC)
               eina_strbuf_append_char(buf, '\n');

             eina_strbuf_append_printf(buf, "EOAPI extern const "
                                       "Efl_Event_Description _%s;\n\n", evn);

             Eina_Strbuf *evdbuf = eo_gen_docs_event_gen(ev,
                eolian_class_full_name_get(cl));
             eina_strbuf_append(buf, eina_strbuf_string_get(evdbuf));
             eina_strbuf_append_char(buf, '\n');
             eina_strbuf_free(evdbuf);
             eina_strbuf_append_printf(buf, "#define %s (&(_%s))\n", evn, evn);

             if (evs == EOLIAN_SCOPE_PROTECTED)
               eina_strbuf_append(buf, "#endif\n");
             if (eolian_event_is_beta(ev))
               eina_strbuf_append(buf, "#endif\n");

             eina_stringshare_del(evn);
          }
        eina_iterator_free(itr);
     }

   free(cname);
   free(cnameu);
   free(cnamel);
}
