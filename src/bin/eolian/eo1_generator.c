#include <Eina.h>
#include <string.h>

#include "Eolian.h"
#include "eo1_generator.h"
#include "common_funcs.h"

static const char
tmpl_dtor[] = "\
static void\n\
_gen_@#class_class_destructor(Eo_Class *klass)\n\
{\n\
   _@#class_class_destructor(klass);\n\
}\n\
\n\
";

static const char
tmpl_eo1_class_ctor[] = "\
static void\n\
_gen_@#class_class_constructor(Eo_Class *klass)\n\
{\n\
   const Eo_Op_Func_Description func_desc[] = {@#list_func\n\
        EO_OP_FUNC_SENTINEL\n\
   };\n\
   eo_class_funcs_set(klass, func_desc);\n\
@#user_ctor_func\
}\n\n";

static const char
tmpl_eo1_ops_desc[] = "\
static const Eo_Op_Description _@#class_op_desc[] = {@#list_op\n\
     EO_OP_DESCRIPTION_SENTINEL\n\
};\n\n";

static const char
tmpl_eo_ops_desc[] = "\
static Eo_Op_Description _@#class_op_desc[] = {@#list_op\n\
     EO_OP_SENTINEL\n\
};\n\n";

static const char
tmpl_events_desc[] = "\
static const Eo_Event_Description *_@#class_event_desc[] = {@#list_evdesc\n\
     NULL\n\
};\n\n";

static const char
tmpl_eo_src[] = "\
@#functions_body\
\n\
@#ctor_func\
@#dtor_func\
@#ops_desc\
@#events_desc\
static const Eo_Class_Description _@#class_class_desc = {\n\
     @#EO_VERSION,\n\
     \"@#Class\",\n\
     @#type_class,\n\
     @#eo_class_desc_ops,\n\
     @#Events_Desc,\n\
     @#SizeOfData,\n\
     @#ctor_name,\n\
     @#dtor_name\n\
};\n\
\n\
EO_DEFINE_CLASS(@#eoprefix_class_get, &_@#class_class_desc, @#list_inheritNULL);\
";

static const char
tmpl_eo1_op_desc[] = "\n     EO_OP_DESCRIPTION(@#EOPREFIX_SUB_ID_@#FUNC, \"@#desc\"),";

static const char
tmpl_eo1_func_desc[] = "\n        EO_OP_FUNC(@#EOPREFIX_ID(@#EOPREFIX_SUB_ID_@#FUNC), _eo_obj_@#class_@#func),";

static const char
tmpl_eo_func_desc[] = "\n     EO_OP_FUNC_OVERRIDE(@#eoprefix_@#func, _@#class_@#func),";

static const char
tmpl_eo1base_func_desc[] = "\n        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_@#FUNC), _eo_obj_@#class_@#func),";

static const char
tmpl_eo_op_desc[] = "\n     EO_OP_FUNC(@#eoprefix_@#func, _@#class_@#func, \"@#desc\"),";

static const char
tmpl_eo_obj_header[] = "\
#define @#EOPREFIX_CLASS @#eoprefix_class_get()\n\
\n\
const Eo_Class *@#eoprefix_class_get(void) EINA_CONST;\n\
\n\
extern EAPI Eo_Op @#EOPREFIX_BASE_ID;\n\
\n\
enum\n\
{@#list_subid\n\
   @#EOPREFIX_SUB_ID_LAST\n\
};\n\
\n\
#define @#EOPREFIX_ID(sub_id) (@#EOPREFIX_BASE_ID + sub_id)\n\n\
";

static const char
tmpl_eo_obj_header_no_ids[] = "\
#define @#EOPREFIX_CLASS @#eoprefix_class_get()\n\
\n\
const Eo_Class *@#eoprefix_class_get(void) EINA_CONST;\n\
\n\
";

static const char
tmpl_eo_subid[] = "\n   @#EOPREFIX_SUB_ID_@#FUNC,";

static const char
tmpl_eo_subid_apnd[] = "   @#EOPREFIX_SUB_ID_@#FUNC,\n";

static const char
tmpl_eo_funcdef_doxygen[] = "\
/**\n\
 * @def @#eoprefix_@#func\n\
 *\n\
@#desc\n\
 *\n\
@#list_desc_param\
@#ret_desc\
 *\n\
 */\n";

static const char
tmpl_eo1_funcdef[] = "#define @#eoprefix_@#func(@#list_param) @#EOPREFIX_ID(@#EOPREFIX_SUB_ID_@#FUNC)@#list_typecheck\n";

static const char
tmpl_eo_funcdef[] = "EAPI @#rettype @#eoprefix_@#func(@#full_params);\n";

static const char
tmpl_eo_pardesc[] =" * @param[%s] %s %s\n";

#if 0
static const char
tmpl_eo_retdesc[] =" * @return %s\n";
#endif

static const char
tmpl_eo1bind_body[] ="\
\n\
@#ret_type _@#class_@#func(Eo *obj, @#Datatype_Data *pd@#full_params);\n\n\
static void\n\
_eo_obj_@#class_@#func(Eo *obj, void *_pd, va_list *list@#list_unused)\n\
{\n\
@#list_vars\
   @#ret_param_@#class_@#func(obj, _pd, @#list_params);\n\
@#return_ret\
}\n\
";

static Eina_Bool
eo1_fundef_generate(const char *classname, Eolian_Function func, Eolian_Function_Type ftype, Eina_Strbuf *functext)
{
   const char *str_dir[] = {"in", "out", "inout"};
   const Eina_List *l;
   void *data;
   char funcname[0xFF];
   char descname[0xFF];
   char *tmpstr = malloc(0x1FF);
   Eina_Bool var_as_ret = EINA_FALSE;
   const char *rettype = NULL;
   Eina_Bool ret_const = EINA_FALSE;
   Eolian_Function_Scope scope = eolian_function_scope_get(func);

   char *fsuffix = "";
   rettype = eolian_function_return_type_get(func, ftype);
   if (rettype && !strcmp(rettype, "void")) rettype = NULL;
   if (ftype == EOLIAN_PROP_GET)
     {
        fsuffix = "_get";
        if (!rettype)
          {
             l = eolian_parameters_list_get(func);
             if (eina_list_count(l) == 1)
               {
                  data = eina_list_data_get(l);
                  eolian_parameter_information_get((Eolian_Function_Parameter)data, NULL, &rettype, NULL, NULL);
                  var_as_ret = EINA_TRUE;
                  ret_const = eolian_parameter_const_attribute_get(data, EINA_TRUE);
               }
          }
     }
   if (ftype == EOLIAN_PROP_SET) fsuffix = "_set";

   sprintf (funcname, "%s%s", eolian_function_name_get(func), fsuffix);
   sprintf (descname, "comment%s", fsuffix);
   const char *funcdesc = eolian_function_description_get(func, descname);

   Eina_Strbuf *str_func = eina_strbuf_new();
   if (scope == EOLIAN_SCOPE_PROTECTED)
      eina_strbuf_append_printf(str_func, "#ifdef %s_PROTECTED\n", capclass);
   _template_fill(str_func, tmpl_eo_funcdef_doxygen, classname, funcname, EINA_FALSE);
#ifndef EO
   _template_fill(str_func, tmpl_eo1_funcdef, classname, funcname, EINA_FALSE);
#else
   _template_fill(str_func, tmpl_eo_funcdef, classname, funcname, EINA_FALSE);
   if (scope == EOLIAN_SCOPE_PROTECTED)
      eina_strbuf_append_printf(str_func, "#endif\n");
#endif
   eina_strbuf_append_printf(str_func, "\n");

   eina_strbuf_replace_all(str_func, "@#EOPREFIX", current_eo_prefix_upper);
   eina_strbuf_replace_all(str_func, "@#eoprefix", current_eo_prefix_lower);

   Eina_Strbuf *linedesc = eina_strbuf_new();
   eina_strbuf_append(linedesc, funcdesc ? funcdesc : "No description supplied.");
   if (eina_strbuf_length_get(linedesc))
     {
        eina_strbuf_replace_all(linedesc, "\n", "\n * ");
        eina_strbuf_replace_all(linedesc, " * \n", " *\n");
        eina_strbuf_prepend(linedesc, " * ");
     }
   else
     {
        eina_strbuf_append(linedesc, " *");
     }

   eina_strbuf_replace_all(str_func, "@#desc", eina_strbuf_string_get(linedesc));
   eina_strbuf_free(linedesc);

   Eina_Strbuf *str_par = eina_strbuf_new();
   Eina_Strbuf *str_pardesc = eina_strbuf_new();
   Eina_Strbuf *str_retdesc = eina_strbuf_new();
   Eina_Strbuf *str_typecheck = eina_strbuf_new();

   EINA_LIST_FOREACH(eolian_property_keys_list_get(func), l, data)
     {
        const char *pname;
        const char *ptype;
        const char *pdesc = NULL;
        eolian_parameter_information_get((Eolian_Function_Parameter)data, NULL, &ptype, &pname, &pdesc);

        eina_strbuf_append_printf(str_pardesc, tmpl_eo_pardesc, "in", pname, pdesc?pdesc:"No description supplied.");

        if (eina_strbuf_length_get(str_par)) eina_strbuf_append(str_par, ", ");
#ifdef EO
        eina_strbuf_append_printf(str_par, "%s %s", ptype, pname);
#else
        eina_strbuf_append(str_par, pname);

        eina_strbuf_append_printf(str_typecheck, ", EO_TYPECHECK(%s, %s)", ptype, pname);
#endif
     }

   if (!var_as_ret)
     {
        EINA_LIST_FOREACH(eolian_parameters_list_get(func), l, data)
          {
             const char *pname;
             const char *ptype;
             const char *pdesc;
             Eina_Bool add_star = EINA_FALSE;
             Eolian_Parameter_Dir pdir;
             eolian_parameter_information_get((Eolian_Function_Parameter)data, &pdir, &ptype, &pname, &pdesc);
             Eina_Bool is_const = eolian_parameter_const_attribute_get(data, ftype == EOLIAN_PROP_GET);
             if (ftype == EOLIAN_PROP_GET) {
                  add_star = EINA_TRUE;
                  pdir = EOLIAN_OUT_PARAM;
             }
             if (ftype == EOLIAN_PROP_SET) pdir = EOLIAN_IN_PARAM;
             if (ftype == EOLIAN_UNRESOLVED || ftype == EOLIAN_METHOD) add_star = (pdir == EOLIAN_OUT_PARAM);
             Eina_Bool had_star = !!strchr(ptype, '*');

             const char *dir_str = str_dir[(int)pdir];

             eina_strbuf_append_printf(str_pardesc, tmpl_eo_pardesc, dir_str, pname, pdesc?pdesc:"No description supplied.");

             if (eina_strbuf_length_get(str_par)) eina_strbuf_append(str_par, ", ");
#ifdef EO
             eina_strbuf_append_printf(str_par, "%s%s%s%s%s",
                   is_const?"const ":"",
                   ptype, had_star?"":" ", add_star?"*":"", pname);
#else
             eina_strbuf_append(str_par, pname);

             eina_strbuf_append_printf(str_typecheck, ", EO_TYPECHECK(%s%s%s%s, %s)",
                   is_const?"const ":"",
                   ptype, had_star?"":" ", add_star?"*":"", pname);
#endif

          }
     }

   if (rettype && strcmp(rettype, "void"))
     {
#ifndef EO
        const char *ret_desc = eolian_function_return_comment_get(func, ftype);
        eina_strbuf_append_printf(str_pardesc, tmpl_eo_pardesc, "out", "ret", ret_desc);
        if (eina_strbuf_length_get(str_par)) eina_strbuf_append(str_par, ", ");
        eina_strbuf_append(str_par, "ret");
        Eina_Bool had_star = !!strchr(rettype, '*');
        eina_strbuf_append_printf(str_typecheck, ", EO_TYPECHECK(%s%s%s*, ret)",
              ret_const ? "const " : "", rettype, had_star?"":" ");

#else
/*        eina_strbuf_append_printf(str_retdesc, tmpl_eo_retdesc, ret_desc); */
#endif
     }
#ifdef EO
   tmpstr[0] = '\0';
   sprintf(tmpstr, "%s%s%s",
         ret_const ? "const " : "",
         rettype ? rettype : "void",
         rettype && strchr(rettype, '*')?"":" ");
   eina_strbuf_replace_all(str_func, "@#rettype", tmpstr);
#endif

   eina_strbuf_replace_all(str_func, "@#list_param", eina_strbuf_string_get(str_par));
   if (!eina_strbuf_length_get(str_par)) eina_strbuf_append(str_par, "void");
   eina_strbuf_replace_all(str_func, "@#full_params", eina_strbuf_string_get(str_par));
   eina_strbuf_replace_all(str_func, "@#list_desc_param", eina_strbuf_string_get(str_pardesc));
   eina_strbuf_replace_all(str_func, "@#ret_desc", eina_strbuf_string_get(str_retdesc));
   eina_strbuf_replace_all(str_func, "@#list_typecheck", eina_strbuf_string_get(str_typecheck));

   free(tmpstr);
   eina_strbuf_free(str_par);
   eina_strbuf_free(str_retdesc);
   eina_strbuf_free(str_pardesc);
   eina_strbuf_free(str_typecheck);

   eina_strbuf_append(functext, eina_strbuf_string_get(str_func));
   eina_strbuf_free(str_func);

   return EINA_TRUE;
}

Eina_Bool
eo1_header_generate(const char *classname, Eina_Strbuf *buf)
{
   const Eolian_Function_Type ftype_order[] = {EOLIAN_CTOR, EOLIAN_PROPERTY, EOLIAN_METHOD};
   const Eina_List *l;
   void *data;
   char *tmpstr = malloc(0x1FF);
   Eina_Strbuf * str_hdr = eina_strbuf_new();

   if (!eolian_class_exists(classname))
     {
        ERR ("Class \"%s\" not found in database", classname);
        free(tmpstr);
        return EINA_FALSE;
     }

#ifndef EO
   if (eolian_class_functions_list_get(classname, EOLIAN_CTOR) ||
         eolian_class_functions_list_get(classname, EOLIAN_DTOR) ||
         eolian_class_functions_list_get(classname, EOLIAN_PROPERTY) ||
         eolian_class_functions_list_get(classname, EOLIAN_METHOD))
     {
        _template_fill(str_hdr, tmpl_eo_obj_header, classname, "", EINA_TRUE);
     }
   else
#endif
     {
        _template_fill(str_hdr, tmpl_eo_obj_header_no_ids, classname, "", EINA_TRUE);
     }

   eina_strbuf_replace_all(str_hdr, "@#EOPREFIX", current_eo_prefix_upper);
   eina_strbuf_replace_all(str_hdr, "@#eoprefix", current_eo_prefix_lower);

   Eina_Strbuf *str_subid = eina_strbuf_new();
   Eina_Strbuf *str_ev = eina_strbuf_new();
   Eina_Strbuf *str_extrn_ev = eina_strbuf_new();
   Eina_Strbuf *tmpbuf = eina_strbuf_new();

   Eolian_Event event;
   EINA_LIST_FOREACH(eolian_class_events_list_get(classname), l, event)
     {
        const char *evname = NULL;
        const char *evdesc = NULL;
        eolian_class_event_information_get(event, &evname, NULL, &evdesc);

        if (!evdesc) evdesc = "No description";
        eina_strbuf_reset(tmpbuf);
        eina_strbuf_append(tmpbuf, evdesc);
        eina_strbuf_replace_all(tmpbuf, "\n", "\n * ");
        eina_strbuf_prepend(tmpbuf," * ");
        eina_strbuf_append_printf(str_ev, "\n/**\n%s\n */\n", eina_strbuf_string_get(tmpbuf));

        _template_fill(tmpbuf, "@#CLASS_EVENT_@#FUNC", classname, evname, EINA_TRUE);
        eina_strbuf_replace_all(tmpbuf, ",", "_");
        const char* s = eina_strbuf_string_get(tmpbuf);
        eina_strbuf_append_printf(str_ev, "#define %s (&(_%s))\n", s, s);
        eina_strbuf_append_printf(str_extrn_ev, "EAPI extern const Eo_Event_Description _%s;\n", s);
     }

   int i;
   for (i = 0; i < 3; i++)
      EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, ftype_order[i]), l, data)
        {
           const Eolian_Function_Type ftype = eolian_function_type_get((Eolian_Function)data);
           const char *funcname = eolian_function_name_get((Eolian_Function)data);
           Eina_Bool prop_read = (ftype == EOLIAN_PROPERTY || ftype == EOLIAN_PROP_GET ) ? EINA_TRUE : EINA_FALSE ;
           Eina_Bool prop_write = (ftype == EOLIAN_PROPERTY || ftype == EOLIAN_PROP_SET ) ? EINA_TRUE : EINA_FALSE ;

           if (!prop_read && !prop_write)
             {
                _template_fill(str_subid, tmpl_eo_subid, classname, funcname, EINA_FALSE);
                eo1_fundef_generate(classname, (Eolian_Function)data, EOLIAN_UNRESOLVED, str_hdr);
             }
           if (prop_write)
             {
                sprintf(tmpstr, "%s_set", funcname);
                _template_fill(str_subid, tmpl_eo_subid, classname, tmpstr, EINA_FALSE);
                eo1_fundef_generate(classname, (Eolian_Function)data, EOLIAN_PROP_SET, str_hdr);
             }
           if (prop_read)
             {
                sprintf(tmpstr, "%s_get", funcname);
                _template_fill(str_subid, tmpl_eo_subid, classname, tmpstr, EINA_FALSE);
                eo1_fundef_generate(classname, (Eolian_Function)data, EOLIAN_PROP_GET, str_hdr);
             }
           eina_strbuf_replace_all(str_subid, "@#EOPREFIX", current_eo_prefix_upper);
        }

   eina_strbuf_replace_all(str_hdr, "@#list_subid", eina_strbuf_string_get(str_subid));
   eina_strbuf_append(str_hdr, eina_strbuf_string_get(str_extrn_ev));
   eina_strbuf_append(str_hdr, eina_strbuf_string_get(str_ev));

   eina_strbuf_append(buf, eina_strbuf_string_get(str_hdr));

   free(tmpstr);
   eina_strbuf_free(str_subid);
   eina_strbuf_free(str_ev);
   eina_strbuf_free(str_extrn_ev);
   eina_strbuf_free(tmpbuf);
   eina_strbuf_free(str_hdr);

   return EINA_TRUE;
}

#ifndef EO
static const char*
_varg_upgr(const char *stype)
{
   if (!strcmp(stype, "Eina_Bool") ||
     !strcmp(stype, "char") ||
     !strcmp(stype, "short") ||
     !strcmp(stype, "unsigned short"))
     return "int";

   return stype;
}
#endif

static Eina_Bool
eo1_bind_func_generate(const char *classname, Eolian_Function funcid, Eolian_Function_Type ftype, Eina_Strbuf *buf, const char *impl_name)
{
   const char *suffix = "";
   Eina_Bool var_as_ret = EINA_FALSE;
   const char *rettype = NULL;
   const char *retname = NULL;
   Eina_Bool ret_const = EINA_FALSE;
   Eina_Bool add_star = EINA_FALSE;

   Eina_Bool need_implementation = EINA_TRUE;
#ifndef EO
   if (!impl_name && eolian_function_is_virtual_pure(funcid, ftype)) return EINA_TRUE;
#else
   if (!impl_name && eolian_function_is_virtual_pure(funcid, ftype)) need_implementation = EINA_FALSE;
#endif

   Eina_Strbuf *fbody = eina_strbuf_new();
   Eina_Strbuf *va_args = eina_strbuf_new();
   Eina_Strbuf *params = eina_strbuf_new(); /* only variables names */
   Eina_Strbuf *full_params = eina_strbuf_new(); /* variables types + names */

   rettype = eolian_function_return_type_get(funcid, ftype);
   if (rettype && !strcmp(rettype, "void")) rettype = NULL;
   retname = "ret";
   if (ftype == EOLIAN_PROP_GET)
     {
        suffix = "_get";
        add_star = EINA_TRUE;
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
     }

   char tmpstr[0xFF];
   sprintf (tmpstr, "%s%s", eolian_function_name_get(funcid), suffix);
   char tmpstr2[0xFF];
   sprintf (tmpstr2, "%s_%s", classname, impl_name);
#ifndef EO
   _template_fill(fbody, tmpl_eo1bind_body, impl_name?tmpstr2:classname, tmpstr, EINA_FALSE);
#else
   _class_func_names_fill(impl_name?tmpstr2:classname, tmpstr);
#endif

   const Eina_List *l;
   void *data;

   EINA_LIST_FOREACH(eolian_property_keys_list_get(funcid), l, data)
     {
        const char *pname;
        const char *ptype;
        eolian_parameter_information_get((Eolian_Function_Parameter)data, NULL, &ptype, &pname, NULL);
        Eina_Bool is_const = eolian_parameter_const_attribute_get(data, ftype == EOLIAN_PROP_GET);
#ifndef EO
        eina_strbuf_append_printf(va_args, "   %s%s %s = va_arg(*list, %s%s);\n",
              is_const?"const ":"", ptype, pname,
              is_const?"const ":"", _varg_upgr(ptype));
#endif
        eina_strbuf_append_printf(params, ", %s", pname);
        eina_strbuf_append_printf(full_params, ", %s%s %s",
              is_const?"const ":"",
              ptype, pname);
     }
   if (!var_as_ret)
     {
        EINA_LIST_FOREACH(eolian_parameters_list_get(funcid), l, data)
          {
             const char *pname;
             const char *ptype;
             Eolian_Parameter_Dir pdir;
             eolian_parameter_information_get((Eolian_Function_Parameter)data, &pdir, &ptype, &pname, NULL);
             Eina_Bool is_const = eolian_parameter_const_attribute_get(data, ftype == EOLIAN_PROP_GET);
             Eina_Bool had_star = !!strchr(ptype, '*');
             if (ftype == EOLIAN_UNRESOLVED || ftype == EOLIAN_METHOD) add_star = (pdir == EOLIAN_OUT_PARAM);
#ifndef EO
             eina_strbuf_append_printf(va_args, "   %s%s%s%s%s = va_arg(*list, %s%s%s%s);\n",
                   is_const?"const ":"", ptype, had_star?"":" ", add_star?"*":"", pname,
                   is_const?"const ":"", add_star ? ptype : _varg_upgr(ptype), !had_star && add_star?" ":"", add_star?"*":"");
#endif
             if (eina_strbuf_length_get(params)) eina_strbuf_append(params, ", ");
             eina_strbuf_append_printf(params, "%s", pname);
             eina_strbuf_append_printf(full_params, ", %s%s%s%s%s",
                   is_const?"const ":"",
                   ptype, had_star?"":" ", add_star?"*":"", pname);
          }
     }

#ifndef EO
   if (rettype && strcmp(rettype, "void"))
     {
        Eina_Bool had_star = !!strchr(rettype, '*');
        eina_strbuf_append_printf(va_args, "   %s%s%s*%s = va_arg(*list, %s%s%s*);\n",
              ret_const?"const ":"",
              rettype, had_star?"":" ", retname,
              ret_const?"const ":"",
              rettype, had_star?"":" ");
        Eina_Strbuf *ret_param = eina_strbuf_new();
        if (rettype) eina_strbuf_append_printf(ret_param, "%s%s%s_%s = ", ret_const?"const ":"", rettype, had_star?"":" ", retname);
        eina_strbuf_replace_all(fbody, "@#ret_param", eina_strbuf_string_get(ret_param));
        sprintf(tmpstr, "%s%s", ret_const?"const ":"", rettype);
        eina_strbuf_replace_all(fbody, "@#ret_type", tmpstr);
        sprintf(tmpstr, "   if (%s) *%s = _%s;\n", retname, retname, retname);
        eina_strbuf_replace_all(fbody, "@#return_ret", tmpstr);
        eina_strbuf_free(ret_param);
     }
   else
     {
        eina_strbuf_replace_all(fbody, "@#ret_param", "");
        eina_strbuf_replace_all(fbody, "@#ret_type", "void");
        eina_strbuf_replace_all(fbody, "@#return_ret", "");
     }

   if (eina_list_count(eolian_parameters_list_get(funcid)) == 0 &&
         (eina_list_count(eolian_property_keys_list_get(funcid)) == 0))
     {
        eina_strbuf_replace_all(fbody, "@#list_unused", " EINA_UNUSED");
     }
   else
     {
        eina_strbuf_replace_all(fbody, "@#list_unused", "");
     }
   if (!eina_strbuf_length_get(params)) eina_strbuf_replace_all(fbody, ", @#list_params", "");
   eina_strbuf_replace_all(fbody, "@#list_vars", eina_strbuf_string_get(va_args));
   eina_strbuf_replace_all(fbody, "@#list_params", eina_strbuf_string_get(params));
#else
   if (need_implementation)
     {
        Eina_Strbuf *ret_param = eina_strbuf_new();
        eina_strbuf_append_printf(fbody, "\n");
        eina_strbuf_append_printf(fbody, "%s%s _%s_%s%s(Eo *obj, @#Datatype_Data *pd@#full_params);\n\n",
              ret_const?"const ":"", rettype?rettype:"void",
              lowclass,
              eolian_function_name_get(funcid), suffix?suffix:"");

        eina_strbuf_replace_all(fbody, "@#return_ret", tmpstr);
        eina_strbuf_free(ret_param);
     }
   if (!impl_name)
     {
        Eina_Strbuf *eo_func_decl = eina_strbuf_new();
        Eina_Bool has_params =
           !var_as_ret &&
           (eina_list_count(eolian_parameters_list_get(funcid)) != 0 ||
           eina_list_count(eolian_property_keys_list_get(funcid)));
        Eina_Bool ret_is_void = (!rettype || !strcmp(rettype, "void"));
        eina_strbuf_append_printf(eo_func_decl,
              "EAPI EO_%sFUNC_BODY%s(%s_%s%s",
              ret_is_void?"VOID_":"", has_params?"V":"",
              current_eo_prefix_lower, eolian_function_name_get(funcid),
              suffix?suffix:"");
        if (!ret_is_void)
          {
             const char *dflt_ret_val =
                eolian_function_return_dflt_value_get(funcid, ftype);
             eina_strbuf_append_printf(eo_func_decl, ", %s%s, %s",
                   ret_const ? "const " : "", rettype,
                   dflt_ret_val?dflt_ret_val:"0");

          }
        if (has_params)
          {
             eina_strbuf_append_printf(eo_func_decl, ", EO_FUNC_CALL(%s)%s",
                   eina_strbuf_string_get(params),
                   eina_strbuf_string_get(full_params));
          }
        eina_strbuf_append_printf(eo_func_decl, ");");
        eina_strbuf_append_printf(fbody, "%s\n", eina_strbuf_string_get(eo_func_decl));
        eina_strbuf_free(eo_func_decl);
     }
#endif
   if (need_implementation)
     {
        eina_strbuf_replace_all(fbody, "@#full_params", eina_strbuf_string_get(full_params));
        const char *data_type = eolian_class_data_type_get(classname);
        if (data_type && !strcmp(data_type, "null"))
           eina_strbuf_replace_all(fbody, "@#Datatype_Data", "void");
        else
          {
             if (data_type) eina_strbuf_replace_all(fbody, "@#Datatype_Data", data_type);
             else eina_strbuf_replace_all(fbody, "@#Datatype", classname);
          }

        if (!data_type || !strcmp(data_type, "null"))
           eina_strbuf_replace_all(fbody, "@#Datatype", classname);
        else
           eina_strbuf_replace_all(fbody, "@#Datatype_Data", data_type);
     }
   eina_strbuf_append(buf, eina_strbuf_string_get(fbody));

   eina_strbuf_free(va_args);
   eina_strbuf_free(full_params);
   eina_strbuf_free(params);
   eina_strbuf_free(fbody);
   return EINA_TRUE;
}

static Eina_Bool
eo1_eo_func_desc_generate(const char *class_name, const char *impl_name, const char *func_name, Eina_Strbuf *buf)
{
   if (impl_name)
      _class_func_names_fill(impl_name, func_name);
   else
      _class_func_names_fill(class_name, func_name);
   eina_strbuf_reset(buf);
   eina_strbuf_append_printf(buf,
         "\n        EO_OP_FUNC(%s_ID(%s_SUB_ID_%s), _eo_obj_%s%s%s_%s),",
         current_eo_prefix_upper, current_eo_prefix_upper, capfunc,
         lowclass, impl_name?"_":"", impl_name?impl_name:"",
         func_name);
   return EINA_TRUE;
}

static Eina_Bool
eo_op_desc_generate(const char *classname, Eolian_Function fid, Eolian_Function_Type ftype,
      const char *desc, Eina_Strbuf *buf)
{
   const char *funcname = eolian_function_name_get(fid);
   const char *suffix = "";

   eina_strbuf_reset(buf);
   _class_func_names_fill(classname, funcname);
#ifndef EO
   if (ftype == EOLIAN_PROP_GET) suffix = "_GET";
   if (ftype == EOLIAN_PROP_SET) suffix = "_SET";
   eina_strbuf_append_printf(buf, "\n     EO_OP_DESCRIPTION(%s_SUB_ID_%s%s, \"%s\"),",
         current_eo_prefix_upper, capfunc, suffix, desc);
#else
   if (ftype == EOLIAN_PROP_GET) suffix = "_get";
   if (ftype == EOLIAN_PROP_SET) suffix = "_set";
   Eina_Bool is_virtual_pure = eolian_function_is_virtual_pure(fid, ftype);
   eina_strbuf_append_printf(buf, "\n     EO_OP_FUNC(%s_%s%s, ", current_eo_prefix_lower, funcname, suffix);
   if (!is_virtual_pure)
      eina_strbuf_append_printf(buf, "_%s_%s%s, \"%s\"),", lowclass, funcname, suffix, desc);
   else
      eina_strbuf_append_printf(buf, "NULL, \"%s\"),", desc);
#endif

   return EINA_TRUE;
}

static Eina_Bool
eo1_source_beginning_generate(const char *classname, Eina_Strbuf *buf)
{
   const Eina_List *itr;

   Eina_Strbuf *tmpbuf = eina_strbuf_new();
   Eina_Strbuf *str_ev = eina_strbuf_new();

#ifndef EO
   if (eolian_class_functions_list_get(classname, EOLIAN_CTOR) ||
         eolian_class_functions_list_get(classname, EOLIAN_DTOR) ||
         eolian_class_functions_list_get(classname, EOLIAN_PROPERTY) ||
         eolian_class_functions_list_get(classname, EOLIAN_METHOD))
     {
        _template_fill(NULL, NULL, classname, NULL, EINA_TRUE);
        eina_strbuf_append_printf(buf, "EAPI Eo_Op @#EOPREFIX_BASE_ID = EO_NOOP;\n");
        eina_strbuf_replace_all(buf, "@#EOPREFIX", current_eo_prefix_upper);
     }
#endif

   Eolian_Event event;
   EINA_LIST_FOREACH(eolian_class_events_list_get(classname), itr, event)
     {
        const char *evname;
        const char *evdesc;
        char *evdesc_line1;

        eolian_class_event_information_get(event, &evname, NULL, &evdesc);
        evdesc_line1 = _source_desc_get(evdesc);
        _template_fill(str_ev, "@#CLASS_EVENT_@#FUNC", classname, evname, EINA_TRUE);
        eina_strbuf_replace_all(str_ev, ",", "_");

        eina_strbuf_append_printf(tmpbuf,
                                  "EAPI const Eo_Event_Description _%s =\n   EO_EVENT_DESCRIPTION(\"%s\", \"%s\");\n",
                                  eina_strbuf_string_get(str_ev), evname, evdesc_line1);
        free(evdesc_line1);
     }

   eina_strbuf_append(buf, eina_strbuf_string_get(tmpbuf));

   eina_strbuf_free(str_ev);
   eina_strbuf_free(tmpbuf);
   return EINA_TRUE;
}

static Eina_Bool
eo1_source_end_generate(const char *classname, Eina_Strbuf *buf)
{
   Eina_Bool ret = EINA_FALSE;
   const Eina_List *itr;
   Eolian_Function fn;

   const char *str_classtype = NULL;
   switch(eolian_class_type_get(classname))
    {
      case EOLIAN_CLASS_REGULAR:
        str_classtype = "EO_CLASS_TYPE_REGULAR";
        break;
      case EOLIAN_CLASS_ABSTRACT:
        str_classtype = "EO_CLASS_TYPE_REGULAR_NO_INSTANT";
        break;
      case EOLIAN_CLASS_MIXIN:
        str_classtype = "EO_CLASS_TYPE_MIXIN";
        break;
      case EOLIAN_CLASS_INTERFACE:
        str_classtype = "EO_CLASS_TYPE_INTERFACE";
        break;
      default:
        break;
    }

   if (!str_classtype)
     {
        ERR ("Unknown class type for class %s !", classname);
        return EINA_FALSE;
     }

   Eina_Strbuf *str_end = eina_strbuf_new();
   Eina_Strbuf *tmpbuf = eina_strbuf_new();
   Eina_Strbuf *str_op = eina_strbuf_new();
   Eina_Strbuf *str_func = eina_strbuf_new();
   Eina_Strbuf *str_bodyf = eina_strbuf_new();
   Eina_Strbuf *str_ev = eina_strbuf_new();
   Eina_Strbuf *tmpl_impl = eina_strbuf_new();

   _template_fill(str_end, tmpl_eo_src, classname, NULL, EINA_TRUE);

   eina_strbuf_replace_all(str_end, "@#EO_VERSION",
#ifdef EO
         "EO_VERSION"
#else
         "EO_VERSION"
#endif
         );


   eina_strbuf_replace_all(str_end, "@#type_class", str_classtype);
   eina_strbuf_replace_all(str_end, "@#EOPREFIX", current_eo_prefix_upper);
   eina_strbuf_replace_all(str_end, "@#eoprefix", current_eo_prefix_lower);

   eina_strbuf_reset(tmpbuf);
#ifndef EO
   _template_fill(tmpbuf, tmpl_eo1_class_ctor, classname, "", EINA_TRUE);
#endif
   eina_strbuf_replace_all(str_end, "@#ctor_func", eina_strbuf_string_get(tmpbuf));

   eina_strbuf_reset(tmpbuf);
#ifndef EO
   _template_fill(tmpbuf, "_gen_@#class_class_constructor", classname, "", EINA_TRUE);
   eina_strbuf_replace_all(str_end, "@#ctor_name", eina_strbuf_string_get(tmpbuf));
   eina_strbuf_reset(tmpbuf);
   if (eolian_class_ctor_enable_get(classname))
      _template_fill(tmpbuf, "   _@#class_class_constructor(klass);\n", classname, "", EINA_TRUE);
   eina_strbuf_replace_all(str_end, "@#user_ctor_func", eina_strbuf_string_get(tmpbuf));
#else
   if (eolian_class_ctor_enable_get(classname))
      _template_fill(tmpbuf, "_@#class_class_constructor", classname, "", EINA_TRUE);
   else
      eina_strbuf_append_printf(tmpbuf, "NULL");
   eina_strbuf_replace_all(str_end, "@#ctor_name", eina_strbuf_string_get(tmpbuf));
#endif

   eina_strbuf_reset(tmpbuf);
   if (eolian_class_dtor_enable_get(classname))
     {
#ifndef EO
        _template_fill(tmpbuf, tmpl_dtor, classname, "", EINA_TRUE);
#endif
        eina_strbuf_replace_all(str_end, "@#dtor_func", eina_strbuf_string_get(tmpbuf));
        eina_strbuf_reset(tmpbuf);
#ifndef EO
        _template_fill(tmpbuf, "_gen_@#class_class_destructor", classname, "", EINA_TRUE);
#else
        _template_fill(tmpbuf, "_@#class_class_destructor", classname, "", EINA_TRUE);
#endif
        eina_strbuf_replace_all(str_end, "@#dtor_name", eina_strbuf_string_get(tmpbuf));
     }
   else
     {
        eina_strbuf_replace_all(str_end, "@#dtor_func", "");
        eina_strbuf_replace_all(str_end, "@#dtor_name", "NULL");
     }

   eina_strbuf_reset(tmpbuf);

   // default constructor
   Eolian_Function ctor_fn = eolian_class_default_constructor_get(classname);
   if (ctor_fn)
     {
        _template_fill(str_func, tmpl_eo1base_func_desc, classname, "constructor", EINA_FALSE);
        eo1_bind_func_generate(classname, ctor_fn, EOLIAN_UNRESOLVED, str_bodyf, NULL);
     }
   // default destructor
   Eolian_Function dtor_fn = eolian_class_default_destructor_get(classname);
   if (dtor_fn)
     {
        _template_fill(str_func, tmpl_eo1base_func_desc, classname, "destructor", EINA_FALSE);
        eo1_bind_func_generate(classname, dtor_fn, EOLIAN_UNRESOLVED, str_bodyf, NULL);
     }

   //Implements - TODO one generate func def for all
   Eolian_Implement impl_desc;
   EINA_LIST_FOREACH(eolian_class_implements_list_get(classname), itr, impl_desc)
     {
        const char *funcname;
        const char *impl_class;
        Eolian_Function_Type ftype;

        eolian_implement_information_get(impl_desc, &impl_class, &funcname, &ftype);

        eina_strbuf_reset(tmpl_impl);
        _template_fill(str_func, NULL, impl_class, NULL, EINA_FALSE); /* Invoked to set the variables */
#ifndef EO
        eina_strbuf_append(tmpl_impl, tmpl_eo1_func_desc);
#else
        eina_strbuf_append(tmpl_impl, tmpl_eo_func_desc);
#endif
        eina_strbuf_replace_all(tmpl_impl, "@#EOPREFIX", current_eo_prefix_upper);
        eina_strbuf_replace_all(tmpl_impl, "@#eoprefix", current_eo_prefix_lower);

        char implname[0xFF];
        char *tp = implname;
        sprintf(implname, "%s_%s", classname, impl_class);
        eina_str_tolower(&tp);

        eina_strbuf_replace_all(tmpl_impl, "@#class", implname);
        const char *tmpl_impl_str = eina_strbuf_string_get(tmpl_impl);

        Eolian_Function in_meth = NULL;
        Eolian_Function in_prop = NULL;
        const Eina_List *itr2;
        Eolian_Function fnid;
        EINA_LIST_FOREACH(eolian_class_functions_list_get((char *)impl_class, EOLIAN_CTOR), itr2, fnid)
          if (fnid && !strcmp(eolian_function_name_get(fnid), funcname)) in_meth = fnid;
        EINA_LIST_FOREACH(eolian_class_functions_list_get((char *)impl_class, EOLIAN_METHOD), itr2, fnid)
          if (fnid && !strcmp(eolian_function_name_get(fnid), funcname)) in_meth = fnid;
        EINA_LIST_FOREACH(eolian_class_functions_list_get((char *)impl_class, EOLIAN_PROPERTY), itr2, fnid)
          if (fnid && !strcmp(eolian_function_name_get(fnid), funcname)) in_prop = fnid;

        if (!in_meth && !in_prop)
          {
             ERR ("Failed to generate implementation of %s:%s - missing form super class", impl_class, funcname);
             goto end;
          }

        /* e.g event_freeze can be a property or a method. If a type is explicit (property EOLIAN_PROP_SET/EOLIAN_PROP_GET),
         * we assume it can't be a method.
         */
        if ((in_meth && in_prop) && (ftype == EOLIAN_PROP_SET || ftype == EOLIAN_PROP_GET)) in_meth = NULL;

        if (in_meth)
          {
#ifndef EO
             _template_fill(str_func, tmpl_impl_str, impl_class, funcname, EINA_FALSE);
#else
             _template_fill(str_op, tmpl_impl_str, impl_class, funcname, EINA_FALSE);
#endif
             eo1_bind_func_generate(classname, in_meth, EOLIAN_UNRESOLVED, str_bodyf, impl_class);
             continue;
          }

        if (in_prop)
          {
             char tmpstr[0xFF];

             if ((ftype != EOLIAN_PROP_GET) && (ftype != EOLIAN_PROP_SET)) ftype = eolian_function_type_get(in_prop);

             Eina_Bool prop_read = ( ftype == EOLIAN_PROP_SET ) ? EINA_FALSE : EINA_TRUE;
             Eina_Bool prop_write = ( ftype == EOLIAN_PROP_GET ) ? EINA_FALSE : EINA_TRUE;

             if (prop_write)
               {
                  sprintf(tmpstr, "%s_set", funcname);
#ifndef EO
                  _template_fill(str_func, tmpl_impl_str, impl_class, tmpstr, EINA_FALSE);
#else
                  _template_fill(str_op, tmpl_impl_str, impl_class, tmpstr, EINA_FALSE);
#endif
                 eo1_bind_func_generate(classname, in_prop, EOLIAN_PROP_SET, str_bodyf, impl_class);
               }

             if (prop_read)
               {
                  sprintf(tmpstr, "%s_get", funcname);
#ifndef EO
                  _template_fill(str_func, tmpl_impl_str, impl_class, tmpstr, EINA_FALSE);
#else
                  _template_fill(str_op, tmpl_impl_str, impl_class, tmpstr, EINA_FALSE);
#endif
                  eo1_bind_func_generate(classname, in_prop, EOLIAN_PROP_GET, str_bodyf, impl_class);
               }
          }
        eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));
     }

   //Constructors
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, EOLIAN_CTOR), itr, fn)
     {
        const char *funcname = eolian_function_name_get(fn);
        char *desc = _source_desc_get(eolian_function_description_get(fn, "comment"));
        eo_op_desc_generate(classname, fn, EOLIAN_CTOR, desc, tmpbuf);
        eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));
        free(desc);

        eo1_eo_func_desc_generate(classname, NULL, funcname, tmpbuf);
        eina_strbuf_append(str_func, eina_strbuf_string_get(tmpbuf));

        eo1_bind_func_generate(classname, fn, EOLIAN_UNRESOLVED, str_bodyf, NULL);
     }

   //Properties
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, EOLIAN_PROPERTY), itr, fn)
     {
        const char *funcname = eolian_function_name_get(fn);
        const Eolian_Function_Type ftype = eolian_function_type_get(fn);
        char tmpstr[0xFF];

        Eina_Bool prop_read = ( ftype == EOLIAN_PROP_SET ) ? EINA_FALSE : EINA_TRUE;
        Eina_Bool prop_write = ( ftype == EOLIAN_PROP_GET ) ? EINA_FALSE : EINA_TRUE;

        if (prop_write)
          {
             char *desc = _source_desc_get(eolian_function_description_get(fn, "comment_set"));

             sprintf(tmpstr, "%s_set", funcname);
             eo_op_desc_generate(classname, fn, EOLIAN_PROP_SET, desc, tmpbuf);
             eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));
             free(desc);

             if (!eolian_function_is_virtual_pure(fn, EOLIAN_PROP_SET))
                eo1_eo_func_desc_generate(classname, NULL, tmpstr, tmpbuf);
             else
                eina_strbuf_reset(tmpbuf);
             eina_strbuf_append(str_func, eina_strbuf_string_get(tmpbuf));
          }
        if (prop_read)
          {
             char *desc = _source_desc_get(eolian_function_description_get(fn, "comment_get"));

             sprintf(tmpstr, "%s_get", funcname);
             eo_op_desc_generate(classname, fn, EOLIAN_PROP_GET, desc, tmpbuf);
             free(desc);
             eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));

             if (!eolian_function_is_virtual_pure(fn, EOLIAN_PROP_GET))
                eo1_eo_func_desc_generate(classname, NULL, tmpstr, tmpbuf);
             else
                eina_strbuf_reset(tmpbuf);
             eina_strbuf_append(str_func, eina_strbuf_string_get(tmpbuf));
          }
     }

   //Methods
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, EOLIAN_METHOD), itr, fn)
     {
        const char *funcname = eolian_function_name_get(fn);

        char *desc = _source_desc_get(eolian_function_description_get(fn, "comment"));
        eo_op_desc_generate(classname, fn, EOLIAN_METHOD, desc, tmpbuf);
        free(desc);
        eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));

        if (!eolian_function_is_virtual_pure(fn, EOLIAN_METHOD))
           eo1_eo_func_desc_generate(classname, NULL, funcname, tmpbuf);
        else
           eina_strbuf_reset(tmpbuf);
        eina_strbuf_append(str_func, eina_strbuf_string_get(tmpbuf));
     }

   Eolian_Event event;
   EINA_LIST_FOREACH(eolian_class_events_list_get(classname), itr, event)
     {
        const char *evname;

        eolian_class_event_information_get(event, &evname, NULL, NULL);
        _template_fill(tmpbuf, "@#CLASS_EVENT_@#FUNC", classname, evname, EINA_TRUE);
        eina_strbuf_replace_all(tmpbuf, ",", "_");
        eina_strbuf_append_printf(str_ev, "\n     %s,", eina_strbuf_string_get(tmpbuf));
     }

   eina_strbuf_reset(tmpbuf);
   if (eina_strbuf_length_get(str_ev))
     {
        Eina_Strbuf *events_desc = eina_strbuf_new();
        _template_fill(events_desc, tmpl_events_desc, classname, "", EINA_TRUE);
        eina_strbuf_replace_all(events_desc, "@#list_evdesc", eina_strbuf_string_get(str_ev));
        eina_strbuf_replace_all(str_end, "@#events_desc", eina_strbuf_string_get(events_desc));
        eina_strbuf_free(events_desc);
        eina_strbuf_append_printf(tmpbuf, "_%s_event_desc", lowclass);
     }
   else
     {
        eina_strbuf_append_printf(tmpbuf, "NULL");
        eina_strbuf_replace_all(str_end, "@#events_desc", "");
     }
   eina_strbuf_replace_all(str_end, "@#Events_Desc", eina_strbuf_string_get(tmpbuf));

   const char *inherit_name;
   eina_strbuf_reset(tmpbuf);
   EINA_LIST_FOREACH(eolian_class_inherits_list_get(classname), itr, inherit_name)
     {
        const char *eo_prefix = eolian_class_eo_prefix_get(inherit_name);
        if (!eo_prefix) eo_prefix = inherit_name;
        _template_fill(tmpbuf, "@#EOPREFIX_CLASS, ", eo_prefix, "", EINA_FALSE);
        eina_strbuf_replace_all(tmpbuf, "@#EOPREFIX", current_eo_prefix_upper);
     }

   if (eina_strbuf_length_get(tmpbuf) == 0) eina_strbuf_append(tmpbuf, "NULL, ");
   eina_strbuf_replace_all(str_end, "@#list_inherit", eina_strbuf_string_get(tmpbuf));

   eina_strbuf_replace_all(str_end, "@#list_func", eina_strbuf_string_get(str_func));

   eina_strbuf_reset(tmpbuf);
   if (eina_strbuf_length_get(str_op))
     {
        Eina_Strbuf *ops_desc = eina_strbuf_new();
#ifndef EO
        _template_fill(ops_desc, tmpl_eo1_ops_desc, classname, "", EINA_TRUE);
#else
        _template_fill(ops_desc, tmpl_eo_ops_desc, classname, "", EINA_TRUE);
#endif
        eina_strbuf_replace_all(ops_desc, "@#list_op", eina_strbuf_string_get(str_op));
        eina_strbuf_replace_all(str_end, "@#ops_desc", eina_strbuf_string_get(ops_desc));
        eina_strbuf_free(ops_desc);
#ifndef EO
        _template_fill(tmpbuf,
              "EO_CLASS_DESCRIPTION_OPS(&@#EOPREFIX_BASE_ID, _@#class_op_desc, @#EOPREFIX_SUB_ID_LAST)",
              classname, NULL, EINA_TRUE);
        eina_strbuf_replace_all(tmpbuf, "@#EOPREFIX", current_eo_prefix_upper);
#else
        _template_fill(tmpbuf,
              "EO_CLASS_DESCRIPTION_OPS(_@#class_op_desc)",
              classname, NULL, EINA_TRUE);
#endif
     }
   else
     {
        eina_strbuf_replace_all(str_end, "@#ops_desc", "");
#ifndef EO
        eina_strbuf_append_printf(tmpbuf, "EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0)");
#else
        eina_strbuf_append_printf(tmpbuf, "EO_CLASS_DESCRIPTION_NOOPS()");
#endif
     }

   eina_strbuf_replace_all(str_end, "@#functions_body", eina_strbuf_string_get(str_bodyf));
   eina_strbuf_replace_all(str_end, "@#eo_class_desc_ops", eina_strbuf_string_get(tmpbuf));

   const char *data_type = eolian_class_data_type_get(classname);
   if (data_type && !strcmp(data_type, "null"))
      eina_strbuf_replace_all(str_end, "@#SizeOfData", "0");
   else
     {
        Eina_Strbuf *sizeofbuf = eina_strbuf_new();
        eina_strbuf_append_printf(sizeofbuf, "sizeof(%s%s)",
              data_type?data_type:classname,
              data_type?"":"_Data");
        eina_strbuf_replace_all(str_end, "@#SizeOfData", eina_strbuf_string_get(sizeofbuf));
        eina_strbuf_free(sizeofbuf);
     }
   eina_strbuf_append(buf, eina_strbuf_string_get(str_end));

   ret = EINA_TRUE;
end:
   eina_strbuf_free(tmpbuf);
   eina_strbuf_free(str_op);
   eina_strbuf_free(str_func);
   eina_strbuf_free(str_bodyf);
   eina_strbuf_free(str_end);
   eina_strbuf_free(str_ev);
   eina_strbuf_free(tmpl_impl);

   return ret;
}

Eina_Bool
eo_source_generate(const char *classname, int eo_version EINA_UNUSED, Eina_Strbuf *buf)
{
   Eina_Bool ret = EINA_FALSE;
   const Eina_List *itr;
   Eolian_Function fn;

   Eina_Strbuf *str_bodyf = eina_strbuf_new();

   if (!eo1_source_beginning_generate(classname, buf)) goto end;

   //Properties
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, EOLIAN_PROPERTY), itr, fn)
     {
        const Eolian_Function_Type ftype = eolian_function_type_get(fn);

        Eina_Bool prop_read = ( ftype == EOLIAN_PROP_SET ) ? EINA_FALSE : EINA_TRUE;
        Eina_Bool prop_write = ( ftype == EOLIAN_PROP_GET ) ? EINA_FALSE : EINA_TRUE;

        if (prop_write)
          {
             if (!eo1_bind_func_generate(classname, fn, EOLIAN_PROP_SET, str_bodyf, NULL)) goto end;
          }
        if (prop_read)
          {
             if (!eo1_bind_func_generate(classname, fn, EOLIAN_PROP_GET, str_bodyf, NULL)) goto end;
          }
     }

   //Methods
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, EOLIAN_METHOD), itr, fn)
     {
        if (!eo1_bind_func_generate(classname, fn, EOLIAN_UNRESOLVED, str_bodyf, NULL)) goto end;
     }

   eina_strbuf_append(buf, eina_strbuf_string_get(str_bodyf));
   eina_strbuf_reset(str_bodyf);

   if (!eo1_source_end_generate(classname, buf)) goto end;

   ret = EINA_TRUE;
end:
   eina_strbuf_free(str_bodyf);
   return ret;
}

