#include "main.h"
#include "docs.h"

/* Used to store the function names that will have to be appended
 * with __eolian during C generation. Needed when params have to
 * be initialized and for future features.
 */
static Eina_Hash *_funcs_params_init_get = NULL;
static Eina_Hash *_funcs_params_init_set = NULL;

static const char *
_get_add_star(Eolian_Function_Type ftype, Eolian_Parameter_Direction pdir)
{
   if (ftype == EOLIAN_PROP_GET)
     return "*";
   if ((pdir == EOLIAN_PARAMETER_OUT) || (pdir == EOLIAN_PARAMETER_INOUT))
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
_gen_func_pointer_param(const char *name, Eina_Stringshare *c_type,
                        const Eolian_Typedecl *typedecl EINA_UNUSED,
                        Eina_Strbuf *params, Eina_Strbuf *params_full,
                        Eina_Strbuf *params_full_imp,
                        Eina_Bool is_empty EINA_UNUSED)
{
   Eina_Strbuf *dataname = eina_strbuf_new();
   Eina_Strbuf *freename = eina_strbuf_new();

   eina_strbuf_append_printf(dataname, "%s_data", name);
   eina_strbuf_append_printf(freename, "%s_free_cb", name);

   if (eina_strbuf_length_get(params))
     eina_strbuf_append(params, ", ");

   eina_strbuf_append_printf(params, "%s, %s, %s",
                             eina_strbuf_string_get(dataname),
                             name,
                             eina_strbuf_string_get(freename));

   eina_strbuf_append_printf(params_full_imp, ", void *%s, %s %s, Eina_Free_Cb %s",
                             eina_strbuf_string_get(dataname),
                             c_type, name,
                             eina_strbuf_string_get(freename));
   eina_strbuf_append_printf(params_full, ", void *%s, %s %s, Eina_Free_Cb %s",
                             eina_strbuf_string_get(dataname),
                             c_type, name,
                             eina_strbuf_string_get(freename));

   eina_strbuf_free(dataname);
   eina_strbuf_free(freename);
}

static void
_append_defval(Eina_Strbuf *buf, const Eolian_Expression *exp, const Eolian_Type *tp, const char *ctp)
{
   if (exp)
     {
        Eolian_Value val = eolian_expression_eval(exp, EOLIAN_MASK_ALL);
        Eina_Stringshare *lit = eolian_expression_value_to_literal(&val);
        if (lit)
          {
             eina_strbuf_append(buf, lit);
             Eina_Stringshare *exps = eolian_expression_serialize(exp);
             if (exps && strcmp(lit, exps))
               eina_strbuf_append_printf(buf, " /* %s */", exps);
             eina_stringshare_del(exps);
             eina_stringshare_del(lit);
             return;
          }
        else WRN("evaluation of default value failed");
     }
   /* default value or fallback */
   const Eolian_Type *btp = eolian_type_aliased_base_get(tp);
   if (eolian_type_is_ptr(btp) || strchr(ctp, '*'))
     {
        eina_strbuf_append(buf, "NULL");
        return;
     }
   const Eolian_Typedecl *tdcl = eolian_type_typedecl_get(btp);
   if (tdcl && (eolian_typedecl_type_get(tdcl) == EOLIAN_TYPEDECL_STRUCT))
     {
        char *sn = eo_gen_c_full_name_get(eolian_typedecl_name_get(tdcl));
        if (eina_streq(sn, "Eina_Rect"))
          eina_strbuf_append(buf, "(EINA_RECT_EMPTY())");
        else
          eina_strbuf_append_printf(buf, "((%s){0})", sn);
        free(sn);
        return;
     }
   /* enums and remaining regulars... 0 should do */
   eina_strbuf_append(buf, "0");
}

static const char *
_free_func_get(const Eolian_Type *type)
{
   const Eolian_Type *ab = eolian_type_aliased_base_get(type);
   switch (eolian_type_builtin_type_get(ab))
     {
      /* simple types */
      case EOLIAN_TYPE_BUILTIN_MSTRING:
        return "free";
      case EOLIAN_TYPE_BUILTIN_STRINGSHARE:
        return "eina_stringshare_del";
      case EOLIAN_TYPE_BUILTIN_ANY_VALUE:
        return "eina_value_flush";
      case EOLIAN_TYPE_BUILTIN_ANY_VALUE_REF:
        return "eina_value_free";
      case EOLIAN_TYPE_BUILTIN_STRBUF:
        return "eina_strbuf_free";
      case EOLIAN_TYPE_BUILTIN_BINBUF:
        return "eina_binbuf_free";
      /* complex types */
      case EOLIAN_TYPE_BUILTIN_ACCESSOR:
        return "eina_accessor_free";
      case EOLIAN_TYPE_BUILTIN_ARRAY:
        return "eina_array_free";
      case EOLIAN_TYPE_BUILTIN_FUTURE:
        return "(void)";
      case EOLIAN_TYPE_BUILTIN_ITERATOR:
        return "eina_iterator_free";
      case EOLIAN_TYPE_BUILTIN_HASH:
        return "eina_hash_free";
      case EOLIAN_TYPE_BUILTIN_LIST:
        return "eina_list_free";
      /* class and user types */
      case EOLIAN_TYPE_BUILTIN_INVALID:
        if (eolian_type_type_get(ab) == EOLIAN_TYPE_CLASS)
          return "efl_del";
        else
          return eolian_typedecl_free_func_get(eolian_type_typedecl_get(ab));
      /* no free func */
      default:
        return NULL;
     }
}

static void
_generate_normal_free(Eina_Strbuf **buf, const Eolian_Type *type, const Eina_Strbuf *parameter, const char *additional_intention)
{
   const char *free_func = _free_func_get(type);
   if (!free_func)
     {
        printf("No free type %s\n", eolian_type_short_name_get(type));
        return;
     }

   if (eolian_type_builtin_type_get(type) == EOLIAN_TYPE_BUILTIN_HASH)
     {
        eina_strbuf_append_printf(*buf,"   eina_hash_free_cb_set(");
        eina_strbuf_append_buffer(*buf, parameter);
        eina_strbuf_append(*buf, ",NULL);\n");
     }

   eina_strbuf_append_printf(*buf,"   %s%s(", additional_intention, free_func);
   eina_strbuf_append_buffer(*buf, parameter);
   eina_strbuf_append(*buf, ");\n");
}

static void
_generate_loop_content(Eina_Strbuf **buf, const Eolian_Type *inner_type, const Eina_Strbuf *iter_param)
{
   eina_strbuf_append(*buf, "     {\n");
   _generate_normal_free(buf, inner_type, iter_param, "     ");
   eina_strbuf_append(*buf, "     }\n");
}

static void
_generate_iterative_free(Eina_Strbuf **buf, const Eolian_Type *type, const Eolian_Type *inner_type, Eolian_Function_Parameter *parameter, Eina_Strbuf *param)
{
   Eina_Strbuf *iterator_header, *iter_param;

   iterator_header = eina_strbuf_new();
   iter_param = eina_strbuf_new();

   Eolian_Type_Builtin_Type t = eolian_type_builtin_type_get(type);

   eina_strbuf_append_printf(iter_param, "%s_iter", eolian_parameter_name_get(parameter));

   //generate the field definition
   eina_strbuf_append_printf(*buf, "   %s", eolian_type_c_type_get(inner_type));
   eina_strbuf_append_buffer(*buf, iter_param);
   eina_strbuf_append(*buf, ";\n");


   if (t == EOLIAN_TYPE_BUILTIN_LIST)
     {
        eina_strbuf_append_printf(*buf, "   EINA_LIST_FREE(");
        eina_strbuf_append_buffer(*buf, param);
        eina_strbuf_append_char(*buf, ',');
        eina_strbuf_append_buffer(*buf, iter_param);
        eina_strbuf_append(*buf, ")\n");
        _generate_loop_content(buf, inner_type, iter_param);
     }
   else if (t == EOLIAN_TYPE_BUILTIN_ITERATOR)
     {
        eina_strbuf_append_printf(*buf, "   EINA_ITERATOR_FOREACH(");
        eina_strbuf_append_buffer(*buf, param);
        eina_strbuf_append_char(*buf, ',');
        eina_strbuf_append_buffer(*buf, iter_param);
        eina_strbuf_append(*buf, ")\n");
        _generate_loop_content(buf, inner_type, iter_param);
     }
   else if (t == EOLIAN_TYPE_BUILTIN_ACCESSOR)
     {
        eina_strbuf_append_printf(*buf, "   unsigned int %s_i = 0;\n", eolian_parameter_name_get(parameter));
        eina_strbuf_append_printf(*buf, "   EINA_ACCESSOR_FOREACH(");
        eina_strbuf_append_buffer(*buf, param);
        eina_strbuf_append_printf(*buf, ",%s_i,", eolian_parameter_name_get(parameter));
        eina_strbuf_append_buffer(*buf, iter_param);
        eina_strbuf_append(*buf, ")\n");
        _generate_loop_content(buf, inner_type, iter_param);
     }
   else if (t == EOLIAN_TYPE_BUILTIN_HASH)
     {
        eina_strbuf_append_printf(*buf,"   eina_hash_free_cb_set(");
        eina_strbuf_append_buffer(*buf, param);
        eina_strbuf_append_printf(*buf, ",%s);\n",_free_func_get(inner_type));
        eina_strbuf_append_printf(*buf,"   eina_hash_free(");
        eina_strbuf_append_buffer(*buf, param);
        eina_strbuf_append(*buf, ");\n");
     }
   else if (t == EOLIAN_TYPE_BUILTIN_ARRAY)
     {
        eina_strbuf_append_printf(*buf, "   while((");
        eina_strbuf_append_buffer(*buf, iter_param);
        eina_strbuf_append_printf(*buf, " = eina_array_pop(");
        eina_strbuf_append_buffer(*buf, param);
        eina_strbuf_append_printf(*buf, ")))\n");
        _generate_loop_content(buf, inner_type, iter_param);
        eina_strbuf_append_printf(*buf, "   eina_array_free(");
        eina_strbuf_append_buffer(*buf, param);
        eina_strbuf_append_printf(*buf, ");\n");
      }
   else
     {
        printf("Error, container unknown?! %d\n", (int)t);
     }

   eina_strbuf_free(iterator_header);
   eina_strbuf_free(iter_param);
}

static int
_gen_function_param_fallback(Eina_Iterator *itr, Eina_Strbuf *fallback_free_ownership, Eina_Strbuf *param_call)
{
   Eolian_Function_Parameter *pr;
   int owners = 0;

   EINA_ITERATOR_FOREACH(itr, pr)
     {
        const Eolian_Type *type, *inner_type;

        type = eolian_parameter_type_get(pr);
        inner_type = eolian_type_base_type_get(type);

        //check if they should be freed or just ignored
        if (!eolian_parameter_is_move(pr) || eolian_parameter_direction_get(pr) == EOLIAN_PARAMETER_OUT)
          {
             eina_strbuf_append_printf(fallback_free_ownership, "   (void)%s;\n", eolian_parameter_name_get(pr));
             continue;
          }

        owners ++;

        eina_strbuf_reset(param_call);

        if (eolian_parameter_direction_get(pr) == EOLIAN_PARAMETER_INOUT)
          eina_strbuf_append_char(param_call, '*');
        eina_strbuf_append(param_call, eolian_parameter_name_get(pr));

        //check if we might want to free or handle the children
        if (!inner_type || !eolian_type_is_move(inner_type))
          {
             _generate_normal_free(&fallback_free_ownership, type, param_call, "");
          }
        else if (inner_type && eolian_type_is_move(inner_type))
          {
             _generate_iterative_free(&fallback_free_ownership, type, inner_type, pr, param_call);
          }
      }
   eina_iterator_free(itr);

   return owners;
}

static const char *
_get_reflect_initf(const Eolian_Type *abtp)
{
   Eolian_Type_Builtin_Type btp = eolian_type_builtin_type_get(abtp);
   const char *initf = NULL;
   switch (btp)
     {
      case EOLIAN_TYPE_BUILTIN_BYTE:
      case EOLIAN_TYPE_BUILTIN_CHAR:
        initf = "char"; break;
      case EOLIAN_TYPE_BUILTIN_UBYTE:
        initf = "uchar"; break;
      case EOLIAN_TYPE_BUILTIN_SHORT:
      case EOLIAN_TYPE_BUILTIN_USHORT:
      case EOLIAN_TYPE_BUILTIN_INT:
      case EOLIAN_TYPE_BUILTIN_UINT:
      case EOLIAN_TYPE_BUILTIN_LONG:
      case EOLIAN_TYPE_BUILTIN_ULONG:
      case EOLIAN_TYPE_BUILTIN_INT64:
      case EOLIAN_TYPE_BUILTIN_UINT64:
      case EOLIAN_TYPE_BUILTIN_TIME:
      case EOLIAN_TYPE_BUILTIN_FLOAT:
      case EOLIAN_TYPE_BUILTIN_DOUBLE:
      case EOLIAN_TYPE_BUILTIN_BOOL:
      case EOLIAN_TYPE_BUILTIN_STRING:
      case EOLIAN_TYPE_BUILTIN_STRINGSHARE:
        initf = eolian_type_name_get(abtp); break;
      default:
        break;
     }
   return initf;
}

static void
_gen_reflect_get(Eina_Strbuf *buf, const char *cnamel,
                 const Eolian_Function_Parameter *pr,
                 const Eolian_Function *fid, Eina_Hash *refh)
{
   const Eolian_Type *valt = eolian_parameter_type_get(pr);
   if (eolian_type_is_ptr(valt))
     return;

   const Eolian_Type *abtp = eolian_type_aliased_base_get(valt);
   const char *initf = _get_reflect_initf(abtp);
   if (!initf)
     return;

   Eolian_Function_Type et = (Eolian_Function_Type)eina_hash_find(refh, &fid);
   if (et == EOLIAN_PROP_SET)
     eina_hash_set(refh, &fid, (void *)EOLIAN_PROPERTY);
   else
     eina_hash_set(refh, &fid, (void *)EOLIAN_PROP_GET);

   eina_strbuf_append(buf, "\nstatic Eina_Value\n");
   eina_strbuf_append_printf(buf, "__eolian_%s_%s_get_reflect(const Eo *obj)\n",
     cnamel, eolian_function_name_get(fid));
   eina_strbuf_append(buf, "{\n");

   Eina_Stringshare *ct = eolian_parameter_c_type_get(pr, EINA_TRUE);
   const char *starsp = (ct[strlen(ct) - 1] != '*') ? " " : "";

   Eina_Stringshare *fcn = eolian_function_full_c_name_get(fid, EOLIAN_PROP_GET);
   eina_strbuf_append_printf(buf, "   %s%sval = %s(obj);\n", ct, starsp, fcn);
   eina_stringshare_del(fcn);
   eina_stringshare_del(ct);

   eina_strbuf_append_printf(buf, "   return eina_value_%s_init(val);\n", initf);
   eina_strbuf_append(buf, "}\n\n");
}

static void
_gen_reflect_set(Eina_Strbuf *buf, const char *cnamel,
                 const Eolian_Function_Parameter *pr,
                 const Eolian_Function *fid, Eina_Hash *refh)
{
   const Eolian_Type *valt = eolian_parameter_type_get(pr);
   if (eolian_type_is_ptr(valt))
     return;

   const Eolian_Type *abtp = eolian_type_aliased_base_get(valt);
   const char *initf = _get_reflect_initf(abtp);
   if (!initf)
     return;

   Eolian_Function_Type et = (Eolian_Function_Type)eina_hash_find(refh, &fid);
   if (et == EOLIAN_PROP_GET)
     eina_hash_set(refh, &fid, (void *)EOLIAN_PROPERTY);
   else
     eina_hash_set(refh, &fid, (void *)EOLIAN_PROP_SET);

   eina_strbuf_append(buf, "\nstatic Eina_Error\n");
   eina_strbuf_append_printf(buf, "__eolian_%s_%s_set_reflect(Eo *obj, Eina_Value val)\n",
     cnamel, eolian_function_name_get(fid));
   eina_strbuf_append(buf, "{\n");
   eina_strbuf_append(buf, "   Eina_Error r = 0;");

   Eina_Stringshare *ct = eolian_parameter_c_type_get(pr, EINA_FALSE);
   const char *starsp = (ct[strlen(ct) - 1] != '*') ? " " : "";
   eina_strbuf_append_printf(buf, "   %s%scval;\n", ct, starsp);
   eina_stringshare_del(ct);

   eina_strbuf_append_printf(buf, "   if (!eina_value_%s_convert(&val, &cval))\n", initf);
   eina_strbuf_append(buf, "      {\n");
   eina_strbuf_append(buf, "         r = EINA_ERROR_VALUE_FAILED;\n");
   eina_strbuf_append(buf, "         goto end;\n");
   eina_strbuf_append(buf, "      }\n");

   Eina_Stringshare *fcn = eolian_function_full_c_name_get(fid, EOLIAN_PROP_SET);
   eina_strbuf_append_printf(buf, "   %s(obj, cval);\n", fcn);
   eina_stringshare_del(fcn);

   eina_strbuf_append(buf, " end:\n");
   eina_strbuf_append(buf, "   eina_value_flush(&val);\n");
   eina_strbuf_append(buf, "   return r;\n");

   eina_strbuf_append(buf, "}\n\n");
}

static void
_emit_class_function(Eina_Strbuf *buf, const Eolian_Function *fid, const char *rtpn, Eina_Strbuf *params_full,
                     const char *ocnamel, const char *func_suffix, Eina_Strbuf *params, const char *function_name)
{
   eina_strbuf_append(buf, "EOAPI ");
   eina_strbuf_append(buf, rtpn);
   eina_strbuf_append(buf, " ");
   eina_strbuf_append(buf, function_name);
   eina_strbuf_append(buf, "(");
   if (eina_strbuf_length_get(params_full) == 0)
     eina_strbuf_append(buf, "void");
   else
     eina_strbuf_append_buffer(buf, params_full);
   eina_strbuf_append(buf, ")\n");
   eina_strbuf_append(buf, "{\n");
   eina_strbuf_append_printf(buf, "   %s();\n", eolian_class_c_get_function_name_get(eolian_function_class_get(fid)));
   if (strcmp(rtpn, "void"))
     eina_strbuf_append(buf, "   return ");
   else
     eina_strbuf_append(buf, "   ");
   eina_strbuf_append_printf(buf, "_%s", ocnamel);
   eina_strbuf_append_char(buf, '_');
   eina_strbuf_append(buf, eolian_function_name_get(fid));
   eina_strbuf_append(buf, func_suffix);
   eina_strbuf_append(buf, "(");
   eina_strbuf_append_buffer(buf, params);
   eina_strbuf_append(buf, ");\n");
   eina_strbuf_append(buf, "}\n");
}

static void
_gen_func(const Eolian_Class *cl, const Eolian_Function *fid,
          Eolian_Function_Type ftype, Eina_Strbuf *buf,
          const Eolian_Implement *impl, Eina_Hash *refh)
{
   Eina_Bool is_empty = eolian_implement_is_empty(impl, ftype);
   Eina_Bool is_auto = eolian_implement_is_auto(impl, ftype);

   if ((ftype != EOLIAN_PROP_GET) && (ftype != EOLIAN_PROP_SET))
     ftype = eolian_function_type_get(fid);

   Eina_Bool is_prop = (ftype == EOLIAN_PROP_GET || ftype == EOLIAN_PROP_SET);
   Eina_Bool var_as_ret = EINA_FALSE;
   /* assume we're not generating reflection api by default */
   const Eolian_Function_Parameter *reflect_param = NULL;

   const Eolian_Expression *def_ret = NULL;
   const Eolian_Type *rtp = eolian_function_return_type_get(fid, ftype);
   Eina_Stringshare *rtpn = NULL;
   if (rtp)
     {
        is_auto = EINA_FALSE; /* can't do auto if func returns */
        def_ret = eolian_function_return_default_value_get(fid, ftype);
        rtpn = eolian_function_return_c_type_get(fid, ftype);
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
                  /* reflect only when returning 1 val */
                  if (!eolian_parameter_is_by_ref(pr))
                    reflect_param = pr;
                  var_as_ret = EINA_TRUE;
                  def_ret = eolian_parameter_default_value_get(pr);
                  rtpn = eolian_parameter_c_type_get(pr, EINA_TRUE);
               }
             eina_iterator_free(itr);
          }
     }
   else if (ftype == EOLIAN_PROP_SET)
     {
        func_suffix = "_set";
        Eina_Iterator *itr = eolian_property_values_get(fid, ftype);
        void *d1, *d2;
        /* reflect with 1 value */
        if (eina_iterator_next(itr, &d1) && !eina_iterator_next(itr, &d2))
          {
             Eolian_Function_Parameter *pr = d1;
             if (!eolian_parameter_is_by_ref(pr))
               reflect_param = pr;
          }
        eina_iterator_free(itr);
     }

   Eina_Strbuf *params = eina_strbuf_new(); /* par1, par2, par3, ... */
   Eina_Strbuf *params_full = eina_strbuf_new(); /* T par1, U par2, ... for decl */
   Eina_Strbuf *params_full_imp = eina_strbuf_new(); /* as above, for impl */
   Eina_Strbuf *params_init = eina_strbuf_new(); /* default value inits */
   Eina_Strbuf *fallback_free_ownership = eina_strbuf_new(); /* list of function calls that are freeing the owned parameters, or doing nothing on the normal parameters, NULL if there is nothing owned*/

   /* property keys */
   {
      Eina_Iterator *itr = eolian_property_keys_get(fid, ftype);
      if (itr) /* has keys: no reflection */
        reflect_param = NULL;
      Eolian_Function_Parameter *pr;
      EINA_ITERATOR_FOREACH(itr, pr)
        {
           const char *prn = eolian_parameter_name_get(pr);
           Eina_Stringshare *ptn = eolian_parameter_c_type_get(pr, EINA_FALSE);

           if (eina_strbuf_length_get(params))
             eina_strbuf_append(params, ", ");
           eina_strbuf_append(params, prn);

           if (eina_strbuf_length_get(params_full) || !eolian_function_is_static(fid))
             {
                eina_strbuf_append(params_full, ", ");
                eina_strbuf_append(params_full_imp, ", ");
             }
           eina_strbuf_append_printf(params_full, "%s", ptn);
           eina_strbuf_append_printf(params_full_imp, "%s", ptn);

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

   /* check if we have owning data that we would have to free in a error case */
   if (ftype == EOLIAN_PROP_GET)
     {
        eina_strbuf_free(fallback_free_ownership);
        fallback_free_ownership = NULL;
     }
   else
     {
          Eina_Iterator *itr;
          int owners = 0;
          Eina_Strbuf *param_call;

          param_call = eina_strbuf_new();

          if (is_prop)
            {
               itr = eolian_property_values_get(fid, ftype);
               owners += _gen_function_param_fallback(itr, fallback_free_ownership, param_call);
               itr = eolian_property_keys_get(fid, ftype);
               owners += _gen_function_param_fallback(itr, fallback_free_ownership, param_call);
            }
          else
            {
               itr = eolian_function_parameters_get(fid);
               owners += _gen_function_param_fallback(itr, fallback_free_ownership, param_call);
            }

          if (owners == 0)
            {
               eina_strbuf_free(fallback_free_ownership);
               fallback_free_ownership = NULL;
            }

        eina_strbuf_free(param_call);
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
             Eolian_Parameter_Direction pd = eolian_parameter_direction_get(pr);
             const Eolian_Expression *dfv = eolian_parameter_default_value_get(pr);
             const char *prn = eolian_parameter_name_get(pr);
             const Eolian_Type *pt = eolian_parameter_type_get(pr);
             Eina_Stringshare *ptn = eolian_parameter_c_type_get(pr, EINA_FALSE);
             const Eolian_Typedecl *ptd = eolian_type_typedecl_get(pt);

             Eina_Bool had_star = ptn[strlen(ptn) - 1] == '*';
             const char *add_star = _get_add_star(ftype, pd);

             if (ptd && eolian_typedecl_type_get(ptd) == EOLIAN_TYPEDECL_FUNCTION_POINTER)
               {
                  _gen_func_pointer_param(prn, ptn, ptd, params, params_full, params_full_imp, is_empty);
                  eina_stringshare_del(ptn);
                  continue;
               }

             if (eina_strbuf_length_get(params))
               eina_strbuf_append(params, ", ");

             if (eina_strbuf_length_get(params_full_imp) || !eolian_function_is_static(fid))
               eina_strbuf_append(params_full_imp, ", ");
             eina_strbuf_append(params_full_imp, ptn);
             if (!had_star)
               eina_strbuf_append_char(params_full_imp, ' ');
             eina_strbuf_append(params_full_imp, add_star);
             eina_strbuf_append(params_full_imp, prn);
             if (!dfv && is_empty)
               eina_strbuf_append(params_full_imp, " EINA_UNUSED");
             eina_strbuf_append(params, prn);

             if (eina_strbuf_length_get(params_full) || !eolian_function_is_static(fid))
               eina_strbuf_append(params_full, ", ");
             eina_strbuf_append(params_full, ptn);
             if (!had_star)
               eina_strbuf_append_char(params_full, ' ');
             eina_strbuf_append(params_full, add_star);
             eina_strbuf_append(params_full, prn);

             if (is_auto)
               {
                  if (ftype == EOLIAN_PROP_SET)
                    eina_strbuf_append_printf(params_init, "   pd->%s = %s;\n", prn, prn);
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
   if (impl_same_class && eolian_implement_is_pure_virtual(impl, ftype))
     impl_need = EINA_FALSE;

   if (!rtpn)
     rtpn = eina_stringshare_add("void");

   char *cname = NULL, *cnamel = NULL, *ocnamel = NULL;
   eo_gen_class_names_get(cl, &cname, NULL, &cnamel);
   eo_gen_class_names_get(eolian_implement_class_get(impl), NULL, NULL, &ocnamel);

   if (impl_need)
     {
        /* figure out the data type */
        Eina_Stringshare *dt = NULL;
        if (eolian_function_is_static(fid))
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
             if (!eolian_function_is_static(fid))
               {
                  if ((ftype == EOLIAN_PROP_GET) || eolian_function_object_is_const(fid))
                    eina_strbuf_append(buf, "const ");
                  eina_strbuf_append(buf, "Eo *obj, ");
                  eina_strbuf_append(buf, dt);
                  eina_strbuf_append(buf, " *pd");
               }
             eina_strbuf_append(buf, eina_strbuf_string_get(params_full_imp));
             if (eina_strbuf_length_get(params_full_imp) == 0 && eolian_function_is_static(fid))
               eina_strbuf_append(buf, "void");
             eina_strbuf_append(buf, ");\n\n");
          }

        if (is_empty || is_auto || eina_strbuf_length_get(params_init))
          {
             /* we need to give the internal function name to Eo,
              * use this hash table as indication
              */
             if (ftype == EOLIAN_PROP_SET)
               eina_hash_add(_funcs_params_init_set, &impl, impl);
             else
               eina_hash_add(_funcs_params_init_get, &impl, impl);
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
             if ((ftype == EOLIAN_PROP_GET) || eolian_function_object_is_const(fid))
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
                  eina_strbuf_append(buf, "   return ");
                  _append_defval(buf, def_ret, rtp, rtpn);
                  eina_strbuf_append(buf, ";\n");
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

   if (impl_same_class && !eolian_function_is_static(fid))
     {
        /* generate reflection implementation */
        if (reflect_param)
          {
             if (ftype == EOLIAN_PROP_GET)
               _gen_reflect_get(buf, cnamel, reflect_param, fid, refh);
             else
               _gen_reflect_set(buf, cnamel, reflect_param, fid, refh);
          }

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

        if (fallback_free_ownership)
          {
             //we have owned parameters we need to take care of
             eina_strbuf_append_printf(buf, "static void\n");
             eina_strbuf_append_printf(buf, "_%s_ownership_fallback(%s)\n{\n", eolian_function_full_c_name_get(fid, ftype), eina_strbuf_string_get(params_full) + 2);

             eina_strbuf_append_buffer(buf, fallback_free_ownership);
             eina_strbuf_append_printf(buf, "}\n\n");
          }

        eina_strbuf_append(buf, "EOAPI EFL_");
        if (!strcmp(rtpn, "void"))
          eina_strbuf_append(buf, "VOID_");
        eina_strbuf_append(buf, "FUNC_BODY");
        if (has_params)
          eina_strbuf_append_char(buf, 'V');
        if ((ftype == EOLIAN_PROP_GET) || eolian_function_object_is_const(fid)
            || eolian_function_is_static(fid))
          {
             eina_strbuf_append(buf, "_CONST");
          }

        if (fallback_free_ownership)
          eina_strbuf_append(buf, "_FALLBACK");

        eina_strbuf_append_char(buf, '(');

        Eina_Stringshare *eofn = eolian_function_full_c_name_get(fid, ftype);
        eina_strbuf_append(buf, eofn);

        if (strcmp(rtpn, "void"))
          {
             eina_strbuf_append_printf(buf, ", %s, ", rtpn);
             _append_defval(buf, def_ret, rtp, rtpn);
          }

        if (fallback_free_ownership)
          eina_strbuf_append_printf(buf, ", _%s_ownership_fallback(%s);", eolian_function_full_c_name_get(fid, ftype), eina_strbuf_string_get(params));

        if (has_params)
          {
             eina_strbuf_append(buf, ", EFL_FUNC_CALL(");
             eina_strbuf_append(buf, eina_strbuf_string_get(params));
             eina_strbuf_append_char(buf, ')');
             eina_strbuf_append(buf, eina_strbuf_string_get(params_full));
          }

        eina_strbuf_append(buf, ");\n");

        eina_stringshare_del(eofn);
     }
   if (impl_same_class && eolian_function_is_static(fid))
     _emit_class_function(buf, fid, rtpn, params_full, ocnamel, func_suffix, params, eolian_function_full_c_name_get(fid, ftype));

   free(cname);
   free(cnamel);
   free(ocnamel);

   eina_stringshare_del(rtpn);

   eina_strbuf_free(params);
   eina_strbuf_free(params_full);
   eina_strbuf_free(params_full_imp);
   eina_strbuf_free(params_init);
   if (fallback_free_ownership)
     eina_strbuf_free(fallback_free_ownership);
}

static void
_gen_opfunc(const Eolian_Function *fid, Eolian_Function_Type ftype,
            Eina_Strbuf *buf, const Eolian_Implement *impl, Eina_Bool pinit,
            const char *cnamel, const char *ocnamel)
{
   Eina_Stringshare *fnm = eolian_function_full_c_name_get(fid, ftype);
   eina_strbuf_append(buf, "      EFL_OBJECT_OP_FUNC(");
   eina_strbuf_append(buf, fnm);
   eina_strbuf_append(buf, ", ");
   if (!ocnamel && eolian_implement_is_pure_virtual(impl, ftype))
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

static void
_gen_reflop(const Eolian_Function *fid, Eina_Strbuf *buf, const char *cnamel, Eina_Hash *refh)
{
   Eolian_Function_Type aftype = (Eolian_Function_Type)eina_hash_find(refh, &fid);
   if (aftype == EOLIAN_UNRESOLVED)
     return;

   eina_strbuf_append_printf(buf, "      {\"%s\", ", eolian_function_name_get(fid));

   if (aftype == EOLIAN_PROP_SET || aftype == EOLIAN_PROPERTY)
     {
        eina_strbuf_append_printf(buf, "__eolian_%s_%s_set_reflect, ",
          cnamel, eolian_function_name_get(fid));
     }
   else
     eina_strbuf_append(buf, "NULL, ");

   if (aftype == EOLIAN_PROP_GET || aftype == EOLIAN_PROPERTY)
     {
        eina_strbuf_append_printf(buf, "__eolian_%s_%s_get_reflect},\n",
          cnamel, eolian_function_name_get(fid));
     }
   else
     eina_strbuf_append(buf, "NULL},\n");
}

static void
_gen_initializer(const Eolian_Class *cl, Eina_Strbuf *buf, Eina_Hash *refh)
{
   char *cnamel = NULL, *cnameu = NULL;
   eo_gen_class_names_get(cl, NULL, &cnameu, &cnamel);

   eina_strbuf_append(buf, "\nstatic Eina_Bool\n_");
   eina_strbuf_append(buf, cnamel);
   eina_strbuf_append(buf, "_class_initializer(Efl_Class *klass)\n{\n");
   eina_strbuf_append(buf, "   const Efl_Object_Ops *opsp = NULL;\n\n");
   eina_strbuf_append(buf, "   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;\n\n");

   Eina_Strbuf *ops = eina_strbuf_new();
   Eina_Strbuf *refls = eina_strbuf_new();

   /* start over with clean itearator */
   const Eolian_Implement *imp;
   Eina_Iterator *itr = eolian_class_implements_get(cl);
   EINA_ITERATOR_FOREACH(itr, imp)
     {
        const Eolian_Class *icl = eolian_implement_class_get(imp);
        Eolian_Function_Type ftype;
        const Eolian_Function *fid = eolian_implement_function_get(imp, &ftype);

        if (eolian_function_is_static(fid)) continue;

        if (!eina_strbuf_length_get(ops))
          eina_strbuf_append_printf(ops, "   EFL_OPS_DEFINE(ops,\n");

        Eina_Bool found_get = !!eina_hash_find(_funcs_params_init_get, &imp);
        Eina_Bool found_set = !!eina_hash_find(_funcs_params_init_set, &imp);
        char *ocnamel = NULL;
        if (cl != icl)
          eo_gen_class_names_get(icl, NULL, NULL, &ocnamel);

        switch (ftype)
          {
           case EOLIAN_PROP_GET:
             _gen_opfunc(fid, EOLIAN_PROP_GET, ops, imp, found_get, cnamel, ocnamel);
             _gen_reflop(fid, refls, cnamel, refh);
             break;
           case EOLIAN_PROP_SET:
             _gen_opfunc(fid, EOLIAN_PROP_SET, ops, imp, found_set, cnamel, ocnamel);
             _gen_reflop(fid, refls, cnamel, refh);
             break;
           case EOLIAN_PROPERTY:
             _gen_opfunc(fid, EOLIAN_PROP_SET, ops, imp, found_set, cnamel, ocnamel);
             _gen_opfunc(fid, EOLIAN_PROP_GET, ops, imp, found_get, cnamel, ocnamel);
             _gen_reflop(fid, refls, cnamel, refh);
             break;
           default:
             _gen_opfunc(fid, EOLIAN_METHOD, ops, imp, found_get, cnamel, ocnamel);
             break;
          }

        free(ocnamel);
     }
   eina_iterator_free(itr);

   if (eina_strbuf_length_get(ops))
     {
        /* make sure the extras are defined */
        eina_strbuf_append_printf(buf, "#ifndef %s_EXTRA_OPS\n", cnameu);
        eina_strbuf_append_printf(buf, "#define %s_EXTRA_OPS\n", cnameu);
        eina_strbuf_append(buf, "#endif\n\n");

        eina_strbuf_append_printf(ops, "      %s_EXTRA_OPS\n   );\n", cnameu);
        eina_strbuf_append(buf, eina_strbuf_string_get(ops));
        eina_strbuf_append(buf, "   opsp = &ops;\n\n");
     }
   else
     {
        /* no predefined, but if custom ones are required define it anyway */
        eina_strbuf_append_printf(buf, "#ifdef %s_EXTRA_OPS\n", cnameu);
        eina_strbuf_append_printf(buf, "   EFL_OPS_DEFINE(ops, %s_EXTRA_OPS);\n", cnameu);
        eina_strbuf_append(buf, "   opsp = &ops;\n");
        eina_strbuf_append(buf, "#endif\n\n");
     }

   if (eina_strbuf_length_get(refls))
     {
        eina_strbuf_append(buf, "   static const Efl_Object_Property_Reflection refl_table[] = {\n");
        eina_strbuf_append_buffer(buf, refls);
        eina_strbuf_append(buf, "   };\n");
        eina_strbuf_append(buf, "   static const Efl_Object_Property_Reflection_Ops rops = {\n");
        eina_strbuf_append(buf, "      refl_table, EINA_C_ARRAY_LENGTH(refl_table)\n");
        eina_strbuf_append(buf, "   };\n");
        eina_strbuf_append(buf, "   ropsp = &rops;\n\n");
     }

   eina_strbuf_append(buf, "   return efl_class_functions_set(klass, opsp, ropsp);\n");

   eina_strbuf_free(ops);
   eina_strbuf_free(refls);

   eina_strbuf_append(buf, "}\n\n");

   free(cnameu);
   free(cnamel);
}

void
eo_gen_source_gen(const Eolian_Class *cl, Eina_Strbuf *buf)
{
   if (!cl)
     return;

   _funcs_params_init_get = eina_hash_pointer_new(NULL);
   _funcs_params_init_set = eina_hash_pointer_new(NULL);

   char *cnamel = NULL;
   eo_gen_class_names_get(cl, NULL, NULL, &cnamel);

   /* event section, they come first */
   {
      Eina_Iterator *itr = eolian_class_events_get(cl);
      Eolian_Event *ev;
      EINA_ITERATOR_FOREACH(itr, ev)
        {
           Eina_Stringshare *evn = eolian_event_c_macro_get(ev);
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

   /* Eolian_Function -> Eolian_Function_Type
    * maps which parts of which functions are qualified for reflection
    */
   Eina_Hash *refh = eina_hash_pointer_new(NULL);

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
                _gen_func(cl, fid, ftype, buf, imp, refh);
                break;
              case EOLIAN_PROPERTY:
                _gen_func(cl, fid, EOLIAN_PROP_SET, buf, imp, refh);
                _gen_func(cl, fid, EOLIAN_PROP_GET, buf, imp, refh);
                break;
              default:
                _gen_func(cl, fid, EOLIAN_METHOD, buf, imp, refh);
             }
        }
      eina_iterator_free(itr);
   }

   /* class initializer - contains method defs */
   _gen_initializer(cl, buf, refh);
   eina_hash_free(refh);

   /* class description */
   eina_strbuf_append(buf, "static const Efl_Class_Description _");
   eina_strbuf_append(buf, cnamel);
   eina_strbuf_append(buf, "_class_desc = {\n"
                           "   EO_VERSION,\n");
   eina_strbuf_append_printf(buf, "   \"%s\",\n", eolian_class_name_get(cl));

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

   eina_strbuf_append_printf(buf, "   _%s_class_initializer,\n", cnamel);

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
      const Eolian_Class *icl = eolian_class_parent_get(cl);
      /* no inherits, NULL parent */
      if (!icl)
        eina_strbuf_append(buf, ", NULL");
      else
        {
           Eina_Stringshare *mname = eolian_class_c_macro_get(icl);
           eina_strbuf_append_printf(buf, ", %s", mname);
           eina_stringshare_del(mname);
        }
      Eina_Iterator *itr = eolian_class_extensions_get(cl);
      EINA_ITERATOR_FOREACH(itr, icl)
        {
           Eina_Stringshare *mname = eolian_class_c_macro_get(icl);
           eina_strbuf_append_printf(buf, ", %s", mname);
           eina_stringshare_del(mname);
        }
      eina_iterator_free(itr);
   }

   /* terminate inherits */
   eina_strbuf_append(buf, ", NULL);\n");

   /* and we're done */
   free(cnamel);
   eina_hash_free(_funcs_params_init_get);
   eina_hash_free(_funcs_params_init_set);
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
           Eina_Stringshare *ptn = eolian_parameter_c_type_get(pr, EINA_FALSE);

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
             Eolian_Parameter_Direction pd = eolian_parameter_direction_get(pr);
             const char *prn = eolian_parameter_name_get(pr);
             const Eolian_Type *pt = eolian_parameter_type_get(pr);
             const Eolian_Typedecl *ptd = eolian_type_typedecl_get(pt);
             Eina_Stringshare *ptn = eolian_parameter_c_type_get(pr, EINA_FALSE);

             if (ptd && eolian_typedecl_type_get(ptd) == EOLIAN_TYPEDECL_FUNCTION_POINTER)
               {
                  eina_strbuf_append_printf(params, ", %s_data, %s, %s_free_cb", prn, prn, prn);
                  eina_strbuf_append_printf(params_full, ", void *%s_data, %s %s, Eina_Free_Cb %s_free_cb", prn, ptn, prn, prn);

                  eina_stringshare_del(ptn);
                  continue;
               }

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
           const Eolian_Implement *impl, const char *dtype, const char *cnamel)
{
   Eina_Bool impl_same_class = (eolian_implement_class_get(impl) == cl);
   if (impl_same_class && eolian_implement_is_pure_virtual(impl, ftype))
     return;

   char *ocnamel = NULL;
   if (!impl_same_class)
     eo_gen_class_names_get(eolian_implement_class_get(impl), NULL, NULL, &ocnamel);

   char fname[256 + 128], iname[256];
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
   Eina_Stringshare *rtpn = NULL;
   if ((ftype == EOLIAN_PROP_GET) && !rtp)
     {
        void *d1, *d2;
        Eina_Iterator *itr = eolian_property_values_get(fid, ftype);
        if (eina_iterator_next(itr, &d1) && !eina_iterator_next(itr, &d2))
          {
             Eolian_Function_Parameter *pr = d1;
             rtp = eolian_parameter_type_get(pr);
             rtpn = eolian_parameter_c_type_get(pr, EINA_FALSE);
             var_as_ret = EINA_TRUE;
          }
        eina_iterator_free(itr);
     }

   eina_strbuf_append(buf, "EOLIAN static ");
   if (rtp)
     {
        if (!rtpn)
          rtpn = eolian_function_return_c_type_get(fid, ftype);
        eina_strbuf_append(buf, rtpn);
        eina_stringshare_del(rtpn);
     }
   else
     eina_strbuf_append(buf, "void");

   eina_strbuf_append_printf(buf, "\n%s(", fname);

   if ((ftype == EOLIAN_PROP_GET) || eolian_function_object_is_const(fid))
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
          Eina_Stringshare *fcn = eolian_function_full_c_name_get(fid, ftype);
          Eina_Stringshare *mname = eolian_class_c_macro_get(cl);
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
                _gen_proto(cl, fid, EOLIAN_METHOD, buf, imp, dt, cnamel);
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
