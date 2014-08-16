#include <Eina.h>
#include <string.h>

#include "Eolian.h"
#include "impl_generator.h"
#include "common_funcs.h"

static _eolian_class_vars class_env;

static Eina_Bool
_params_generate(const Eolian_Function *foo, Eolian_Function_Type ftype, Eina_Bool var_as_ret, Eina_Strbuf *params, Eina_Strbuf *short_params)
{
   Eina_Iterator *itr;
   Eolian_Function_Parameter *param;
   eina_strbuf_reset(params);
   eina_strbuf_reset(short_params);
   itr = eolian_property_keys_get(foo);
   EINA_ITERATOR_FOREACH(itr, param)
     {
        const Eolian_Type *ptypet;
        const char *pname;
        const char *ptype;
        eolian_parameter_information_get(param, NULL, &ptypet, &pname, NULL);
        ptype = eolian_type_c_type_get(ptypet);
        Eina_Bool had_star = !!strchr(ptype, '*');
        Eina_Bool is_const = eolian_parameter_const_attribute_get(param, ftype == EOLIAN_PROP_GET);
        if (eina_strbuf_length_get(params))
          {
             eina_strbuf_append(params, ", ");
             eina_strbuf_append(short_params, ", ");
          }
        eina_strbuf_append_printf(params, "%s%s%s%s",
              is_const?"const ":"", ptype,
              had_star?"":" ",
              pname);
        eina_strbuf_append_printf(short_params, "%s", pname);
        eina_stringshare_del(ptype);
     }
   eina_iterator_free(itr);
   if (!var_as_ret)
     {
        Eina_Bool add_star = (ftype == EOLIAN_PROP_GET);
        itr = eolian_function_parameters_get(foo);
        EINA_ITERATOR_FOREACH(itr, param)
          {
             const Eolian_Type *ptypet;
             const char *pname;
             const char *ptype;
             Eolian_Parameter_Dir pdir;
             eolian_parameter_information_get(param, &pdir, &ptypet, &pname, NULL);
             ptype = eolian_type_c_type_get(ptypet);
             Eina_Bool is_const = eolian_parameter_const_attribute_get(param, ftype == EOLIAN_PROP_GET);
             Eina_Bool had_star = !!strchr(ptype, '*');
             if (ftype == EOLIAN_UNRESOLVED || ftype == EOLIAN_METHOD) add_star = (pdir == EOLIAN_OUT_PARAM || pdir == EOLIAN_INOUT_PARAM);
             if (eina_strbuf_length_get(params))
               {
                  eina_strbuf_append(params, ", ");
                  eina_strbuf_append(short_params, ", ");
               }
             eina_strbuf_append_printf(params, "%s%s%s%s%s",
                   is_const?"const ":"",
                   ptype, had_star?"":" ", add_star?"*":"", pname);
             eina_strbuf_append_printf(short_params, "%s", pname);
             eina_stringshare_del(ptype);
          }
        eina_iterator_free(itr);
     }
   return EINA_TRUE;
}

static Eina_Bool
_function_exists(const char* func_name, Eina_Strbuf *buffer)
{
   const char *ptr = eina_strbuf_string_get(buffer);
   int func_len = strlen(func_name);
   while ((ptr = strstr(ptr, func_name)) != NULL)
     {
        switch (*(ptr - 1))
          {
           case '\n': case ' ':
                {
                   switch (*(ptr + func_len))
                     {
                      case ' ': case '(':
                         return EINA_TRUE;
                     }
                }
          }
        ptr++; /* so strstr doesn't fall again on func_name */
     }
   return EINA_FALSE;
}

/* Check if the type is used in the file, not if it is a typedef... */
static Eina_Bool
_type_exists(const char* type_name, Eina_Strbuf *buffer)
{
   const char *ptr = eina_strbuf_string_get(buffer);
   int type_len = strlen(type_name);
   while ((ptr = strstr(ptr, type_name)) != NULL)
     {
        switch (*(ptr - 1))
          {
           case '\n': case ' ': case ',':
                {
                   switch (*(ptr + type_len))
                     {
                      case '\n': case ' ': case ',': case ';':
                         return EINA_TRUE;
                     }
                }
          }
        ptr++; /* so strstr doesn't fall again on type_name */
     }
   return EINA_FALSE;
}

static Eina_Bool
_prototype_generate(const Eolian_Function *foo, Eolian_Function_Type ftype, Eina_Strbuf *data_type_buf, Eolian_Implement *impl_desc, Eina_Strbuf *buffer)
{
   Eina_Bool var_as_ret = EINA_FALSE, ret_const = EINA_FALSE;
   Eina_Strbuf *params = NULL, *short_params = NULL, *super_invok = NULL;
   char func_name[PATH_MAX];
   char impl_name[PATH_MAX];
   const char *fname;
   int flen;
   _eolian_class_vars impl_env;

   if (!impl_desc && eolian_function_is_virtual_pure(foo, ftype)) return EINA_TRUE;

   super_invok = eina_strbuf_new();
   if (impl_desc)
     {
        const Eolian_Class *impl_class;
        eolian_implement_information_get(impl_desc, &impl_class, NULL, NULL);

        _class_env_create(impl_class, NULL, &impl_env);

        char *tmp = impl_name;
        sprintf(impl_name, "%s_%s", class_env.full_classname, impl_env.full_classname);
        eina_str_tolower(&tmp);
     }

   sprintf(func_name, "_%s_%s%s",
         impl_desc?impl_name:class_env.lower_classname, eolian_function_name_get(foo),
         ftype == EOLIAN_PROP_GET?"_get": (ftype == EOLIAN_PROP_SET?"_set":""));

   if (_function_exists(func_name, buffer)) goto end;

   printf("Generation of function %s\n", func_name);
   const Eolian_Type *rettypet = eolian_function_return_type_get(foo, ftype);
   if (ftype == EOLIAN_PROP_GET && !rettypet)
     {
        Eina_Iterator *itr = eolian_function_parameters_get(foo);
        void *data, *data2;
        /* We want to check if there is only one parameter */
        if (eina_iterator_next(itr, &data) && !eina_iterator_next(itr, &data2))
          {
             Eolian_Function_Parameter *param = data;
             eolian_parameter_information_get(param, NULL, &rettypet, NULL, NULL);
             var_as_ret = EINA_TRUE;
             ret_const = eolian_parameter_const_attribute_get(param, EINA_TRUE);
          }
        eina_iterator_free(itr);
     }

   params = eina_strbuf_new();
   short_params = eina_strbuf_new();
   _params_generate(foo, ftype, var_as_ret, params, short_params);
   if (eina_strbuf_length_get(params))
      eina_strbuf_prepend_printf(params, ", ");

   if (impl_desc && ftype == EOLIAN_CTOR)
     {
        eina_strbuf_append_printf(super_invok,
              "   eo_do_super(obj, %s_%s, %s_%s(%s));\n",
              class_env.upper_eo_prefix, class_env.upper_classtype,
              impl_env.lower_eo_prefix, eolian_function_name_get(foo),
              eina_strbuf_string_get(short_params));
     }

   fname = eolian_function_name_get(foo);
   flen = strlen(fname);
   if ((fname) && (flen >= strlen("destructor")))
     {
        if (impl_desc && !strcmp(fname + flen - strlen("destructor"), "destructor"))
          {
             eina_strbuf_append_printf(super_invok,
                   "   eo_do_super(obj, %s_%s, %s_%s(%s));\n",
                   class_env.upper_eo_prefix, class_env.upper_classtype,
                   impl_env.lower_eo_prefix, eolian_function_name_get(foo),
                   eina_strbuf_string_get(short_params));
          }
     }

   const char *rettype = NULL;
   if (rettypet) rettype = eolian_type_c_type_get(rettypet);

   eina_strbuf_append_printf(buffer,
         "EOLIAN static %s%s\n%s(%sEo *obj, %s *pd%s%s)\n{\n%s\n}\n\n",
         ret_const?"const ":"", !rettype?"void":rettype,
         func_name,
         eolian_function_object_is_const(foo)?"const ":"",
         !eina_strbuf_length_get(data_type_buf) ? "void" : eina_strbuf_string_get(data_type_buf),
         !eina_strbuf_length_get(data_type_buf) ? " EINA_UNUSED" : "",
         eina_strbuf_string_get(params),
         eina_strbuf_string_get(super_invok)
         );

   if (rettype) eina_stringshare_del(rettype);

end:
   eina_strbuf_free(short_params);
   eina_strbuf_free(params);
   eina_strbuf_free(super_invok);
   return EINA_TRUE;
}

Eina_Bool
impl_source_generate(const Eolian_Class *class, Eina_Strbuf *buffer)
{
   Eina_Bool ret = EINA_FALSE;
   Eina_Strbuf *data_type_buf = eina_strbuf_new();
   Eina_Iterator *itr;
   const Eolian_Function *foo;
   Eina_Strbuf *begin = eina_strbuf_new();
   const char *class_name = eolian_class_name_get(class);
   char core_incl[PATH_MAX];

   _class_env_create(class, NULL, &class_env);

   if (!_type_exists("EFL_BETA_API_SUPPORT", buffer))
     {
        printf("Generation of EFL_BETA_API_SUPPORT\n");
        eina_strbuf_append_printf(begin, "#define EFL_BETA_API_SUPPORT\n");
     }

   if (!_type_exists("<Eo.h>", buffer))
     {
        printf("Generation of #include <Eo.h> and \"%s.eo.h\"\n", class_env.lower_classname);
        eina_strbuf_append_printf(begin, "#include <Eo.h>\n#include \"%s.eo.h\"\n\n", class_env.lower_classname);
     }

   /* Little calculation of the prefix of the data */
   const char *data_type = eolian_class_data_type_get(class);
   if (data_type)
     {
        if (strcmp(data_type, "null"))
           eina_strbuf_append_printf(data_type_buf, "%s", data_type);
     }
   else
      eina_strbuf_append_printf(data_type_buf, "%s_Data", class_name);

   /* Definition of the structure */
   const char *data_type_str = eina_strbuf_string_get(data_type_buf);
   if (!_type_exists(data_type_str, buffer) && 0 != eina_strbuf_length_get(data_type_buf))
     {
        printf("Generation of type %s\n", data_type_str);
        eina_strbuf_append_printf(begin, "typedef struct\n{\n\n} %s;\n\n", data_type_str);
     }

   if (eina_strbuf_length_get(begin))
      eina_strbuf_prepend_printf(buffer, "%s", eina_strbuf_string_get(begin));
   eina_strbuf_free(begin);

   /* Properties */
   itr = eolian_class_functions_get(class, EOLIAN_PROPERTY);
   EINA_ITERATOR_FOREACH(itr, foo)
     {
        const Eolian_Function_Type ftype = eolian_function_type_get(foo);
        if (ftype == EOLIAN_PROP_SET || ftype == EOLIAN_PROPERTY)
           _prototype_generate(foo, EOLIAN_PROP_SET, data_type_buf, NULL, buffer);

        if (ftype == EOLIAN_PROP_GET || ftype == EOLIAN_PROPERTY)
           _prototype_generate(foo, EOLIAN_PROP_GET, data_type_buf, NULL, buffer);
     }
   eina_iterator_free(itr);

   /* Methods */
   itr = eolian_class_functions_get(class, EOLIAN_METHOD);
   EINA_ITERATOR_FOREACH(itr, foo)
     {
        _prototype_generate(foo, EOLIAN_METHOD, data_type_buf, NULL, buffer);
     }
   eina_iterator_free(itr);

   /* Custom constructors */
   itr = eolian_class_functions_get(class, EOLIAN_CTOR);
   EINA_ITERATOR_FOREACH(itr, foo)
     {
        _prototype_generate(foo, EOLIAN_CTOR, data_type_buf, NULL, buffer);
     }
   eina_iterator_free(itr);

   itr = eolian_class_implements_get(class);
   if (itr)
     {
        Eolian_Implement *impl_desc;
        EINA_ITERATOR_FOREACH(itr, impl_desc)
          {
             const Eolian_Class *impl_class = NULL;
             Eolian_Function_Type ftype;

             foo = NULL;
             eolian_implement_information_get(impl_desc, &impl_class, &foo, &ftype);
             if (!foo)
               {
                  ERR ("Failed to generate implementation of %s - missing form super class",
                        eolian_implement_full_name_get(impl_desc));
                  goto end;
               }
             switch (ftype)
               {
                case EOLIAN_PROP_SET: case EOLIAN_PROP_GET:
                   _prototype_generate(foo, ftype, data_type_buf, impl_desc, buffer);
                   break;
                default:
                   _prototype_generate(foo, eolian_function_type_get(foo), data_type_buf, impl_desc, buffer);
                   break;
               }
          }
        eina_iterator_free(itr);
     }

   if (eolian_class_ctor_enable_get(class))
     {
        char func_name[100];
        sprintf(func_name, "_%s_class_constructor", class_env.lower_classname);
        if (!_function_exists(func_name, buffer))
          {
             printf("Generation of function %s\n", func_name);
             eina_strbuf_append_printf(buffer,
                   "EOLIAN static void\n_%s_class_constructor(Eo_Class *klass)\n{\n\n}\n\n",
                   class_env.lower_classname);
          }
     }

   if (eolian_class_dtor_enable_get(class))
     {
        char func_name[100];
        sprintf(func_name, "_%s_class_destructor", class_env.lower_classname);
        if (!_function_exists(func_name, buffer))
          {
             printf("Generation of function %s\n", func_name);
             eina_strbuf_append_printf(buffer, "EOLIAN static void\n_%s_class_destructor(Eo_Class *klass)\n{\n\n}\n\n",
                   class_env.lower_classname);
          }
     }
   printf("Removal of all inline instances of #include \"%s.eo.c\"\n", class_env.lower_classname);
   snprintf(core_incl, sizeof(core_incl), "\n#include \"%s.eo.c\"\n", class_env.lower_classname);
   eina_strbuf_replace_all(buffer, core_incl, "\n");

   snprintf(core_incl, sizeof(core_incl), "\"%s.eo.c\"", class_env.lower_classname);
   printf("Generation of #include \"%s.eo.c\"\n", class_env.lower_classname);
   eina_strbuf_append_printf(buffer, "#include \"%s.eo.c\"\n", class_env.lower_classname);

   ret = EINA_TRUE;
end:
   eina_strbuf_free(data_type_buf);
   return ret;
}

