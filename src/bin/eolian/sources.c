#include "main.h"
#include "docs.h"

/* Used to store the function names that will have to be appended
 * with __eolian during C generation. Needed when params have to
 * be initialized and for future features.
 */
static Eina_Hash *_funcs_params_init = NULL;

static const char *
_get_add_star(Eolian_Function_Type ftype, Eolian_Parameter_Dir pdir)
{
   if (ftype == EOLIAN_PROP_GET)
     return "*";
   if ((pdir == EOLIAN_OUT_PARAM) || (pdir == EOLIAN_INOUT_PARAM))
     return "*";
   return "";
}

static Eina_Bool
_function_exists(const char *fname, Eina_Strbuf *buf)
{
   const char *ptr = eina_strbuf_string_get(buf);
   size_t flen = strlen(fname);
   while ((ptr = strstr(ptr, fname)) != NULL)
     {
        switch (*(ptr - 1))
          {
           case '\n':
           case ' ':
             switch (*(ptr + flen))
               {
                case ' ':
                case '(':
                  return EINA_TRUE;
               }
          }
        ++ptr;
     }
   return EINA_FALSE;
}

/* Check if the type is used in the file, not if it is a typedef... */
static Eina_Bool
_type_exists(const char *tname, Eina_Strbuf *buf)
{
   const char *ptr = eina_strbuf_string_get(buf);
   size_t tlen = strlen(tname);
   while ((ptr = strstr(ptr, tname)) != NULL)
     {
        switch (*(ptr - 1))
          {
           case '\n':
           case ' ':
           case ',':
             switch (*(ptr + tlen))
               {
                case '\n':
                case ' ':
                case ',':
                case ';':
                  return EINA_TRUE;
               }
          }
        ++ptr;
     }
   return EINA_FALSE;
}

static void
_gen_func(const Eolian_Class *cl, const Eolian_Function *fid,
          Eolian_Function_Type ftype, Eina_Strbuf *buf,
          const Eolian_Implement *impl, Eina_Strbuf *lbuf)
{
   Eina_Bool is_empty = eolian_implement_is_empty(impl);
   Eina_Bool is_auto = eolian_implement_is_auto(impl);

   if ((ftype != EOLIAN_PROP_GET) && (ftype != EOLIAN_PROP_SET))
     ftype = eolian_function_type_get(fid);

   Eina_Bool is_prop = (ftype == EOLIAN_PROP_GET || ftype == EOLIAN_PROP_SET);
   Eina_Bool var_as_ret = EINA_FALSE;

   const Eolian_Expression *def_ret = NULL;
   const Eolian_Type *rtp = eolian_function_return_type_get(fid, ftype);
   if (rtp)
     {
        is_auto = EINA_FALSE; /* can't do auto if func returns */
        def_ret = eolian_function_return_default_value_get(fid, ftype);
     }

   const char *func_suffix = "";
   if (ftype == EOLIAN_PROP_GET)
     {
        func_suffix = "_get";
        if (!rtp)
          {
             void *d1, *d2;
             Eina_Iterator *itr = eolian_property_values_get(fid, ftype);
             if (eina_iterator_next(itr, &d1) && !eina_iterator_next(itr, &d2))
               {
                  Eolian_Function_Parameter *pr = d1;
                  rtp = eolian_parameter_type_get(pr);
                  var_as_ret = EINA_TRUE;
                  def_ret = eolian_parameter_default_value_get(pr);
               }
             eina_iterator_free(itr);
          }
     }
   else if (ftype == EOLIAN_PROP_SET)
     func_suffix = "_set";

   Eina_Strbuf *params = eina_strbuf_new(); /* par1, par2, par3, ... */
   Eina_Strbuf *params_full = eina_strbuf_new(); /* T par1, U par2, ... for decl */
   Eina_Strbuf *params_full_imp = eina_strbuf_new(); /* as above, for impl */
   Eina_Strbuf *params_init = eina_strbuf_new(); /* default value inits */

   Eina_Stringshare *promise_param_name = NULL;
   Eina_Stringshare *promise_param_type = NULL;

   /* property keys */
   {
      Eina_Iterator *itr = eolian_property_keys_get(fid, ftype);
      Eolian_Function_Parameter *pr;
      EINA_ITERATOR_FOREACH(itr, pr)
        {
           const char *prn = eolian_parameter_name_get(pr);
           const Eolian_Type *pt = eolian_parameter_type_get(pr);
           Eina_Stringshare *ptn = eolian_type_c_type_get(pt);

           if (eina_strbuf_length_get(params))
             eina_strbuf_append(params, ", ");
           eina_strbuf_append(params, prn);

           eina_strbuf_append_printf(params_full, ", %s", ptn);
           eina_strbuf_append_printf(params_full_imp, ", %s", ptn);
           if (ptn[strlen(ptn) - 1] != '*')
             {
                eina_strbuf_append_char(params_full, ' ');
                eina_strbuf_append_char(params_full_imp, ' ');
             }
           eina_strbuf_append(params_full, prn);
           eina_strbuf_append(params_full_imp, prn);
           if (is_empty || is_auto)
             eina_strbuf_append(params_full_imp, " EINA_UNUSED");

           eina_stringshare_del(ptn);
        }
      eina_iterator_free(itr);
   }

   /* property values or method params if applicable */
   if (!var_as_ret)
     {
        Eina_Iterator *itr;
        if (is_prop)
          itr = eolian_property_values_get(fid, ftype);
        else
          itr = eolian_function_parameters_get(fid);
        Eolian_Function_Parameter *pr;
        EINA_ITERATOR_FOREACH(itr, pr)
          {
             Eolian_Parameter_Dir pd = eolian_parameter_direction_get(pr);
             const Eolian_Expression *dfv = eolian_parameter_default_value_get(pr);
             const char *prn = eolian_parameter_name_get(pr);
             const Eolian_Type *pt = eolian_parameter_type_get(pr);
             Eina_Stringshare *ptn = eolian_type_c_type_get(pt);

             Eina_Bool had_star = ptn[strlen(ptn) - 1] == '*';
             const char *add_star = _get_add_star(ftype, pd);

             if (eina_strbuf_length_get(params))
               eina_strbuf_append(params, ", ");

             eina_strbuf_append(params_full_imp, ", ");
             eina_strbuf_append(params_full_imp, ptn);
             if (!had_star)
               eina_strbuf_append_char(params_full_imp, ' ');
             eina_strbuf_append(params_full_imp, add_star);
             eina_strbuf_append(params_full_imp, prn);
             if (!dfv && is_empty)
               eina_strbuf_append(params_full_imp, " EINA_UNUSED");
             eina_strbuf_append(params, prn);

             eina_strbuf_append(params_full, ", ");
             eina_strbuf_append(params_full, ptn);
             if (!had_star)
               eina_strbuf_append_char(params_full, ' ');
             eina_strbuf_append(params_full, add_star);
             eina_strbuf_append(params_full, prn);

             if (is_auto)
               {
                  if (ftype == EOLIAN_PROP_SET)
                    eina_strbuf_append_printf(params_init, "   %s = pd->%s;\n", prn, prn);
                  else
                    {
                       eina_strbuf_append_printf(params_init, "   if (%s) *%s = pd->%s;\n",
                                                 prn, prn, prn);
                    }
               }
             else if ((ftype != EOLIAN_PROP_SET) && dfv)
               {
                  Eolian_Value val = eolian_expression_eval(dfv, EOLIAN_MASK_ALL);
                  if (val.type)
                    {
                       Eina_Stringshare *vals = eolian_expression_value_to_literal(&val);
                       eina_strbuf_append_printf(params_init, "   if (%s) *%s = %s;",
                                                 prn, prn, vals);
                       eina_stringshare_del(vals);
                       if (eolian_expression_type_get(dfv) == EOLIAN_EXPR_NAME)
                         {
                            Eina_Stringshare *vs = eolian_expression_serialize(dfv);
                            eina_strbuf_append_printf(params_init, " /* %s */", vs);
                            eina_stringshare_del(vs);
                         }
                       eina_strbuf_append_char(params_init, '\n');
                    }
               }

             eina_stringshare_del(ptn);
          }
        eina_iterator_free(itr);
     }

   Eina_Bool impl_same_class = (eolian_implement_class_get(impl) == cl);
   Eina_Bool impl_need = EINA_TRUE;
   if (impl_same_class && eolian_function_is_virtual_pure(fid, ftype))
     impl_need = EINA_FALSE;

   Eina_Stringshare *rtpn = rtp ? eolian_type_c_type_get(rtp)
                                : eina_stringshare_add("void");

   char *cname = NULL, *cnamel = NULL, *ocnamel = NULL;
   eo_gen_class_names_get(cl, &cname, NULL, &cnamel);
   eo_gen_class_names_get(eolian_implement_class_get(impl), NULL, NULL, &ocnamel);

   if (impl_need)
     {
        /* figure out the data type */
        Eina_Stringshare *dt = NULL;
        if (eolian_function_is_class(fid))
          dt = eina_stringshare_add("void");
        else
          dt = eolian_class_c_data_type_get(cl);

        eina_strbuf_append_char(buf, '\n');
        /* no need for prototype with empty/auto impl */
        if (!is_empty && !is_auto)
          {
             /* T _class_name[_orig_class]_func_name_suffix */
             eina_strbuf_append(buf, rtpn);
             if (rtpn[strlen(rtpn) - 1] != '*')
               eina_strbuf_append_char(buf, ' ');
             eina_strbuf_append_char(buf, '_');
             eina_strbuf_append(buf, cnamel);
             if (!impl_same_class)
               eina_strbuf_append_printf(buf, "_%s", ocnamel);
             eina_strbuf_append_char(buf, '_');
             eina_strbuf_append(buf, eolian_function_name_get(fid));
             eina_strbuf_append(buf, func_suffix);
             /* ([const ]Eo *obj, Data_Type *pd, impl_full_params); */
             eina_strbuf_append_char(buf, '(');
             if (eolian_function_object_is_const(fid))
               eina_strbuf_append(buf, "const ");
             eina_strbuf_append(buf, "Eo *obj, ");
             eina_strbuf_append(buf, dt);
             eina_strbuf_append(buf, " *pd");
             eina_strbuf_append(buf, eina_strbuf_string_get(params_full_imp));
             eina_strbuf_append(buf, ");\n\n");
          }

        if (is_empty || is_auto || eina_strbuf_length_get(params_init))
          {
             /* we need to give the internal function name to Eo,
              * use this hash table as indication
              */
             eina_hash_add(_funcs_params_init, &impl, impl);
             /* generation of intermediate __eolian_... */
             eina_strbuf_append(buf, "static ");
             eina_strbuf_append(buf, rtpn);
             if (rtpn[strlen(rtpn) - 1] != '*')
               eina_strbuf_append_char(buf, ' ');
             eina_strbuf_append(buf, "__eolian_");
             eina_strbuf_append(buf, cnamel);
             if (!impl_same_class)
               eina_strbuf_append_printf(buf, "_%s", ocnamel);
             eina_strbuf_append_char(buf, '_');
             eina_strbuf_append(buf, eolian_function_name_get(fid));
             eina_strbuf_append(buf, func_suffix);
             eina_strbuf_append_char(buf, '(');
             if (eolian_function_object_is_const(fid))
               eina_strbuf_append(buf, "const ");
             eina_strbuf_append(buf, "Eo *obj");
             if (is_empty || is_auto)
               eina_strbuf_append(buf, " EINA_UNUSED");
             eina_strbuf_append(buf, ", ");
             eina_strbuf_append(buf, dt);
             eina_strbuf_append(buf, " *pd");
             if (is_empty || (is_auto && !eina_strbuf_length_get(params_init)))
               eina_strbuf_append(buf, " EINA_UNUSED");
             eina_strbuf_append(buf, eina_strbuf_string_get(params_full_imp));
             eina_strbuf_append(buf, ")\n{\n");
          }
        if (eina_strbuf_length_get(params_init))
          eina_strbuf_append(buf, eina_strbuf_string_get(params_init));
        if (is_empty || is_auto)
          {
             if (rtp)
               {
                  const char *vals = NULL;
                  if (def_ret)
                    {
                       Eolian_Value val = eolian_expression_eval(def_ret, EOLIAN_MASK_ALL);
                       if (val.type)
                         vals = eolian_expression_value_to_literal(&val);
                    }
                  eina_strbuf_append_printf(buf, "   return %s;\n", vals ? vals : "0");
                  eina_stringshare_del(vals);
               }
             eina_strbuf_append(buf, "}\n\n");
          }
        else if (eina_strbuf_length_get(params_init))
          {
             eina_strbuf_append(buf, "   ");
             if (rtp)
               eina_strbuf_append(buf, "return ");
             eina_strbuf_append_char(buf, '_');
             eina_strbuf_append(buf, cnamel);
             if (!impl_same_class)
               eina_strbuf_append_printf(buf, "_%s", ocnamel);
             eina_strbuf_append_char(buf, '_');
             eina_strbuf_append(buf, eolian_function_name_get(fid));
             eina_strbuf_append(buf, func_suffix);
             eina_strbuf_append(buf, "(obj, pd, ");
             eina_strbuf_append(buf, eina_strbuf_string_get(params));
             eina_strbuf_append(buf, ");\n}\n\n");
          }

        eina_stringshare_del(dt);
     }

   if (impl_same_class)
     {
        void *data;
        Eina_Iterator *itr = eolian_property_keys_get(fid, ftype);
        Eina_Bool has_params = eina_iterator_next(itr, &data);
        eina_iterator_free(itr);

        if (!has_params && !var_as_ret)
          {
             if (is_prop)
               itr = eolian_property_values_get(fid, ftype);
             else
               itr = eolian_function_parameters_get(fid);
             has_params = eina_iterator_next(itr, &data);
             eina_iterator_free(itr);
          }

        eina_strbuf_append(buf, "EOAPI EFL_");
        if (!strcmp(rtpn, "void"))
          eina_strbuf_append(buf, "VOID_");
        eina_strbuf_append(buf, "FUNC_BODY");
        if (has_params)
          eina_strbuf_append_char(buf, 'V');
        if ((ftype == EOLIAN_PROP_GET) || eolian_function_object_is_const(fid)
            || eolian_function_is_class(fid))
          {
             eina_strbuf_append(buf, "_CONST");
          }
        eina_strbuf_append_char(buf, '(');

        Eina_Stringshare *eofn = eolian_function_full_c_name_get(fid, ftype, EINA_FALSE);
        eina_strbuf_append(buf, eofn);

        if (strcmp(rtpn, "void"))
          {
             const char *vals = NULL;
             if (def_ret)
               {
                  Eolian_Value val = eolian_expression_eval(def_ret, EOLIAN_MASK_ALL);
                  if (val.type)
                    vals = eolian_expression_value_to_literal(&val);
               }
             eina_strbuf_append_printf(buf, ", %s, %s", rtpn, vals ? vals : "0");
             if (vals && (eolian_expression_type_get(def_ret) == EOLIAN_EXPR_NAME))
               {
                  Eina_Stringshare *valn = eolian_expression_serialize(def_ret);
                  eina_strbuf_append_printf(buf, " /* %s */", valn);
                  eina_stringshare_del(valn);
               }
             eina_stringshare_del(vals);
          }
        if (has_params)
          {
             eina_strbuf_append(buf, ", EFL_FUNC_CALL(");
             eina_strbuf_append(buf, eina_strbuf_string_get(params));
             eina_strbuf_append_char(buf, ')');
             eina_strbuf_append(buf, eina_strbuf_string_get(params_full));
          }

        eina_strbuf_append(buf, ");\n");

        /* now try legacy */
        Eina_Stringshare *lfn = eolian_function_full_c_name_get(fid, ftype, EINA_TRUE);
        if (!eolian_function_is_beta(fid) && lfn)
          {
             eina_strbuf_append(lbuf, "\nEAPI ");
             eina_strbuf_append(lbuf, rtpn);
             eina_strbuf_append_char(lbuf, '\n');
             eina_strbuf_append(lbuf, lfn);
             /* param list */
             eina_strbuf_append_char(lbuf, '(');
             /* for class funcs, offset the params to remove comma */
             int poff = 2;
             if (!eolian_function_is_class(fid))
               {
                  /* non-class funcs have the obj though */
                  poff = 0;
                  if ((ftype == EOLIAN_PROP_GET) || eolian_function_object_is_const(fid))
                    eina_strbuf_append(lbuf, "const ");
                  eina_strbuf_append_printf(lbuf, "%s *obj", cname);
               }
             eina_strbuf_append(lbuf, eina_strbuf_string_get(params_full) + poff);
             eina_strbuf_append(lbuf, ")\n{\n");
             /* body */
             if (strcmp(rtpn, "void"))
               eina_strbuf_append(lbuf, "   return ");
             else
               eina_strbuf_append(lbuf, "   ");
             eina_strbuf_append(lbuf, eofn);
             eina_strbuf_append_char(lbuf, '(');
             if (!eolian_function_is_class(fid))
               eina_strbuf_append(lbuf, "obj");
             else
               {
                  Eina_Stringshare *mname = eolian_class_c_name_get(cl);
                  eina_strbuf_append(lbuf, mname);
                  eina_stringshare_del(mname);
               }
             if (has_params)
               eina_strbuf_append_printf(lbuf, ", %s", eina_strbuf_string_get(params));
             eina_strbuf_append(lbuf, ");\n}\n");
          }

        eina_stringshare_del(lfn);
        eina_stringshare_del(eofn);
     }

   free(cname);
   free(cnamel);
   free(ocnamel);

   eina_stringshare_del(rtpn);

   eina_stringshare_del(promise_param_name);
   eina_stringshare_del(promise_param_type);

   eina_strbuf_free(params);
   eina_strbuf_free(params_full);
   eina_strbuf_free(params_full_imp);
   eina_strbuf_free(params_init);
}

static void
_gen_opfunc(const Eolian_Function *fid, Eolian_Function_Type ftype,
            Eina_Strbuf *buf, Eina_Bool pinit,
            const char *cnamel, const char *ocnamel)
{
   Eina_Stringshare *fnm = eolian_function_full_c_name_get(fid, ftype, EINA_FALSE);
   eina_strbuf_append(buf, "      EFL_OBJECT_OP_FUNC(");
   eina_strbuf_append(buf, fnm);
   eina_strbuf_append(buf, ", ");
   if (!ocnamel && eolian_function_is_virtual_pure(fid, ftype))
     eina_strbuf_append(buf, "NULL),\n");
   else
     {
        if (pinit)
          eina_strbuf_append(buf, "__eolian");
        eina_strbuf_append_printf(buf, "_%s_", cnamel);
        if (ocnamel)
          eina_strbuf_append_printf(buf, "%s_", ocnamel);
        eina_strbuf_append(buf, eolian_function_name_get(fid));
        if (ftype == EOLIAN_PROP_GET)
          eina_strbuf_append(buf, "_get");
        else if (ftype == EOLIAN_PROP_SET)
          eina_strbuf_append(buf, "_set");
        eina_strbuf_append(buf, "),\n");
     }
}

static Eina_Bool
_gen_initializer(const Eolian_Class *cl, Eina_Strbuf *buf)
{
   Eina_Iterator *itr = eolian_class_implements_get(cl);
   const Eolian_Implement *imp;
   if (!eina_iterator_next(itr, (void **)&imp))
     {
        eina_iterator_free(itr);
        return EINA_FALSE;
     }
   eina_iterator_free(itr);

   char *cnamel = NULL;
   eo_gen_class_names_get(cl, NULL, NULL, &cnamel);

   eina_strbuf_append(buf, "\nstatic Eina_Bool\n_");
   eina_strbuf_append(buf, cnamel);
   eina_strbuf_append(buf, "_class_initializer(Efl_Class *klass)\n{\n");

   Eina_Strbuf *ops = eina_strbuf_new(), *cops = eina_strbuf_new();

   /* start over with clean itearator */
   itr = eolian_class_implements_get(cl);
   EINA_ITERATOR_FOREACH(itr, imp)
     {
        const Eolian_Class *icl = eolian_implement_class_get(imp);
        Eolian_Function_Type ftype;
        const Eolian_Function *fid = eolian_implement_function_get(imp, &ftype);

        Eina_Strbuf *obuf = ops;
        if (eolian_function_is_class(fid))
          obuf = cops;

        if (!eina_strbuf_length_get(obuf))
          eina_strbuf_append_printf(obuf, "   EFL_OPS_DEFINE(%s,\n",
                                    (obuf == ops) ? "ops" : "cops");

        Eina_Bool found = !!eina_hash_find(_funcs_params_init, &imp);
        char *ocnamel = NULL;
        if (cl != icl)
          eo_gen_class_names_get(icl, NULL, NULL, &ocnamel);

        switch (ftype)
          {
           case EOLIAN_PROP_GET:
             _gen_opfunc(fid, EOLIAN_PROP_GET, obuf, found, cnamel, ocnamel);
             break;
           case EOLIAN_PROP_SET:
             _gen_opfunc(fid, EOLIAN_PROP_SET, obuf, found, cnamel, ocnamel);
             break;
           case EOLIAN_PROPERTY:
             _gen_opfunc(fid, EOLIAN_PROP_SET, obuf, found, cnamel, ocnamel);
             _gen_opfunc(fid, EOLIAN_PROP_GET, obuf, found, cnamel, ocnamel);
             break;
           default:
             _gen_opfunc(fid, EOLIAN_METHOD, obuf, found, cnamel, ocnamel);
             break;
          }

        free(ocnamel);
     }
   eina_iterator_free(itr);

   /* strip the final comma before appending */
   if (eina_strbuf_length_get(ops))
     {
        eina_strbuf_remove(ops, eina_strbuf_length_get(ops) - 2,
                           eina_strbuf_length_get(ops));
        eina_strbuf_append(ops, "\n   );\n");
        eina_strbuf_append(buf, eina_strbuf_string_get(ops));
     }
   if (eina_strbuf_length_get(cops))
     {
        eina_strbuf_remove(cops, eina_strbuf_length_get(cops) - 2,
                           eina_strbuf_length_get(cops));
        eina_strbuf_append(cops, "\n   );\n");
        eina_strbuf_append(buf, eina_strbuf_string_get(cops));
     }

   eina_strbuf_append(buf, "   return efl_class_functions_set(klass, ");
   if (eina_strbuf_length_get(ops))
     eina_strbuf_append(buf, "&ops, ");
   else
     eina_strbuf_append(buf, "NULL, ");
   if (eina_strbuf_length_get(cops))
     eina_strbuf_append(buf, "&cops);\n");
   else
     eina_strbuf_append(buf, "NULL);\n");

   eina_strbuf_free(ops);
   eina_strbuf_free(cops);

   eina_strbuf_append(buf, "}\n\n");

   free(cnamel);

   return EINA_TRUE;
}

void
eo_gen_source_gen(const Eolian_Class *cl, Eina_Strbuf *buf)
{
   if (!cl)
     return;

   _funcs_params_init = eina_hash_pointer_new(NULL);

   char *cnamel = NULL;
   eo_gen_class_names_get(cl, NULL, NULL, &cnamel);

   /* event section, they come first */
   {
      Eina_Iterator *itr = eolian_class_events_get(cl);
      Eolian_Event *ev;
      EINA_ITERATOR_FOREACH(itr, ev)
        {
           Eina_Stringshare *evn = eolian_event_c_name_get(ev);
           eina_strbuf_append(buf, "EWAPI const Efl_Event_Description _");
           eina_strbuf_append(buf, evn);
           eina_strbuf_append(buf, " =\n   EFL_EVENT_DESCRIPTION");
           if (eolian_event_is_hot(ev))
             eina_strbuf_append(buf, "_HOT");
           if (eolian_event_is_restart(ev))
             eina_strbuf_append(buf, "_RESTART");
           eina_strbuf_append_printf(buf, "(\"%s\");\n", eolian_event_name_get(ev));
           eina_stringshare_del(evn);
        }
      eina_iterator_free(itr);
   }

   Eina_Strbuf *lbuf = eina_strbuf_new();

   /* method section */
   {
      Eina_Iterator *itr = eolian_class_implements_get(cl);
      const Eolian_Implement *imp;
      EINA_ITERATOR_FOREACH(itr, imp)
        {
           Eolian_Function_Type ftype = EOLIAN_UNRESOLVED;
           const Eolian_Function *fid = eolian_implement_function_get(imp, &ftype);
           switch (ftype)
             {
              case EOLIAN_PROP_GET:
              case EOLIAN_PROP_SET:
                _gen_func(cl, fid, ftype, buf, imp, lbuf);
                break;
              case EOLIAN_PROPERTY:
                _gen_func(cl, fid, EOLIAN_PROP_SET, buf, imp, lbuf);
                _gen_func(cl, fid, EOLIAN_PROP_GET, buf, imp, lbuf);
                break;
              default:
                _gen_func(cl, fid, EOLIAN_UNRESOLVED, buf, imp, lbuf);
             }
        }
      eina_iterator_free(itr);
   }

   /* class initializer - contains method defs */
   Eina_Bool has_init = _gen_initializer(cl, buf);

   /* class description */
   eina_strbuf_append(buf, "static const Efl_Class_Description _");
   eina_strbuf_append(buf, cnamel);
   eina_strbuf_append(buf, "_class_desc = {\n"
                           "   EO_VERSION,\n");
   eina_strbuf_append_printf(buf, "   \"%s\",\n", eolian_class_full_name_get(cl));

   switch (eolian_class_type_get(cl))
     {
      case EOLIAN_CLASS_ABSTRACT:
        eina_strbuf_append(buf, "   EFL_CLASS_TYPE_REGULAR_NO_INSTANT,\n");
        break;
      case EOLIAN_CLASS_MIXIN:
        eina_strbuf_append(buf, "   EFL_CLASS_TYPE_MIXIN,\n");
        break;
      case EOLIAN_CLASS_INTERFACE:
        eina_strbuf_append(buf, "   EFL_CLASS_TYPE_INTERFACE,\n");
        break;
      default:
        eina_strbuf_append(buf, "   EFL_CLASS_TYPE_REGULAR,\n");
        break;
     }

   Eina_Stringshare *dt = eolian_class_c_data_type_get(cl);
   if (!strcmp(dt, "void"))
     eina_strbuf_append(buf, "   0,\n");
   else
     eina_strbuf_append_printf(buf, "   sizeof(%s),\n", dt);
   eina_stringshare_del(dt);

   if (has_init)
     eina_strbuf_append_printf(buf, "   _%s_class_initializer,\n", cnamel);
   else
     eina_strbuf_append(buf, "   NULL,\n");

   if (eolian_class_ctor_enable_get(cl))
     eina_strbuf_append_printf(buf, "   _%s_class_constructor,\n", cnamel);
   else
     eina_strbuf_append(buf, "   NULL,\n");

   if (eolian_class_dtor_enable_get(cl))
     eina_strbuf_append_printf(buf, "   _%s_class_destructor\n", cnamel);
   else
     eina_strbuf_append(buf, "   NULL\n");

   eina_strbuf_append(buf, "};\n\n");

   /* class def */
   eina_strbuf_append(buf, "EFL_DEFINE_CLASS(");

   Eina_Stringshare *cgfunc = eolian_class_c_get_function_name_get(cl);
   eina_strbuf_append(buf, cgfunc);
   eina_stringshare_del(cgfunc);

   eina_strbuf_append_printf(buf, ", &_%s_class_desc", cnamel);

   /* inherits in EFL_DEFINE_CLASS */
   {
      const char *iname;
      Eina_Iterator *itr = eolian_class_inherits_get(cl);
      /* no inherits, NULL parent */
      if (!itr)
        eina_strbuf_append(buf, ", NULL");
      EINA_ITERATOR_FOREACH(itr, iname)
        {
           const Eolian_Class *icl = eolian_class_get_by_name(iname);
           Eina_Stringshare *mname = eolian_class_c_name_get(icl);
           eina_strbuf_append_printf(buf, ", %s", mname);
           eina_stringshare_del(mname);
        }
      eina_iterator_free(itr);
   }

   /* terminate inherits */
   eina_strbuf_append(buf, ", NULL);\n");

   /* append legacy if there */
   eina_strbuf_append(buf, eina_strbuf_string_get(lbuf));

   eina_strbuf_free(lbuf);

   /* and we're done */
   free(cnamel);
   eina_hash_free(_funcs_params_init);
}

static void
_gen_params(const Eolian_Function *fid, Eolian_Function_Type ftype,
            Eina_Bool var_as_ret, Eina_Strbuf *params, Eina_Strbuf *params_full)
{
   Eina_Bool is_prop = (ftype == EOLIAN_PROP_GET || ftype == EOLIAN_PROP_SET);

   /* property keys */
   {
      Eina_Iterator *itr = eolian_property_keys_get(fid, ftype);
      Eolian_Function_Parameter *pr;
      EINA_ITERATOR_FOREACH(itr, pr)
        {
           const char *prn = eolian_parameter_name_get(pr);
           const Eolian_Type *pt = eolian_parameter_type_get(pr);
           Eina_Stringshare *ptn = eolian_type_c_type_get(pt);

           eina_strbuf_append(params, ", ");
           eina_strbuf_append(params, prn);

           eina_strbuf_append_printf(params_full, ", %s", ptn);
           if (ptn[strlen(ptn) - 1] != '*')
             eina_strbuf_append_char(params_full, ' ');
           eina_strbuf_append(params_full, prn);

           eina_stringshare_del(ptn);
        }
      eina_iterator_free(itr);
   }

   /* property values or method params if applicable */
   if (!var_as_ret)
     {
        Eina_Iterator *itr;
        if (is_prop)
          itr = eolian_property_values_get(fid, ftype);
        else
          itr = eolian_function_parameters_get(fid);
        Eolian_Function_Parameter *pr;
        EINA_ITERATOR_FOREACH(itr, pr)
          {
             Eolian_Parameter_Dir pd = eolian_parameter_direction_get(pr);
             const char *prn = eolian_parameter_name_get(pr);
             const Eolian_Type *pt = eolian_parameter_type_get(pr);
             Eina_Stringshare *ptn = eolian_type_c_type_get(pt);

             Eina_Bool had_star = ptn[strlen(ptn) - 1] == '*';
             const char *add_star = _get_add_star(ftype, pd);

             eina_strbuf_append(params, ", ");
             eina_strbuf_append(params, prn);

             eina_strbuf_append(params_full, ", ");
             eina_strbuf_append(params_full, ptn);
             if (!had_star)
               eina_strbuf_append_char(params_full, ' ');
             eina_strbuf_append(params_full, add_star);
             eina_strbuf_append(params_full, prn);

             eina_stringshare_del(ptn);
          }
        eina_iterator_free(itr);
     }
}

static void
_gen_proto(const Eolian_Class *cl, const Eolian_Function *fid,
           Eolian_Function_Type ftype, Eina_Strbuf *buf,
           const Eolian_Implement *impl, const char *dtype,
           const char *cnamel)
{
   Eina_Bool impl_same_class = (eolian_implement_class_get(impl) == cl);
   if (impl_same_class && eolian_function_is_virtual_pure(fid, ftype))
     return;

   char *ocnamel = NULL;
   if (!impl_same_class)
     eo_gen_class_names_get(eolian_implement_class_get(impl), NULL, NULL, &ocnamel);

   char fname[256], iname[256];
   if (!impl_same_class)
     snprintf(iname, sizeof(iname), "%s_%s", cnamel, ocnamel);
   else
     snprintf(iname, sizeof(iname), "%s", cnamel);

   snprintf(fname, sizeof(fname), "_%s_%s%s", iname, eolian_function_name_get(fid),
            (ftype == EOLIAN_PROP_GET)
               ? "_get" : ((ftype == EOLIAN_PROP_SET) ? "_set" : ""));

   if (_function_exists(fname, buf))
     {
        free(ocnamel);
        return;
     }

   printf("generating function %s...\n", fname);

   Eina_Bool var_as_ret = EINA_FALSE;
   const Eolian_Type *rtp = eolian_function_return_type_get(fid, ftype);
   if ((ftype == EOLIAN_PROP_GET) && !rtp)
     {
        void *d1, *d2;
        Eina_Iterator *itr = eolian_property_values_get(fid, ftype);
        if (eina_iterator_next(itr, &d1) && !eina_iterator_next(itr, &d2))
          {
             Eolian_Function_Parameter *pr = d1;
             rtp = eolian_parameter_type_get(pr);
             var_as_ret = EINA_TRUE;
          }
        eina_iterator_free(itr);
     }

   eina_strbuf_append(buf, "EOLIAN static ");
   if (rtp)
     {
        Eina_Stringshare *rtpn = eolian_type_c_type_get(rtp);
        eina_strbuf_append(buf, rtpn);
        eina_stringshare_del(rtpn);
     }
   else
     eina_strbuf_append(buf, "void");

   eina_strbuf_append_printf(buf, "\n%s(", fname);

   if (eolian_function_object_is_const(fid))
     eina_strbuf_append(buf, "const ");

   eina_strbuf_append(buf, "Eo *obj, ");
   if (strcmp(dtype, "void"))
     eina_strbuf_append_printf(buf, "%s *pd", dtype);
   else
     eina_strbuf_append(buf, "void *pd EINA_UNUSED");

   /* gen params here */
   Eina_Strbuf *params = eina_strbuf_new();
   Eina_Strbuf *params_full = eina_strbuf_new();
   _gen_params(fid, ftype, var_as_ret, params, params_full);

   if (eina_strbuf_length_get(params_full))
     eina_strbuf_append(buf, eina_strbuf_string_get(params_full));

   eina_strbuf_append(buf, ")\n{\n");

   const char *efname = eolian_function_name_get(fid);
   if (strlen(efname) >= (sizeof("destructor") - 1) && !impl_same_class)
     if (!strcmp(efname + strlen(efname) - sizeof("destructor") + 1, "destructor"))
       {
          Eina_Stringshare *fcn = eolian_function_full_c_name_get(fid, ftype, EINA_FALSE);
          Eina_Stringshare *mname = eolian_class_c_name_get(cl);
          eina_strbuf_append(buf, "   ");
          eina_strbuf_append(buf, fcn);
          eina_stringshare_del(fcn);
          eina_strbuf_append_printf(buf, "(efl_super(obj, %s)", mname);
          eina_stringshare_del(mname);
          if (eina_strbuf_length_get(params))
            eina_strbuf_append(buf, eina_strbuf_string_get(params));
          eina_strbuf_append(buf, ");\n");
       }
   eina_strbuf_append(buf, "\n}\n\n");

   eina_strbuf_free(params_full);
   eina_strbuf_free(params);
   free(ocnamel);
}

void
eo_gen_impl_gen(const Eolian_Class *cl, Eina_Strbuf *buf)
{
   if (!cl)
     return;

   char *cname = NULL, *cnamel = NULL;
   eo_gen_class_names_get(cl, &cname, NULL, &cnamel);

   Eina_Strbuf *beg = eina_strbuf_new();

   if (!_type_exists("EFL_BETA_API_SUPPORT", buf))
     {
        printf("generating EFL_BETA_API_SUPPORT...\n");
        eina_strbuf_append(beg, "#define EFL_BETA_API_SUPPORT\n");
     }

   if (!_type_exists("<Eo.h>", buf))
     {
        printf("generating includes for <Eo.h> and \"%s.eo.h\"...\n", cnamel);
        eina_strbuf_append(beg, "#include <Eo.h>\n");
        eina_strbuf_append_printf(beg, "#include \"%s.eo.h\"\n\n", cnamel);
     }

   /* determine data type name */
   Eina_Stringshare *dt = eolian_class_c_data_type_get(cl);

   /* generate data type struct */
   if (strcmp(dt, "void") && !_type_exists(dt, buf))
     {
        printf("generating data type structure %s...\n", dt);
        eina_strbuf_append_printf(beg, "typedef struct\n{\n\n} %s;\n\n", dt);
     }

   if (eina_strbuf_length_get(beg))
     eina_strbuf_prepend(buf, eina_strbuf_string_get(beg));

   eina_strbuf_free(beg);

   /* method section */
   {
      Eina_Iterator *itr = eolian_class_implements_get(cl);
      const Eolian_Implement *imp;
      EINA_ITERATOR_FOREACH(itr, imp)
        {
           Eolian_Function_Type ftype = EOLIAN_UNRESOLVED;
           const Eolian_Function *fid = eolian_implement_function_get(imp, &ftype);
           switch (ftype)
             {
              case EOLIAN_PROP_GET:
              case EOLIAN_PROP_SET:
                _gen_proto(cl, fid, ftype, buf, imp, dt, cnamel);
                break;
              case EOLIAN_PROPERTY:
                _gen_proto(cl, fid, EOLIAN_PROP_SET, buf, imp, dt, cnamel);
                _gen_proto(cl, fid, EOLIAN_PROP_GET, buf, imp, dt, cnamel);
                break;
              default:
                _gen_proto(cl, fid, EOLIAN_UNRESOLVED, buf, imp, dt, cnamel);
             }
        }
      eina_iterator_free(itr);
   }

   /* free the data type */
   eina_stringshare_del(dt);

   if (eolian_class_ctor_enable_get(cl))
     {
        char fname[128];
        snprintf(fname, sizeof(fname), "_%s_class_constructor", cnamel);
        if (!_function_exists(fname, buf))
          {
             printf("generating function %s...\n", fname);
             eina_strbuf_append_printf(buf,
                                       "EOLIAN static void\n"
                                       "_%s_class_constructor(Efl_Class *klass)\n"
                                       "{\n\n"
                                       "}\n\n", cnamel);
          }
     }

   if (eolian_class_dtor_enable_get(cl))
     {
        char fname[128];
        snprintf(fname, sizeof(fname), "_%s_class_destructor", cnamel);
        if (!_function_exists(fname, buf))
          {
             printf("generating function %s...\n", fname);
             eina_strbuf_append_printf(buf,
                                       "EOLIAN static void\n"
                                       "_%s_class_destructor(Efl_Class *klass)\n"
                                       "{\n\n"
                                       "}\n\n", cnamel);
          }
     }

   printf("removing includes for \"%s.eo.c\"\n", cnamel);
   char ibuf[512];
   snprintf(ibuf, sizeof(ibuf), "\n#include \"%s.eo.c\"\n", cnamel);
   eina_strbuf_replace_all(buf, ibuf, "\n");

   printf("generating include for \"%s.eo.c\"\n", cnamel);
   eina_strbuf_append_printf(buf, "#include \"%s.eo.c\"\n", cnamel);

   free(cname);
   free(cnamel);
}
