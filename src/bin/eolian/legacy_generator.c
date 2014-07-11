#include <Eina.h>
#include <string.h>

#include "Eolian.h"

#include "legacy_generator.h"
#include "common_funcs.h"

static _eolian_class_vars class_env;

static const char
tmpl_eapi_funcdef[] = "\n\
/**\n\
 *\n\
@#desc\n\
 *\n\
@#list_desc_param\
 */\n\
EAPI @#type_return%s(@#is_constEo *obj@#params)@#flags;\n\
";

/*@#CLASS_CHECK(obj) @#check_ret;\n\*/
static const char
tmpl_eapi_body[] ="\
\n\
EAPI @#ret_type\n\
@#eapi_func(@#is_constEo *obj@#full_params)\n\
{\n\
@#ret_init_val\
   eo_do((Eo *) obj, @#eo_ret_assign@#eo_func(@#eo_params));\n\
   return @#ret_val;\n\
}\n\
";
static const char
tmpl_eapi_body_void[] ="\
\n\
EAPI void\n\
@#eapi_func(@#is_constEo *obj@#full_params)\n\
{\n\
   eo_do((Eo *) obj, @#eo_func(@#eo_params));\n\
}\n\
";

static void
_eapi_decl_func_generate(const Eolian_Class *class, const Eolian_Function *funcid, Eolian_Function_Type ftype, Eina_Strbuf *buf)
{
   _eolian_class_func_vars func_env;
   const char *funcname = eolian_function_name_get(funcid);
   const char *suffix = "";
   const Eolian_Type *rettypet = NULL;
   const char *rettype = NULL;
   Eina_Bool var_as_ret = EINA_FALSE;
   Eina_Bool add_star = EINA_FALSE;
   Eina_Bool ret_const = EINA_FALSE;
   char tmpstr[0xFF];
   const Eina_List *l;
   void *data;
   Eina_Strbuf *flags = NULL;
   int leg_param_idx = 1; /* Index of the parameter inside the legacy function. It begins from 1 since obj is the first. */

   Eina_Strbuf *fbody = eina_strbuf_new();
   Eina_Strbuf *fparam = eina_strbuf_new();
   Eina_Strbuf *descparam = eina_strbuf_new();

   _class_func_env_create(class, funcname, ftype, &func_env);
   rettypet = eolian_function_return_type_get(funcid, ftype);
   if (ftype == EOLIAN_PROP_GET)
     {
        suffix = "_get";
        add_star = EINA_TRUE;
        if (!rettypet)
          {
             l = eolian_parameters_list_get(funcid);
             if (eina_list_count(l) == 1)
               {
                  data = eina_list_data_get(l);
                  eolian_parameter_information_get((Eolian_Function_Parameter*)data, NULL, &rettypet, NULL, NULL);
                  var_as_ret = EINA_TRUE;
                  ret_const = eolian_parameter_const_attribute_get(data, EINA_TRUE);
               }
          }
     }

   if (ftype == EOLIAN_PROP_SET) suffix = "_set";

   if (func_env.legacy_func[0] == '\0') goto end;
   eina_strbuf_append_printf(fbody, tmpl_eapi_funcdef, func_env.legacy_func);

   sprintf (tmpstr, "comment%s", suffix);
   const char *desc = eolian_function_description_get(funcid, tmpstr);
   Eina_Strbuf *linedesc = eina_strbuf_new();
   eina_strbuf_append(linedesc, desc ? desc : "No description supplied.");
   if (eina_strbuf_length_get(linedesc))
     {
        eina_strbuf_replace_all(linedesc, "\n", "\n * ");
        eina_strbuf_replace_all(linedesc, " * \n", " *\n");
        eina_strbuf_prepend(linedesc," * ");
     }
   else
     {
        eina_strbuf_append(linedesc," *");
     }

   eina_strbuf_replace_all(fbody, "@#desc", eina_strbuf_string_get(linedesc));
   eina_strbuf_free(linedesc);

   EINA_LIST_FOREACH(eolian_property_keys_list_get(funcid), l, data)
     {
        const Eolian_Type *ptypet;
        const char *pname;
        const char *pdesc;
        const char *ptype;
        eolian_parameter_information_get((Eolian_Function_Parameter*)data, NULL, &ptypet, &pname, &pdesc);
        ptype = eolian_type_c_type_get(ptypet);
        leg_param_idx++;
        eina_strbuf_append_printf(fparam, ", %s%s %s",
              eolian_parameter_const_attribute_get(data, ftype == EOLIAN_PROP_GET)?"const":"",
              ptype, pname);
        eina_stringshare_del(ptype);
        eina_strbuf_append_printf(descparam, " * @param[in] %s %s\n", pname, pdesc?pdesc:"No description supplied.");
        if (eolian_parameter_is_nonull((Eolian_Function_Parameter*)data))
          {
             if (!flags)
               {
                  flags = eina_strbuf_new();
                  eina_strbuf_append_printf(flags, " EINA_ARG_NONNULL(%d", leg_param_idx);
               }
             else
                eina_strbuf_append_printf(flags, ", %d", leg_param_idx);
          }
     }
   if (!var_as_ret)
     {
       EINA_LIST_FOREACH(eolian_parameters_list_get(funcid), l, data)
         {
            const Eolian_Type *ptypet;
            const char *pname;
            const char *pdesc;
            const char *ptype;
            Eolian_Parameter_Dir pdir;
            const char *str_dir[] = {"in", "out", "inout"};
            eolian_parameter_information_get((Eolian_Function_Parameter*)data, &pdir, &ptypet, &pname, &pdesc);
            ptype = eolian_type_c_type_get(ptypet);
            Eina_Bool had_star = !!strchr(ptype, '*');
            if (ftype == EOLIAN_UNRESOLVED || ftype == EOLIAN_METHOD) add_star = (pdir == EOLIAN_OUT_PARAM || pdir == EOLIAN_INOUT_PARAM);
            if (ftype == EOLIAN_PROP_GET) pdir = EOLIAN_OUT_PARAM;
            if (ftype == EOLIAN_PROP_SET) pdir = EOLIAN_IN_PARAM;
            leg_param_idx++;
            eina_strbuf_append_printf(fparam, ", %s%s%s%s%s",
                  eolian_parameter_const_attribute_get(data, ftype == EOLIAN_PROP_GET)?"const ":"",
                  ptype, had_star?"":" ", add_star?"*":"", pname);
            eina_stringshare_del(ptype);
            const char *dir_str = str_dir[(int)pdir];
            eina_strbuf_append_printf(descparam, " * @param[%s] %s %s\n", dir_str, pname, pdesc?pdesc:"No description supplied.");
            if (eolian_parameter_is_nonull((Eolian_Function_Parameter*)data))
              {
                 if (!flags)
                   {
                      flags = eina_strbuf_new();
                      eina_strbuf_append_printf(flags, " EINA_ARG_NONNULL(%d", leg_param_idx);
                   }
                 else
                    eina_strbuf_append_printf(flags, ", %d", leg_param_idx);
              }
         }
     }
   if (flags) eina_strbuf_append_printf(flags, ")");

   if (rettypet) rettype = eolian_type_c_type_get(rettypet);

   eina_strbuf_replace_all(fbody, "@#params", eina_strbuf_string_get(fparam));
   eina_strbuf_replace_all(fbody, "@#list_desc_param", eina_strbuf_string_get(descparam));
   eina_strbuf_reset(fparam);
   eina_strbuf_append_printf(fparam, "%s%s%s",
         ret_const ? "const " : "",
         rettype ? rettype : "void",
         rettype && strchr(rettype, '*')?"":" ");
   eina_strbuf_replace_all(fbody, "@#type_return", eina_strbuf_string_get(fparam));
   eina_strbuf_replace_all(fbody, "@#is_const", (ftype == EOLIAN_PROP_GET || eolian_function_object_is_const(funcid)) ? "const " : "");
   if (eolian_function_return_is_warn_unused(funcid, ftype))
     {
        Eina_Bool no_nonull = !flags;
        if (no_nonull) flags = eina_strbuf_new();
        eina_strbuf_prepend(flags, " EINA_WARN_UNUSED_RESULT");
     }
   if (flags)
      eina_strbuf_replace_all(fbody, "@#flags", eina_strbuf_string_get(flags));
   eina_strbuf_replace_all(fbody, "@#flags", (eolian_function_return_is_warn_unused(funcid, ftype)) ? " EINA_WARN_UNUSED_RESULT" : "");
   eina_strbuf_append(buf, eina_strbuf_string_get(fbody));

   if (rettype) eina_stringshare_del(rettype);

end:
   eina_strbuf_free(flags);
   eina_strbuf_free(fbody);
   eina_strbuf_free(fparam);
   eina_strbuf_free(descparam);
}

static void
_eapi_func_generate(const Eolian_Class *class, const Eolian_Function *funcid, Eolian_Function_Type ftype, Eina_Strbuf *buf)
{
   _eolian_class_func_vars func_env;
   char tmpstr[0xFF];
   Eina_Bool var_as_ret = EINA_FALSE;
   const Eolian_Type *rettypet = NULL;
   const char *rettype = NULL;
   const char *retname = NULL;
   Eina_Bool ret_const = EINA_FALSE;
   Eina_Bool add_star = EINA_FALSE;
   Eina_Bool ret_is_void = EINA_FALSE;

   Eina_Strbuf *fbody = eina_strbuf_new();
   Eina_Strbuf *fparam = eina_strbuf_new();
   Eina_Strbuf *eoparam = eina_strbuf_new();

   _class_func_env_create(class, eolian_function_name_get(funcid), ftype, &func_env);
   rettypet = eolian_function_return_type_get(funcid, ftype);
   if (rettypet) rettype = eolian_type_c_type_get(rettypet);
   if (rettype && !strcmp(rettype, "void")) ret_is_void = EINA_TRUE;
   retname = "ret";
   if (ftype == EOLIAN_PROP_GET)
     {
        add_star = EINA_TRUE;
        if (!rettypet)
          {
             const Eina_List *l = eolian_parameters_list_get(funcid);
             if (eina_list_count(l) == 1)
               {
                  void* data = eina_list_data_get(l);
                  eolian_parameter_information_get((Eolian_Function_Parameter*)data, NULL, &rettypet, &retname, NULL);
                  var_as_ret = EINA_TRUE;
                  ret_const = eolian_parameter_const_attribute_get(data, EINA_TRUE);
               }
          }
     }

   if (func_env.legacy_func[0] == '\0') goto end;

   if (!rettype && rettypet) rettype = eolian_type_c_type_get(rettypet);

   if (rettype && (!ret_is_void))
     eina_strbuf_append(fbody, tmpl_eapi_body);
   else
     eina_strbuf_append(fbody, tmpl_eapi_body_void);

   eina_strbuf_replace_all(fbody, "@#eapi_func", func_env.legacy_func);
   eina_strbuf_replace_all(fbody, "@#eo_func", func_env.lower_eo_func);

   const Eina_List *l;
   void *data;

   tmpstr[0] = '\0';

   EINA_LIST_FOREACH(eolian_property_keys_list_get(funcid), l, data)
     {
        const Eolian_Type *ptypet;
        const char *pname;
        const char *ptype;
        eolian_parameter_information_get((Eolian_Function_Parameter*)data, NULL, &ptypet, &pname, NULL);
        ptype = eolian_type_c_type_get(ptypet);
        eina_strbuf_append_printf(fparam, ", %s%s %s",
              eolian_parameter_const_attribute_get(data, ftype == EOLIAN_PROP_GET)?"const ":"",
              ptype, pname);
        eina_stringshare_del(ptype);
        if (eina_strbuf_length_get(eoparam)) eina_strbuf_append(eoparam, ", ");
        eina_strbuf_append_printf(eoparam, "%s", pname);
     }
   if (!var_as_ret)
   {
       EINA_LIST_FOREACH(eolian_parameters_list_get(funcid), l, data)
         {
            const Eolian_Type *ptypet;
            const char *pname;
            const char *ptype;
            Eolian_Parameter_Dir pdir;
            eolian_parameter_information_get((Eolian_Function_Parameter*)data, &pdir, &ptypet, &pname, NULL);
            ptype = eolian_type_c_type_get(ptypet);
            Eina_Bool had_star = !!strchr(ptype, '*');
            if (ftype == EOLIAN_UNRESOLVED || ftype == EOLIAN_METHOD) add_star = (pdir == EOLIAN_OUT_PARAM || pdir == EOLIAN_INOUT_PARAM);
            eina_strbuf_append_printf(fparam, ", %s%s%s%s%s",
                  eolian_parameter_const_attribute_get(data, ftype == EOLIAN_PROP_GET)?"const ":"",
                  ptype, had_star?"":" ", add_star?"*":"", pname);
            eina_stringshare_del(ptype);
            if (eina_strbuf_length_get(eoparam)) eina_strbuf_append(eoparam, ", ");
            eina_strbuf_append_printf(eoparam, "%s", pname);
         }
   }

   if (rettype && (!ret_is_void))
     {
        char tmp_ret_str[0xFF];
        sprintf (tmp_ret_str, "%s%s", ret_const?"const ":"", rettype);
             const char *default_ret_val =
                eolian_function_return_default_value_get(funcid, ftype);
             Eina_Bool had_star = !!strchr(rettype, '*');
             sprintf (tmpstr, "   %s%s%s%s = %s;\n",
                   ret_const?"const ":"", rettype, had_star?"":" ", retname,
                   default_ret_val?default_ret_val:"0");

             eina_strbuf_replace_all(fbody, "@#ret_type", tmp_ret_str);
             eina_strbuf_replace_all(fbody, "@#ret_init_val", tmpstr);
             tmp_ret_str[0] = '\0';
             if (rettype && !ret_is_void) sprintf(tmp_ret_str, "%s = ", retname);
             eina_strbuf_replace_all(fbody, "@#eo_ret_assign", tmp_ret_str);
     }

   eina_strbuf_replace_all(fbody, "@#full_params", eina_strbuf_string_get(fparam));
   eina_strbuf_replace_all(fbody, "@#eo_params", eina_strbuf_string_get(eoparam));

   eina_strbuf_replace_all(fbody, "@#ret_val", (rettype && !ret_is_void) ? retname : "");
   eina_strbuf_replace_all(fbody, "@#is_const", (ftype == EOLIAN_PROP_GET || eolian_function_object_is_const(funcid)) ? "const " : "");

   eina_strbuf_append(buf, eina_strbuf_string_get(fbody));

   if (rettype) eina_stringshare_del(rettype);

end:
   eina_strbuf_free(fbody);
   eina_strbuf_free(fparam);
   eina_strbuf_free(eoparam);
}

Eina_Bool
legacy_header_generate(const Eolian_Class *class, Eina_Strbuf *buf)
{
   const Eolian_Function_Type ftype_order[] = {EOLIAN_PROPERTY, EOLIAN_METHOD};
   const Eina_List *l;
   void *data;

   _class_env_create(class, NULL, &class_env);

   const char *desc = eolian_class_description_get(class);
   if (desc)
     {
        Eina_Strbuf *linedesc = eina_strbuf_new();
        eina_strbuf_append(linedesc, "/**\n");
        eina_strbuf_append(linedesc, desc);
        eina_strbuf_replace_all(linedesc, "\n", "\n * ");
        eina_strbuf_append(linedesc, "\n */\n");
        eina_strbuf_replace_all(linedesc, " * \n", " *\n"); /* Remove trailing whitespaces */
        eina_strbuf_append(buf, eina_strbuf_string_get(linedesc));
        eina_strbuf_free(linedesc);
     }

   int i;
   for (i = 0; i < 2; i++)
      EINA_LIST_FOREACH(eolian_class_functions_list_get(class, ftype_order[i]), l, data)
        {
           const Eolian_Function_Type ftype = eolian_function_type_get((Eolian_Function*)data);
           Eina_Bool prop_read = (ftype == EOLIAN_PROPERTY || ftype == EOLIAN_PROP_GET ) ? EINA_TRUE : EINA_FALSE ;
           Eina_Bool prop_write = (ftype == EOLIAN_PROPERTY || ftype == EOLIAN_PROP_SET ) ? EINA_TRUE : EINA_FALSE ;

           if (!prop_read && !prop_write)
             {
                _eapi_decl_func_generate(class, (Eolian_Function*)data, EOLIAN_METHOD, buf);
             }
           if (prop_write)
             {
                _eapi_decl_func_generate(class, (Eolian_Function*)data, EOLIAN_PROP_SET, buf);
             }
           if (prop_read)
             {
                _eapi_decl_func_generate(class, (Eolian_Function*)data, EOLIAN_PROP_GET, buf);
             }
        }
   return EINA_TRUE;
}

Eina_Bool
legacy_source_generate(const Eolian_Class *class, Eina_Strbuf *buf)
{
   Eina_Bool ret = EINA_FALSE;
   const Eina_List *itr;
   Eolian_Function *fn;

   _class_env_create(class, NULL, &class_env);

   Eina_Strbuf *tmpbuf = eina_strbuf_new();
   Eina_Strbuf *str_bodyf = eina_strbuf_new();

   //Properties
   EINA_LIST_FOREACH(eolian_class_functions_list_get(class, EOLIAN_PROPERTY), itr, fn)
     {
        const Eolian_Function_Type ftype = eolian_function_type_get(fn);

        Eina_Bool prop_read = ( ftype == EOLIAN_PROP_SET ) ? EINA_FALSE : EINA_TRUE;
        Eina_Bool prop_write = ( ftype == EOLIAN_PROP_GET ) ? EINA_FALSE : EINA_TRUE;

        if (prop_write)
          {
             _eapi_func_generate(class, fn, EOLIAN_PROP_SET, str_bodyf);
          }
        if (prop_read)
          {
             _eapi_func_generate(class, fn, EOLIAN_PROP_GET, str_bodyf);
          }
     }

   //Methods
   EINA_LIST_FOREACH(eolian_class_functions_list_get(class, EOLIAN_METHOD), itr, fn)
     {
        _eapi_func_generate(class, fn, EOLIAN_UNRESOLVED, str_bodyf);
     }

   eina_strbuf_append(buf, eina_strbuf_string_get(str_bodyf));

   ret = EINA_TRUE;
   eina_strbuf_free(tmpbuf);
   eina_strbuf_free(str_bodyf);

   return ret;
}
