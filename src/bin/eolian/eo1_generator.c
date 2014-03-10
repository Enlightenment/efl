#include <Eina.h>
#include <string.h>

#include "Eolian.h"
#include "eo1_generator.h"
#include "common_funcs.h"

static const char
tmpl_eo_src_begin[] = "\
\n\
EAPI Eo_Op @#EOPREFIX_BASE_ID = EO_NOOP;\n\
\n\
@#list_events\n\
\n\
";

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
tmpl_eo_src_end[] = "\
@#list_ctors_body\
\n\
static void\n\
_gen_@#class_class_constructor(Eo_Class *klass)\n\
{\n\
   const Eo_Op_Func_Description func_desc[] = {@#list_func\n\
        EO_OP_FUNC_SENTINEL\n\
   };\n\
   eo_class_funcs_set(klass, func_desc);\n\
@#ctor_func\
}\n\
\n\
@#dtor_func\
static const Eo_Op_Description _@#class_op_desc[] = {@#list_op\n\
     EO_OP_DESCRIPTION_SENTINEL\n\
};\n\
\n\
static const Eo_Event_Description *_@#class_event_desc[] = {@#list_evdesc\n\
     NULL\n\
};\n\
\n\
static const Eo_Class_Description _@#class_class_desc = {\n\
     EO_VERSION,\n\
     \"@#Class\",\n\
     @#type_class,\n\
     EO_CLASS_DESCRIPTION_OPS(&@#EOPREFIX_BASE_ID, _@#class_op_desc, @#EOPREFIX_SUB_ID_LAST),\n\
     _@#class_event_desc,\n\
     @#SizeOfData,\n\
     _gen_@#class_class_constructor,\n\
     @#dtor_name\n\
};\n\
\n\
EO_DEFINE_CLASS(@#eoprefix_class_get, &_@#class_class_desc, @#list_inheritNULL);\
";

static const char
tmpl_eo_op_desc[] = "\n     EO_OP_DESCRIPTION(@#EOPREFIX_SUB_ID_@#FUNC, \"@#desc\"),";

static const char
tmpl_eo_func_desc[] = "\n        EO_OP_FUNC(@#EOPREFIX_ID(@#EOPREFIX_SUB_ID_@#FUNC), _eo_obj_@#class_@#func),";

static const char
tmpl_eobase_func_desc[] = "\n        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_@#FUNC), _eo_obj_@#class_@#func),";

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
#define @#EOPREFIX_ID(sub_id) (@#EOPREFIX_BASE_ID + sub_id)\n\
";

static const char
tmpl_eo_subid[] = "\n   @#EOPREFIX_SUB_ID_@#FUNC,";

static const char
tmpl_eo_subid_apnd[] = "   @#EOPREFIX_SUB_ID_@#FUNC,\n";

static const char
tmpl_eo_funcdef[] = "\n\
/**\n\
 * @def @#eoprefix_@#func\n\
 *\n\
@#desc\n\
 *\n\
@#list_desc_param\
 *\n\
 */\n\
#define @#eoprefix_@#func(@#list_param) @#EOPREFIX_ID(@#EOPREFIX_SUB_ID_@#FUNC)@#list_typecheck\n\
";

static const char
tmpl_eo_pardesc[] =" * @param[%s] %s %s\n";

static const char
tmpl_eobind_body[] ="\
\n\
@#ret_type _@#class_@#func(Eo *obj, @#Datatype_Data *pd@#full_params);\n\n\
static void\n\
_eo_obj_@#class_@#func(Eo *obj, void *_pd, va_list *list@#list_unused)\n\
{\n\
@#list_vars\
   @#ret_param_@#class_@#func(obj, _pd@#list_params);\n\
}\n\
";

Eina_Bool
eo1_enum_append(const char *classname, const char *funcname, Eina_Strbuf *str)
{
   _template_fill(str, tmpl_eo_subid_apnd, classname, funcname, EINA_FALSE);
   return EINA_TRUE;
}

Eina_Bool
eo1_fundef_generate(const char *classname, Eolian_Function func, Eolian_Function_Type ftype, Eina_Strbuf *functext)
{
   const char *str_dir[] = {"in", "out", "inout"};
   const Eina_List *l;
   void *data;
   char funcname[0xFF];
   char descname[0xFF];
   char *tmpstr = malloc(0x1FF);

   char *fsuffix = "";
   if (ftype == GET) fsuffix = "_get";
   if (ftype == SET) fsuffix = "_set";

   sprintf (funcname, "%s%s", eolian_function_name_get(func), fsuffix);
   sprintf (descname, "comment%s", fsuffix);
   const char *funcdesc = eolian_function_description_get(func, descname);

   Eina_Strbuf *str_func = eina_strbuf_new();
   _template_fill(str_func, tmpl_eo_funcdef, classname, funcname, EINA_TRUE);

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
   Eina_Strbuf *str_typecheck = eina_strbuf_new();

   EINA_LIST_FOREACH(eolian_property_keys_list_get(func), l, data)
     {
        const char *pname;
        const char *ptype;
        const char *pdesc = NULL;
        eolian_parameter_information_get((Eolian_Function_Parameter)data, NULL, &ptype, &pname, &pdesc);

        eina_strbuf_append_printf(str_pardesc, tmpl_eo_pardesc, "in", pname, pdesc?pdesc:"No description supplied.");

        if (eina_strbuf_length_get(str_par)) eina_strbuf_append(str_par, ", ");
        eina_strbuf_append(str_par, pname);

        eina_strbuf_append_printf(str_typecheck, ", EO_TYPECHECK(%s, %s)", ptype, pname);
     }

   EINA_LIST_FOREACH(eolian_parameters_list_get(func), l, data)
     {
        const char *pname;
        const char *ptype;
        const char *pdesc;
        Eina_Bool add_star = EINA_FALSE;
        Eolian_Parameter_Dir pdir;
        eolian_parameter_information_get((Eolian_Function_Parameter)data, &pdir, &ptype, &pname, &pdesc);
        if (ftype == GET) {
             add_star = EINA_TRUE;
             pdir = EOLIAN_OUT_PARAM;
        }
        if (ftype == SET) pdir = EOLIAN_IN_PARAM;
        if (ftype == UNRESOLVED || ftype == METHOD_FUNC) add_star = (pdir == EOLIAN_OUT_PARAM);
        Eina_Bool had_star = !!strchr(ptype, '*');

        const char *dir_str = str_dir[(int)pdir];

        eina_strbuf_append_printf(str_pardesc, tmpl_eo_pardesc, dir_str, pname, pdesc?pdesc:"No description supplied.");

        if (eina_strbuf_length_get(str_par)) eina_strbuf_append(str_par, ", ");
        eina_strbuf_append(str_par, pname);

        eina_strbuf_append_printf(str_typecheck, ", EO_TYPECHECK(%s%s%s, %s)", ptype, had_star?"":" ", add_star?"*":"", pname);
     }

   const char* rettype = eolian_function_return_type_get(func, ftype);
   if (rettype && strcmp(rettype, "void"))
     {
        const char *ret_desc = eolian_function_return_comment_get(func, ftype);
        eina_strbuf_append_printf(str_pardesc, tmpl_eo_pardesc, "out", "ret", ret_desc);
        if (eina_strbuf_length_get(str_par)) eina_strbuf_append(str_par, ", ");
        eina_strbuf_append(str_par, "ret");
        Eina_Bool had_star = !!strchr(rettype, '*');
        eina_strbuf_append_printf(str_typecheck, ", EO_TYPECHECK(%s%s*, ret)", rettype, had_star?"":" ");
     }

   eina_strbuf_replace_all(str_func, "@#list_param", eina_strbuf_string_get(str_par));
   eina_strbuf_replace_all(str_func, "@#list_desc_param", eina_strbuf_string_get(str_pardesc));
   eina_strbuf_replace_all(str_func, "@#list_typecheck", eina_strbuf_string_get(str_typecheck));

   free(tmpstr);
   eina_strbuf_free(str_par);
   eina_strbuf_free(str_pardesc);
   eina_strbuf_free(str_typecheck);

   eina_strbuf_append(functext, eina_strbuf_string_get(str_func));
   eina_strbuf_free(str_func);

   return EINA_TRUE;
}

Eina_Bool
eo1_header_generate(const char *classname, Eina_Strbuf *buf)
{
   const Eolian_Function_Type ftype_order[] = {CONSTRUCTOR, PROPERTY_FUNC, METHOD_FUNC};
   const Eina_List *l;
   void *data;
   char *tmpstr = malloc(0x1FF);

   if (!eolian_class_exists(classname))
     {
        ERR ("Class \"%s\" not found in database", classname);
        return EINA_FALSE;
     }

   Eina_Strbuf * str_hdr = eina_strbuf_new();
   _template_fill(str_hdr, tmpl_eo_obj_header, classname, "", EINA_TRUE);

   eina_strbuf_replace_all(str_hdr, "@#EOPREFIX", current_eo_prefix_upper);
   eina_strbuf_replace_all(str_hdr, "@#eoprefix", current_eo_prefix_lower);

   Eina_Strbuf *str_subid = eina_strbuf_new();
   Eina_Strbuf *str_ev = eina_strbuf_new();
   Eina_Strbuf *tmpbuf = eina_strbuf_new();

   Eolian_Event event;
   EINA_LIST_FOREACH(eolian_class_events_list_get(classname), l, event)
     {
        const char *evname = NULL;
        const char *evdesc = NULL;
        eolian_class_event_information_get(event, &evname, &evdesc);

        if (!evdesc) evdesc = "No description";
        eina_strbuf_reset(tmpbuf);
        eina_strbuf_append(tmpbuf, evdesc);
        eina_strbuf_replace_all(tmpbuf, "\n", "\n * ");
        eina_strbuf_prepend(tmpbuf," * ");
        eina_strbuf_append_printf(str_ev, "\n/**\n%s\n */\n", eina_strbuf_string_get(tmpbuf));

        _template_fill(tmpbuf, "@#CLASS_@#FUNC", classname, evname, EINA_TRUE);
        eina_strbuf_replace_all(tmpbuf, ",", "_");
        const char* s = eina_strbuf_string_get(tmpbuf);
        eina_strbuf_append_printf(str_ev, "#define %s (&(_%s))\n", s, s);
     }

   int i;
   for (i = 0; i < 3; i++)
      EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, ftype_order[i]), l, data)
        {
           const Eolian_Function_Type ftype = eolian_function_type_get((Eolian_Function)data);
           const char *funcname = eolian_function_name_get((Eolian_Function)data);
           Eina_Bool prop_read = (ftype == PROPERTY_FUNC || ftype == GET ) ? EINA_TRUE : EINA_FALSE ;
           Eina_Bool prop_write = (ftype == PROPERTY_FUNC || ftype == SET ) ? EINA_TRUE : EINA_FALSE ;

           if (!prop_read && !prop_write)
             {
                _template_fill(str_subid, tmpl_eo_subid, classname, funcname, EINA_FALSE);
                eo1_fundef_generate(classname, (Eolian_Function)data, UNRESOLVED, str_hdr);
             }
           if (prop_write)
             {
                sprintf(tmpstr, "%s_set", funcname);
                _template_fill(str_subid, tmpl_eo_subid, classname, tmpstr, EINA_FALSE);
                eo1_fundef_generate(classname, (Eolian_Function)data, SET, str_hdr);
             }
           if (prop_read)
             {
                sprintf(tmpstr, "%s_get", funcname);
                _template_fill(str_subid, tmpl_eo_subid, classname, tmpstr, EINA_FALSE);
                eo1_fundef_generate(classname, (Eolian_Function)data, GET, str_hdr);
             }
           eina_strbuf_replace_all(str_subid, "@#EOPREFIX", current_eo_prefix_upper);
        }

   eina_strbuf_replace_all(str_hdr, "@#list_subid", eina_strbuf_string_get(str_subid));
   eina_strbuf_append(str_hdr, eina_strbuf_string_get(str_ev));

   eina_strbuf_append(buf, eina_strbuf_string_get(str_hdr));

   free(tmpstr);
   eina_strbuf_free(str_subid);
   eina_strbuf_free(str_ev);
   eina_strbuf_free(tmpbuf);
   eina_strbuf_free(str_hdr);

   return EINA_TRUE;
}

static const char*
_varg_upgr(const char *stype)
{
   if (!strcmp(stype, "Eina_Bool") ||
     !strcmp(stype, "char") ||
     !strcmp(stype, "short"))
     return "int";
   return stype;
}

Eina_Bool
eo1_bind_func_generate(const char *classname, Eolian_Function funcid, Eolian_Function_Type ftype, Eina_Strbuf *buf, const char *impl_name)
{
   const char *suffix = "";
   Eina_Bool var_as_ret = EINA_FALSE;
   const char *rettype = NULL;
   const char *retname = NULL;
   Eina_Bool ret_const = EINA_FALSE;
   Eina_Bool add_star = EINA_FALSE;

   if (eolian_function_is_virtual_pure(funcid)) return EINA_TRUE;
   Eina_Strbuf *fbody = eina_strbuf_new();
   Eina_Strbuf *va_args = eina_strbuf_new();
   Eina_Strbuf *params = eina_strbuf_new(); /* only variables names */
   Eina_Strbuf *full_params = eina_strbuf_new(); /* variables types + names */

   rettype = eolian_function_return_type_get(funcid, ftype);
   if (rettype && !strcmp(rettype, "void")) rettype = NULL;
   retname = "ret";
   if (ftype == GET)
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
                  ret_const = eolian_parameter_get_const_attribute_get(data);
               }
          }
     }
   if (ftype == SET)
     {
        suffix = "_set";
     }

   char tmpstr[0xFF];
   sprintf (tmpstr, "%s%s", eolian_function_name_get(funcid), suffix);
   char tmpstr2[0xFF];
   sprintf (tmpstr2, "%s_%s", classname, impl_name);
   _template_fill(fbody, tmpl_eobind_body, impl_name?tmpstr2:classname, tmpstr, EINA_FALSE);

   const Eina_List *l;
   void *data;

   EINA_LIST_FOREACH(eolian_property_keys_list_get(funcid), l, data)
     {
        const char *pname;
        const char *ptype;
        eolian_parameter_information_get((Eolian_Function_Parameter)data, NULL, &ptype, &pname, NULL);
        Eina_Bool is_const = eolian_parameter_get_const_attribute_get(data);
        eina_strbuf_append_printf(va_args, "   %s%s %s = va_arg(*list, %s%s);\n",
              ftype == GET && is_const?"const ":"", ptype, pname,
              ftype == GET && is_const?"const ":"", _varg_upgr(ptype));
        eina_strbuf_append_printf(params, ", %s", pname);
        eina_strbuf_append_printf(full_params, ", %s%s %s",
              ftype == GET && eolian_parameter_get_const_attribute_get(data)?"const ":"",
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
             Eina_Bool is_const = eolian_parameter_get_const_attribute_get(data);
             Eina_Bool had_star = !!strchr(ptype, '*');
             if (ftype == UNRESOLVED || ftype == METHOD_FUNC) add_star = (pdir == EOLIAN_OUT_PARAM);
             eina_strbuf_append_printf(va_args, "   %s%s%s%s%s = va_arg(*list, %s%s%s%s);\n",
                   ftype == GET && is_const?"const ":"", ptype, had_star?"":" ", add_star?"*":"", pname,
                   ftype == GET && is_const?"const ":"", add_star ? ptype : _varg_upgr(ptype), !had_star && add_star?" ":"", add_star?"*":"");
             eina_strbuf_append_printf(params, ", %s", pname);
             eina_strbuf_append_printf(full_params, ", %s%s%s%s%s",
                   ftype == GET && eolian_parameter_get_const_attribute_get(data)?"const ":"",
                   ptype, had_star?"":" ", add_star?"*":"", pname);
          }
     }

   if (rettype && strcmp(rettype, "void"))
     {
        Eina_Bool had_star = !!strchr(rettype, '*');
        eina_strbuf_append_printf(va_args, "   %s%s%s*%s = va_arg(*list, %s%s%s*);\n",
              ret_const?"const ":"",
              rettype, had_star?"":" ", retname,
              ret_const?"const ":"",
              rettype, had_star?"":" ");
        Eina_Strbuf *ret_param = eina_strbuf_new();
        eina_strbuf_append_printf(ret_param, "*%s = ", retname);
        eina_strbuf_replace_all(fbody, "@#ret_param", eina_strbuf_string_get(ret_param));
        sprintf(tmpstr, "%s%s", ret_const?"const ":"", rettype);
        eina_strbuf_replace_all(fbody, "@#ret_type", tmpstr);
        eina_strbuf_free(ret_param);
     }
   else
     {
        eina_strbuf_replace_all(fbody, "@#ret_param", "");
        eina_strbuf_replace_all(fbody, "@#ret_type", "void");
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
   eina_strbuf_replace_all(fbody, "@#list_vars", eina_strbuf_string_get(va_args));
   eina_strbuf_replace_all(fbody, "@#full_params", eina_strbuf_string_get(full_params));
   eina_strbuf_replace_all(fbody, "@#list_params", eina_strbuf_string_get(params));
   const char *data_type = eolian_class_data_type_get(classname);
   if (data_type && !strcmp(data_type, "null"))
      eina_strbuf_replace_all(fbody, "@#Datatype_Data", "void *");
   else
     {
        if (data_type) eina_strbuf_replace_all(fbody, "@#Datatype_Data", data_type);
        else eina_strbuf_replace_all(fbody, "@#Datatype", classname);
     }

   if (!data_type || !strcmp(data_type, "null"))
      eina_strbuf_replace_all(fbody, "@#Datatype", classname);
   else
      eina_strbuf_replace_all(fbody, "@#Datatype_Data", data_type);
   eina_strbuf_append(buf, eina_strbuf_string_get(fbody));

   eina_strbuf_free(va_args);
   eina_strbuf_free(full_params);
   eina_strbuf_free(params);
   eina_strbuf_free(fbody);
   return EINA_TRUE;
}

Eina_Bool
eo1_eo_func_desc_generate(const char *classname, const char *funcname, Eina_Strbuf *buf)
{
   _template_fill(buf, tmpl_eo_func_desc, classname, funcname, EINA_TRUE);
   eina_strbuf_replace_all(buf, "@#EOPREFIX", current_eo_prefix_upper);
   return EINA_TRUE;
}

Eina_Bool
eo1_eo_op_desc_generate(const char *classname, const char *funcname, Eina_Strbuf *buf)
{
   _template_fill(buf, tmpl_eo_op_desc, classname, funcname, EINA_TRUE);
   eina_strbuf_replace_all(buf, "@#EOPREFIX", current_eo_prefix_upper);

   return EINA_TRUE;
}

Eina_Bool
eo1_source_beginning_generate(const char *classname, Eina_Strbuf *buf)
{
   const Eina_List *itr;

   Eina_Strbuf *tmpbuf = eina_strbuf_new();
   Eina_Strbuf *str_begin = eina_strbuf_new();
   Eina_Strbuf *str_ev = eina_strbuf_new();

   _template_fill(str_begin, tmpl_eo_src_begin, classname, "", EINA_TRUE);
   eina_strbuf_replace_all(str_begin, "@#EOPREFIX", current_eo_prefix_upper);

   Eolian_Event event;
   EINA_LIST_FOREACH(eolian_class_events_list_get(classname), itr, event)
     {
        const char *evname;
        const char *evdesc;
        char *evdesc_line1;

        eolian_class_event_information_get(event, &evname, &evdesc);
        evdesc_line1 = _source_desc_get(evdesc);
        _template_fill(str_ev, "@#CLASS_@#FUNC", classname, evname, EINA_TRUE);
        eina_strbuf_replace_all(str_ev, ",", "_");

        eina_strbuf_append_printf(tmpbuf,
                                  "EAPI const Eo_Event_Description _%s = EO_EVENT_DESCRIPTION(\"%s\", \"%s\");\n",
                                  eina_strbuf_string_get(str_ev), evname, evdesc_line1);
        free(evdesc_line1);
     }

   eina_strbuf_replace_all(str_begin, "@#list_events", eina_strbuf_string_get(tmpbuf));
   eina_strbuf_append(buf, eina_strbuf_string_get(str_begin));

   eina_strbuf_free(str_ev);
   eina_strbuf_free(tmpbuf);
   eina_strbuf_free(str_begin);
   return EINA_TRUE;
}

Eina_Bool
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

   _template_fill(str_end, tmpl_eo_src_end, classname, "", EINA_TRUE);

   eina_strbuf_replace_all(str_end, "@#type_class", str_classtype);
   eina_strbuf_replace_all(str_end, "@#EOPREFIX", current_eo_prefix_upper);
   eina_strbuf_replace_all(str_end, "@#eoprefix", current_eo_prefix_lower);

   if (eolian_class_dtor_enable_get(classname))
     {
        _template_fill(tmpbuf, tmpl_dtor, classname, "", EINA_TRUE);
        eina_strbuf_replace_all(str_end, "@#dtor_func", eina_strbuf_string_get(tmpbuf));
        _template_fill(tmpbuf, "_gen_@#class_class_destructor", classname, "", EINA_TRUE);
        eina_strbuf_replace_all(str_end, "@#dtor_name", eina_strbuf_string_get(tmpbuf));
     }
   else
     {
        eina_strbuf_replace_all(str_end, "@#dtor_func", "");
        eina_strbuf_replace_all(str_end, "@#dtor_name", "NULL");
     }

   eina_strbuf_reset(tmpbuf);
   if (eolian_class_ctor_enable_get(classname))
      _template_fill(tmpbuf, "   _@#class_class_constructor(klass);\n", classname, "", EINA_TRUE);
   eina_strbuf_replace_all(str_end, "@#ctor_func", eina_strbuf_string_get(tmpbuf));

   // default constructor
   Eolian_Function ctor_fn = eolian_class_default_constructor_get(classname);
   if (ctor_fn)
     {
        _template_fill(str_func, tmpl_eobase_func_desc, classname, "constructor", EINA_FALSE);
        eo1_bind_func_generate(classname, ctor_fn, UNRESOLVED, str_bodyf, NULL);
     }
   // default destructor
   Eolian_Function dtor_fn = eolian_class_default_destructor_get(classname);
   if (dtor_fn)
     {
        _template_fill(str_func, tmpl_eobase_func_desc, classname, "destructor", EINA_FALSE);
        eo1_bind_func_generate(classname, dtor_fn, UNRESOLVED, str_bodyf, NULL);
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
        eina_strbuf_append(tmpl_impl, tmpl_eo_func_desc);
        eina_strbuf_replace_all(tmpl_impl, "@#EOPREFIX", current_eo_prefix_upper);

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
        EINA_LIST_FOREACH(eolian_class_functions_list_get((char *)impl_class, CONSTRUCTOR), itr2, fnid)
          if (fnid && !strcmp(eolian_function_name_get(fnid), funcname)) in_meth = fnid;
        EINA_LIST_FOREACH(eolian_class_functions_list_get((char *)impl_class, METHOD_FUNC), itr2, fnid)
          if (fnid && !strcmp(eolian_function_name_get(fnid), funcname)) in_meth = fnid;
        EINA_LIST_FOREACH(eolian_class_functions_list_get((char *)impl_class, PROPERTY_FUNC), itr2, fnid)
          if (fnid && !strcmp(eolian_function_name_get(fnid), funcname)) in_prop = fnid;

        if (!in_meth && !in_prop)
          {
             ERR ("Failed to generate implementation of %s:%s - missing form super class", impl_class, funcname);
             goto end;
          }

        if (in_meth)
          {
             _template_fill(str_func, tmpl_impl_str, impl_class, funcname, EINA_FALSE);
             eo1_bind_func_generate(classname, in_meth, UNRESOLVED, str_bodyf, impl_class);
          }

        if (in_prop)
          {
             char tmpstr[0xFF];

             if ((ftype != GET) && (ftype != SET)) ftype = eolian_function_type_get(in_prop);

             Eina_Bool prop_read = ( ftype == SET ) ? EINA_FALSE : EINA_TRUE;
             Eina_Bool prop_write = ( ftype == GET ) ? EINA_FALSE : EINA_TRUE;

             if (prop_write)
               {
                  sprintf(tmpstr, "%s_set", funcname);
                  _template_fill(str_func, tmpl_impl_str, impl_class, tmpstr, EINA_FALSE);
                 eo1_bind_func_generate(classname, in_prop, SET, str_bodyf, impl_class);
               }

             if (prop_read)
               {
                  sprintf(tmpstr, "%s_get", funcname);
                  _template_fill(str_func, tmpl_impl_str, impl_class, tmpstr, EINA_FALSE);
                  eo1_bind_func_generate(classname, in_prop, GET, str_bodyf, impl_class);
               }
          }
     }

   //Constructors
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, CONSTRUCTOR), itr, fn)
     {
        const char *funcname = eolian_function_name_get(fn);
        char *desc = _source_desc_get(eolian_function_description_get(fn, "comment"));

        _template_fill(tmpbuf, tmpl_eo_op_desc, classname, funcname, EINA_TRUE);
        eina_strbuf_replace_all(tmpbuf, "@#EOPREFIX", current_eo_prefix_upper);

        eina_strbuf_replace_all(tmpbuf, "@#desc", desc);
        free(desc);

        eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));

        _template_fill(str_func, tmpl_eo_func_desc, classname, funcname, EINA_FALSE);
        eina_strbuf_replace_all(str_func, "@#EOPREFIX", current_eo_prefix_upper);
        eo1_bind_func_generate(classname, fn, UNRESOLVED, str_bodyf, NULL);
     }

   //Properties
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, PROPERTY_FUNC), itr, fn)
     {
        const char *funcname = eolian_function_name_get(fn);
        const Eolian_Function_Type ftype = eolian_function_type_get(fn);
        char tmpstr[0xFF];

        Eina_Bool prop_read = ( ftype == SET ) ? EINA_FALSE : EINA_TRUE;
        Eina_Bool prop_write = ( ftype == GET ) ? EINA_FALSE : EINA_TRUE;

        if (prop_write)
          {
             char *desc = _source_desc_get(eolian_function_description_get(fn, "comment_set"));

             sprintf(tmpstr, "%s_set", funcname);
             eo1_eo_op_desc_generate(classname, tmpstr, tmpbuf);
             eina_strbuf_replace_all(tmpbuf, "@#desc", desc);
             eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));
             free(desc);

             if (!eolian_function_is_virtual_pure(fn))
                eo1_eo_func_desc_generate(classname, tmpstr, tmpbuf);
             else
                eina_strbuf_reset(tmpbuf);
             eina_strbuf_append(str_func, eina_strbuf_string_get(tmpbuf));
          }
        if (prop_read)
          {
             char *desc = _source_desc_get(eolian_function_description_get(fn, "comment_get"));

             sprintf(tmpstr, "%s_get", funcname);
             eo1_eo_op_desc_generate(classname, tmpstr, tmpbuf);
             eina_strbuf_replace_all(tmpbuf, "@#desc", desc);
             free(desc);
             eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));

             if (!eolian_function_is_virtual_pure(fn))
                eo1_eo_func_desc_generate(classname, tmpstr, tmpbuf);
             else
                eina_strbuf_reset(tmpbuf);
             eina_strbuf_append(str_func, eina_strbuf_string_get(tmpbuf));
          }
     }

   //Methods
   EINA_LIST_FOREACH(eolian_class_functions_list_get(classname, METHOD_FUNC), itr, fn)
     {
        const char *funcname = eolian_function_name_get(fn);
        char *desc = _source_desc_get(eolian_function_description_get(fn, "comment"));

        eo1_eo_op_desc_generate(classname, funcname, tmpbuf);
        eina_strbuf_replace_all(tmpbuf, "@#desc", desc);
        free(desc);
        eina_strbuf_append(str_op, eina_strbuf_string_get(tmpbuf));

        if (!eolian_function_is_virtual_pure(fn))
           eo1_eo_func_desc_generate(classname, funcname, tmpbuf);
        else
           eina_strbuf_reset(tmpbuf);
        eina_strbuf_append(str_func, eina_strbuf_string_get(tmpbuf));
     }

   Eolian_Event event;
   EINA_LIST_FOREACH(eolian_class_events_list_get(classname), itr, event)
     {
        const char *evname;

        eolian_class_event_information_get(event, &evname, NULL);
        _template_fill(tmpbuf, "@#CLASS_@#FUNC", classname, evname, EINA_TRUE);
        eina_strbuf_replace_all(tmpbuf, ",", "_");
        eina_strbuf_append_printf(str_ev, "\n     %s,", eina_strbuf_string_get(tmpbuf));
     }
   eina_strbuf_replace_all(str_end, "@#list_evdesc", eina_strbuf_string_get(str_ev));

   const char *inherit_name;
   eina_strbuf_reset(tmpbuf);
   EINA_LIST_FOREACH(eolian_class_inherits_list_get(classname), itr, inherit_name)
     {
        _template_fill(tmpbuf, "@#EOPREFIX_CLASS, ", inherit_name, "", EINA_FALSE);
        eina_strbuf_replace_all(tmpbuf, "@#EOPREFIX", current_eo_prefix_upper);
     }

   if (eina_strbuf_length_get(tmpbuf) == 0) eina_strbuf_append(tmpbuf, "NULL, ");
   eina_strbuf_replace_all(str_end, "@#list_inherit", eina_strbuf_string_get(tmpbuf));

   eina_strbuf_replace_all(str_end, "@#list_func", eina_strbuf_string_get(str_func));
   eina_strbuf_replace_all(str_end, "@#list_op", eina_strbuf_string_get(str_op));
   eina_strbuf_replace_all(str_end, "@#list_ctors_body", eina_strbuf_string_get(str_bodyf));

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

