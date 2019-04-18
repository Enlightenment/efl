#include "main.h"
#include "docs.h"

static const char *
_get_add_star(Eolian_Function_Type ftype, Eolian_Parameter_Dir pdir)
{
   if (ftype == EOLIAN_PROP_GET)
     return "*";
   if ((pdir == EOLIAN_OUT_PARAM) || (pdir == EOLIAN_INOUT_PARAM))
     return "*";
   return "";
}

static int
_gen_param(Eina_Strbuf *buf, Eolian_Function_Parameter *pr,
           Eolian_Function_Type ftype, int *rpid)
{
   const Eolian_Type *prt = eolian_parameter_type_get(pr);
   const Eolian_Typedecl *ptd = eolian_type_typedecl_get(prt);
   const char *prn = eolian_parameter_name_get(pr);
   Eina_Stringshare *prtn = eolian_type_c_type_get(prt, EOLIAN_C_TYPE_PARAM);

   if (ptd && (eolian_typedecl_type_get(ptd) == EOLIAN_TYPEDECL_FUNCTION_POINTER))
     {
        eina_strbuf_append_printf(buf, "void *%s_data, %s %s, Eina_Free_Cb %s_free_cb",
                                  prn, prtn, prn, prn);
        eina_stringshare_del(prtn);
        *rpid = 1;
        return 3;
     }

   eina_strbuf_append(buf, prtn);
   if (prtn[strlen(prtn) - 1] != '*')
     eina_strbuf_append_char(buf, ' ');
   eina_strbuf_append(buf, _get_add_star(ftype, eolian_parameter_direction_get(pr)));
   eina_strbuf_append(buf, prn);
   eina_stringshare_del(prtn);
   *rpid = 0;
   return 1;
}

void
eo_gen_params(Eina_Iterator *itr, Eina_Strbuf *buf,
              Eina_Strbuf **flagbuf, int *nidx, Eolian_Function_Type ftype)
{
   Eolian_Function_Parameter *pr;
   EINA_ITERATOR_FOREACH(itr, pr)
     {
        int rpid = 0;
        if (*nidx)
          eina_strbuf_append(buf, ", ");
        *nidx += _gen_param(buf, pr, ftype, &rpid);

        if (!eolian_parameter_is_nonull(pr) || !flagbuf)
          continue;

        if (!*flagbuf)
          {
             *flagbuf = eina_strbuf_new();
             eina_strbuf_append_printf(*flagbuf, " EINA_ARG_NONNULL(%d", *nidx - rpid);
          }
        else
          eina_strbuf_append_printf(*flagbuf, ", %d", *nidx - rpid);
     }
   eina_iterator_free(itr);
}

static void
_gen_func(const Eolian_State *state, const Eolian_Function *fid,
          Eolian_Function_Type ftype, Eina_Strbuf *buf, char *cnameu)
{
   Eina_Stringshare *fcn = eolian_function_full_c_name_get(fid, ftype);
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

   if (eolian_function_is_beta(fid))
     eina_strbuf_append(buf, "#ifdef EFL_BETA_API_SUPPORT\n");
   if (fsc == EOLIAN_SCOPE_PROTECTED)
     eina_strbuf_append_printf(buf, "#ifdef %s_PROTECTED\n", cnameu);

   const Eolian_Implement *fimp = eolian_function_implement_get(fid);

   Eina_Bool hasdoc = !!eolian_implement_documentation_get(fimp, ftype);
   if (!hasdoc && ((ftype == EOLIAN_PROP_GET) || (ftype == EOLIAN_PROP_SET)))
     hasdoc = !!eolian_implement_documentation_get(fimp, EOLIAN_PROPERTY);
   if (hasdoc)
     {
        Eina_Strbuf *dbuf = eo_gen_docs_func_gen(state, fid, ftype, 0);
        eina_strbuf_append(buf, eina_strbuf_string_get(dbuf));
        eina_strbuf_append_char(buf, '\n');
        eina_strbuf_free(dbuf);
     }
   eina_strbuf_append(buf, "EOAPI ");
   if (rtp)
     {
        Eina_Stringshare *rtps = eolian_type_c_type_get(rtp, EOLIAN_C_TYPE_RETURN);
        eina_strbuf_append(buf, rtps);
        if (rtps[strlen(rtps) - 1] != '*')
          eina_strbuf_append_char(buf, ' ');
        eina_stringshare_del(rtps);
     }
   else
     eina_strbuf_append(buf, "void ");

   eina_strbuf_append(buf, fcn);
   eina_stringshare_del(fcn);

   Eina_Strbuf *flagbuf = NULL;
   int nidx = !eolian_function_is_class(fid);

   eina_strbuf_append_char(buf, '(');
   if (nidx)
     {
        if ((ftype == EOLIAN_PROP_GET) || eolian_function_object_is_const(fid)
            || eolian_function_is_class(fid))
          {
             eina_strbuf_append(buf, "const ");
          }
        eina_strbuf_append(buf, "Eo *obj");
     }

   eo_gen_params(eolian_property_keys_get(fid, ftype), buf, &flagbuf, &nidx, EOLIAN_PROPERTY);

   if (!var_as_ret)
     {
        Eina_Iterator *itr = NULL;
        if (ftype == EOLIAN_PROP_GET || ftype == EOLIAN_PROP_SET)
          itr = eolian_property_values_get(fid, ftype);
        else
          itr = eolian_function_parameters_get(fid);
        eo_gen_params(itr, buf, &flagbuf, &nidx, ftype);
     }

   if (flagbuf)
     eina_strbuf_append_char(flagbuf, ')');

   /* zero-arg funcs in C need void for arguments */
   if (eina_strbuf_string_get(buf)[eina_strbuf_length_get(buf) - 1] == '(')
     eina_strbuf_append(buf, "void");

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

   if (fsc == EOLIAN_SCOPE_PROTECTED)
     eina_strbuf_append_printf(buf, "#endif\n");
   if (eolian_function_is_beta(fid))
     eina_strbuf_append_printf(buf, "#endif /* EFL_BETA_API_SUPPORT */\n");
}

void
eo_gen_header_gen(const Eolian_State *state, const Eolian_Class *cl,
                  Eina_Strbuf *buf, Eina_Bool legacy)
{
   if (!cl || legacy)
     return;

   Eina_Iterator *itr;
   Eolian_Event *ev;
   char *cnameu = NULL;
   eo_gen_class_names_get(cl, NULL, &cnameu, NULL);

   /* class definition */

   if (eolian_class_is_beta(cl))
     {
        eina_strbuf_append(buf, "#ifdef EFL_BETA_API_SUPPORT\n");
     }
   const Eolian_Documentation *doc = eolian_class_documentation_get(cl);
   if (doc)
     {
        Eina_Strbuf *cdoc = eo_gen_docs_full_gen(state, doc,
           eolian_class_name_get(cl), 0, EINA_FALSE);
        if (cdoc)
          {
             eina_strbuf_append(buf, eina_strbuf_string_get(cdoc));
             eina_strbuf_append_char(buf, '\n');
             eina_strbuf_free(cdoc);
          }
     }

   Eina_Stringshare *mname = eolian_class_c_name_get(cl);
   Eina_Stringshare *gname = eolian_class_c_get_function_name_get(cl);
   eina_strbuf_append_printf(buf, "#define %s %s()\n\n", mname, gname);
   eina_stringshare_del(mname);

   eina_strbuf_append_printf(buf, "EWAPI const Efl_Class *%s(void);\n", gname);
   eina_stringshare_del(gname);

   /* method section */
   itr = eolian_class_implements_get(cl);
   if (!itr)
     goto events;

   const Eolian_Implement *imp;
   EINA_ITERATOR_FOREACH(itr, imp)
     {
        if (eolian_implement_class_get(imp) != cl)
          continue;
        Eolian_Function_Type ftype = EOLIAN_UNRESOLVED;
        const Eolian_Function *fid = eolian_implement_function_get(imp, &ftype);
        eina_strbuf_append_char(buf, '\n');
         switch (ftype)
          {
           case EOLIAN_PROP_GET:
           case EOLIAN_PROP_SET:
             _gen_func(state, fid, ftype, buf, cnameu);
             break;
           case EOLIAN_PROPERTY:
             _gen_func(state, fid, EOLIAN_PROP_SET, buf, cnameu);
             eina_strbuf_append_char(buf, '\n');
              _gen_func(state, fid, EOLIAN_PROP_GET, buf, cnameu);
             break;
           default:
             _gen_func(state, fid, EOLIAN_METHOD, buf, cnameu);
          }
     }
   eina_iterator_free(itr);

events:
   /* event section */
   itr = eolian_class_events_get(cl);
   EINA_ITERATOR_FOREACH(itr, ev)
     {
        Eina_Stringshare *evn = eolian_event_c_name_get(ev);
        Eolian_Object_Scope evs = eolian_event_scope_get(ev);

        if (evs == EOLIAN_SCOPE_PRIVATE)
          continue;

        if (eolian_event_is_beta(ev))
          {
             eina_strbuf_append(buf, "#ifdef EFL_BETA_API_SUPPORT\n");
          }
        if (evs == EOLIAN_SCOPE_PROTECTED)
          {
             if (!eolian_event_is_beta(ev))
               eina_strbuf_append_char(buf, '\n');
             eina_strbuf_append_printf(buf, "#ifdef %s_PROTECTED\n", cnameu);
          }

        if (!eolian_event_is_beta(ev) && evs == EOLIAN_SCOPE_PUBLIC)
          eina_strbuf_append_char(buf, '\n');

        eina_strbuf_append_printf(buf, "EWAPI extern const "
                                  "Efl_Event_Description _%s;\n\n", evn);

        Eina_Strbuf *evdbuf = eo_gen_docs_event_gen(state, ev,
           eolian_class_name_get(cl));
        eina_strbuf_append(buf, eina_strbuf_string_get(evdbuf));
        eina_strbuf_append_char(buf, '\n');
        eina_strbuf_free(evdbuf);
        eina_strbuf_append_printf(buf, "#define %s (&(_%s))\n", evn, evn);

        if (evs == EOLIAN_SCOPE_PROTECTED)
          eina_strbuf_append(buf, "#endif\n");
        if (eolian_event_is_beta(ev))
          eina_strbuf_append(buf, "#endif /* EFL_BETA_API_SUPPORT */\n");

        eina_stringshare_del(evn);
     }
   eina_iterator_free(itr);

   if (eolian_class_is_beta(cl))
     {
        eina_strbuf_append(buf, "#endif /* EFL_BETA_API_SUPPORT */\n");
     }

   free(cnameu);
}
