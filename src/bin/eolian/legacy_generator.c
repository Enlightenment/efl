#include <Eina.h>
#include <string.h>

#include "Eolian.h"

#include "legacy_generator.h"
#include "common_funcs.h"

static const char
tmpl_eapi_funcdef[] = "\n\
/**\n\
 * @def @#class_@#func\n\
 *\n\
@#desc\n\
 *\n\
@#list_desc_param\
 */\n\
EAPI @#type_return@#class_@#func(@#is_constEo *obj@#params);@#flags\n\
";

/*@#CLASS_CHECK(obj) @#check_ret;\n\*/
static const char
tmpl_eapi_body[] ="\
\n\
EAPI @#ret_type\n\
@#eapi_prefix_@#func(@#is_constEo *obj@#full_params)\n\
{\n\
@#ret_init_val\
   eo_do((Eo *) obj, @#eoprefix_@#func(@#eo_params));\n\
   return @#ret_val;\n\
}\n\
";

static void
_eapi_decl_func_generate(const char *classname, Eolian_Function funcid, Eolian_Function_Type ftype, Eina_Strbuf *buf)
{
   //TODO return value
   const char *suffix = "";
   const char *rettype = NULL;
   const char *func_lpref = NULL;
   Eina_Bool var_as_ret = EINA_FALSE;
   Eina_Bool add_star = EINA_FALSE;
   char tmpstr[0xFF];
   const Eina_List *l;
   void *data;
   Eina_Strbuf *flags = NULL;
   int leg_param_idx = 1; /* Index of the parameter inside the legacy function. It begins from 1 since obj is the first. */

   Eina_Strbuf *fbody = eina_strbuf_new();
   Eina_Strbuf *fparam = eina_strbuf_new();
   Eina_Strbuf *descparam = eina_strbuf_new();

   rettype = eolian_function_return_type_get(funcid, ftype);
   if (ftype == EOLIAN_PROP_GET)
     {
        suffix = "_get";
        add_star = EINA_TRUE;
        func_lpref = eolian_function_data_get(funcid, EOLIAN_LEGACY_GET);
        if (!rettype)
          {
             l = eolian_parameters_list_get(funcid);
             if (eina_list_count(l) == 1)
               {
                  data = eina_list_data_get(l);
                  eolian_parameter_information_get((Eolian_Function_Parameter)data, NULL, &rettype, NULL, NULL);
                  var_as_ret = EINA_TRUE;
               }
          }
     }

   if (ftype == EOLIAN_PROP_SET)
     {
        suffix = "_set";
        func_lpref = eolian_function_data_get(funcid, EOLIAN_LEGACY_SET);
     }

   func_lpref = (func_lpref) ? func_lpref : eolian_function_data_get(funcid, EOLIAN_LEGACY);
   if (func_lpref && !strcmp(func_lpref, "null")) goto end;

   if (func_lpref)
     {
        _template_fill(fbody, tmpl_eapi_funcdef, "@#class", "@#func", EINA_FALSE);
        eina_strbuf_replace_all (fbody, "@#class_@#func", func_lpref);
     }
   else
     {
        func_lpref = eolian_class_legacy_prefix_get(classname);
        if (func_lpref && !strcmp(func_lpref, "null")) goto end;

        if (!func_lpref) func_lpref = classname;
        sprintf (tmpstr, "%s%s", eolian_function_name_get(funcid), suffix);
        _template_fill(fbody, tmpl_eapi_funcdef, func_lpref, tmpstr, EINA_FALSE);
     }

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
        const char *pname;
        const char *pdesc;
        const char *ptype;
        eolian_parameter_information_get((Eolian_Function_Parameter)data, NULL, &ptype, &pname, &pdesc);
        leg_param_idx++;
        eina_strbuf_append_printf(fparam, ", %s%s %s",
              eolian_parameter_const_attribute_get(data, ftype == EOLIAN_PROP_GET)?"const":"",
              ptype, pname);
        eina_strbuf_append_printf(descparam, " * @param[in] %s %s\n", pname, pdesc?pdesc:"No description supplied.");
        if (eolian_parameter_is_nonull((Eolian_Function_Parameter)data))
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
            const char *pname;
            const char *pdesc;
            const char *ptype;
            Eolian_Parameter_Dir pdir;
            const char *str_dir[] = {"in", "out", "inout"};
            eolian_parameter_information_get((Eolian_Function_Parameter)data, &pdir, &ptype, &pname, &pdesc);
            Eina_Bool had_star = !!strchr(ptype, '*');
            if (ftype == EOLIAN_UNRESOLVED || ftype == EOLIAN_METHOD) add_star = (pdir == EOLIAN_OUT_PARAM);
            if (ftype == EOLIAN_PROP_GET) pdir = EOLIAN_OUT_PARAM;
            if (ftype == EOLIAN_PROP_SET) pdir = EOLIAN_IN_PARAM;
            leg_param_idx++;
            eina_strbuf_append_printf(fparam, ", %s%s%s%s%s",
                  eolian_parameter_const_attribute_get(data, ftype == EOLIAN_PROP_GET)?"const":"",
                  ptype, had_star?"":" ", add_star?"*":"", pname);
            const char *dir_str = str_dir[(int)pdir];
            eina_strbuf_append_printf(descparam, " * @param[%s] %s %s\n", dir_str, pname, pdesc?pdesc:"No description supplied.");
            if (eolian_parameter_is_nonull((Eolian_Function_Parameter)data))
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

   eina_strbuf_replace_all(fbody, "@#params", eina_strbuf_string_get(fparam));
   eina_strbuf_replace_all(fbody, "@#list_desc_param", eina_strbuf_string_get(descparam));
   eina_strbuf_reset(fparam);
   eina_strbuf_append_printf(fparam, "%s%s",
         rettype ? rettype : "void",
         rettype && strchr(rettype, '*')?"":" ");
   eina_strbuf_replace_all(fbody, "@#type_return", eina_strbuf_string_get(fparam));
   eina_strbuf_replace_all(fbody, "@#is_const", (ftype == EOLIAN_PROP_GET || eolian_function_object_is_const(funcid)) ? "const " : "");
   if (eolian_function_return_is_warn_unused(funcid, ftype))
     {
        Eina_Bool no_nonull = !flags;
        if (no_nonull) flags = eina_strbuf_new();
        eina_strbuf_prepend_printf(flags, " EINA_WARN_UNUSED_RESULT%s", !no_nonull?", ":"");
     }
   if (flags)
      eina_strbuf_replace_all(fbody, "@#flags", eina_strbuf_string_get(flags));
   eina_strbuf_replace_all(fbody, "@#flags", (eolian_function_return_is_warn_unused(funcid, ftype)) ? " EINA_WARN_UNUSED_RESULT" : "");
   eina_strbuf_append(buf, eina_strbuf_string_get(fbody));

end:
   eina_strbuf_free(flags);
   eina_strbuf_free(fbody);
   eina_strbuf_free(fparam);
   eina_strbuf_free(descparam);
}

static void
_eapi_func_generate(const char *classname, Eolian_Function funcid, Eolian_Function_Type ftype, Eina_Strbuf *buf)
{
   //TODO return value
   char tmpstr[0xFF];
   const char *suffix = "";
   const char *func_lpref = NULL;
   Eina_Bool var_as_ret = EINA_FALSE;
   const char *rettype = NULL;
   const char *retname = NULL;
   Eina_Bool ret_const = EINA_FALSE;
   Eina_Bool add_star = EINA_FALSE;
   Eina_Bool ret_is_void = EINA_FALSE;

   Eina_Strbuf *fbody = eina_strbuf_new();
   Eina_Strbuf *fparam = eina_strbuf_new();
   Eina_Strbuf *eoparam = eina_strbuf_new();

   rettype = eolian_function_return_type_get(funcid, ftype);
   if (rettype && !strcmp(rettype, "void")) ret_is_void = EINA_TRUE;
   retname = "ret";
   if (ftype == EOLIAN_PROP_GET)
     {
        suffix = "_get";
        add_star = EINA_TRUE;
        func_lpref = eolian_function_data_get(funcid, EOLIAN_LEGACY_GET);
        if (!rettype)
          {
             const Eina_List *l = eolian_parameters_list_get(funcid);
             if (eina_list_count(l) == 1)
               {
                  void* data = eina_list_data_get(l);
                  eolian_parameter_information_get((Eolian_Function_Parameter)data, NULL, &rettype, &retname, NULL);
                  var_as_ret = EINA_TRUE;
                  ret_const = eolian_parameter_const_attribute_get(data, EINA_TRUE);
               }
          }
     }
   if (ftype == EOLIAN_PROP_SET)
     {
        suffix = "_set";
        func_lpref = eolian_function_data_get(funcid, EOLIAN_LEGACY_SET);
     }

   func_lpref = (func_lpref) ? func_lpref : eolian_function_data_get(funcid, EOLIAN_LEGACY);
   if (func_lpref && !strcmp(func_lpref, "null")) goto end;

   _template_fill(fbody, tmpl_eapi_body, classname, NULL, EINA_FALSE);
   eina_strbuf_replace_all(fbody, "@#eoprefix", current_eo_prefix_lower);

   if (func_lpref)
      eina_strbuf_replace_all(fbody, "@#eapi_prefix_@#func", func_lpref);
   else
     {
        func_lpref = eolian_class_legacy_prefix_get(classname);
        if (func_lpref && !strcmp(func_lpref, "null")) goto end;

        if (func_lpref) eina_strbuf_replace_all(fbody, "@#eapi_prefix", func_lpref);
        else
          {
             strncpy(tmpstr, classname, sizeof(tmpstr) - 1);
             char *p = tmpstr;
             eina_str_tolower(&p);
             eina_strbuf_replace_all(fbody, "@#eapi_prefix", tmpstr);
          }
     }

   sprintf (tmpstr, "%s%s", eolian_function_name_get(funcid), suffix);
   eina_strbuf_replace_all(fbody, "@#func", tmpstr);

   const Eina_List *l;
   void *data;

   tmpstr[0] = '\0';

   EINA_LIST_FOREACH(eolian_property_keys_list_get(funcid), l, data)
     {
        const char *pname;
        const char *ptype;
        eolian_parameter_information_get((Eolian_Function_Parameter)data, NULL, &ptype, &pname, NULL);
        eina_strbuf_append_printf(fparam, ", %s%s %s",
              eolian_parameter_const_attribute_get(data, ftype == EOLIAN_PROP_GET)?"const ":"",
              ptype, pname);
        if (eina_strbuf_length_get(eoparam)) eina_strbuf_append(eoparam, ", ");
        eina_strbuf_append_printf(eoparam, "%s", pname);
     }
   if (!var_as_ret)
   {
       EINA_LIST_FOREACH(eolian_parameters_list_get(funcid), l, data)
         {
            const char *pname;
            const char *ptype;
            Eolian_Parameter_Dir pdir;
            eolian_parameter_information_get((Eolian_Function_Parameter)data, &pdir, &ptype, &pname, NULL);
            Eina_Bool had_star = !!strchr(ptype, '*');
            if (ftype == EOLIAN_UNRESOLVED || ftype == EOLIAN_METHOD) add_star = (pdir == EOLIAN_OUT_PARAM);
            eina_strbuf_append_printf(fparam, ", %s%s%s%s%s",
                  eolian_parameter_const_attribute_get(data, ftype == EOLIAN_PROP_GET)?"const ":"",
                  ptype, had_star?"":" ", add_star?"*":"", pname);
            if (eina_strbuf_length_get(eoparam)) eina_strbuf_append(eoparam, ", ");
            eina_strbuf_append_printf(eoparam, "%s", pname);
         }
   }

   char tmp_ret_str[0xFF];
   sprintf (tmp_ret_str, "%s%s", ret_const?"const ":"", rettype?rettype:"void");
   if (rettype && !ret_is_void)
     {
        if (eina_strbuf_length_get(eoparam)) eina_strbuf_append(eoparam, ", ");
        Eina_Bool had_star = !!strchr(rettype, '*');
        const char *dflt_ret_val =
           eolian_function_return_dflt_value_get(funcid, ftype);
        sprintf (tmpstr, "   %s%s%s%s = %s;\n",
              ret_const?"const ":"", rettype, had_star?"":" ", retname,
              dflt_ret_val?dflt_ret_val:"0");
        eina_strbuf_append_printf(eoparam, "&%s", retname);
     }

   eina_strbuf_replace_all(fbody, "@#full_params", eina_strbuf_string_get(fparam));
   eina_strbuf_replace_all(fbody, "@#eo_params", eina_strbuf_string_get(eoparam));
   eina_strbuf_replace_all(fbody, "@#ret_type", tmp_ret_str);
   eina_strbuf_replace_all(fbody, "@#ret_init_val", tmpstr);
   eina_strbuf_replace_all(fbody, "@#ret_val", (rettype && !ret_is_void) ? retname : "");
   eina_strbuf_replace_all(fbody, "@#is_const", (ftype == EOLIAN_PROP_GET || eolian_function_object_is_const(funcid)) ? "const " : "");

   eina_strbuf_append(buf, eina_strbuf_string_get(fbody));

end:
   eina_strbuf_free(fbody);
   eina_strbuf_free(fparam);
   eina_strbuf_free(eoparam);
}

Eina_Bool
legacy_header_generate(const char *classname, int eo_version EINA_UNUSED, Eina_Strbuf *buf)
{
   const Eolian_Function_Type ftype_order[] = {EOLIAN_PROPERTY, EOLIAN_METHOD};
   const Eina_List *l;
   void *data;

   if (!eolian_class_exists(classname))
     {
        printf ("Class \"%s\" not found in database\n", classname);
        return EINA_FALSE;
     }

   int i;
   for (i = 0; i < 2; i++)
      EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, ftype_order[i]), l, data)
        {
           const Eolian_Function_Type ftype = eolian_function_type_get((Eolian_Function)data);
           Eina_Bool prop_read = (ftype == EOLIAN_PROPERTY || ftype == EOLIAN_PROP_GET ) ? EINA_TRUE : EINA_FALSE ;
           Eina_Bool prop_write = (ftype == EOLIAN_PROPERTY || ftype == EOLIAN_PROP_SET ) ? EINA_TRUE : EINA_FALSE ;

           if (!prop_read && !prop_write)
             {
                _eapi_decl_func_generate(classname, (Eolian_Function)data, EOLIAN_METHOD, buf);
             }
           if (prop_write)
             {
                _eapi_decl_func_generate(classname, (Eolian_Function)data, EOLIAN_PROP_SET, buf);
             }
           if (prop_read)
             {
                _eapi_decl_func_generate(classname, (Eolian_Function)data, EOLIAN_PROP_GET, buf);
             }
        }
   return EINA_TRUE;
}

Eina_Bool
legacy_source_generate(const char *classname, int eo_version EINA_UNUSED, Eina_Strbuf *buf)
{
   Eina_Bool ret = EINA_FALSE;
   const Eina_List *itr;
   Eolian_Function fn;

   if (!eolian_class_exists(classname))
     {
        printf ("Class \"%s\" not found in database\n", classname);
        return EINA_FALSE;
     }

   Eina_Strbuf *tmpbuf = eina_strbuf_new();
   Eina_Strbuf *str_bodyf = eina_strbuf_new();

   //Properties
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, EOLIAN_PROPERTY), itr, fn)
     {
        const Eolian_Function_Type ftype = eolian_function_type_get(fn);

        Eina_Bool prop_read = ( ftype == EOLIAN_PROP_SET ) ? EINA_FALSE : EINA_TRUE;
        Eina_Bool prop_write = ( ftype == EOLIAN_PROP_GET ) ? EINA_FALSE : EINA_TRUE;

        if (prop_write)
          {
             _eapi_func_generate(classname, fn, EOLIAN_PROP_SET, str_bodyf);
          }
        if (prop_read)
          {
             _eapi_func_generate(classname, fn, EOLIAN_PROP_GET, str_bodyf);
          }
     }

   //Methods
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, EOLIAN_METHOD), itr, fn)
     {
        _eapi_func_generate(classname, fn, EOLIAN_UNRESOLVED, str_bodyf);
     }

   eina_strbuf_append(buf, eina_strbuf_string_get(str_bodyf));

   ret = EINA_TRUE;
   eina_strbuf_free(tmpbuf);
   eina_strbuf_free(str_bodyf);

   return ret;
}
