#include "common_funcs.h"
#include "Eolian.h"

int _eolian_gen_log_dom = -1;

Eolian_Class *current_class;

static void
_class_name_concatenate(const Eolian_Class *class, char *buffer)
{
   Eina_Iterator *itr = eolian_class_namespaces_get(class);
   const char *name;
   buffer[0] = '\0';
   EINA_ITERATOR_FOREACH(itr, name)
     {
        sprintf(buffer, "%s_", name);
        buffer += (strlen(name) + 1);
     }
   sprintf(buffer, "%s", eolian_class_name_get(class));
   eina_iterator_free(itr);
}

void
_class_env_create(const Eolian_Class *class, const char *over_classname, _eolian_class_vars *env)
{
   if (!env) return;

   const char *eo_prefix = NULL;
   char *p;

   if (!class)
      strncpy(env->full_classname, over_classname, PATH_MAX - 1);
   else
      _class_name_concatenate(class, env->full_classname);

   /* class/CLASS*/
   p = strncpy(env->upper_classname, env->full_classname, PATH_MAX - 1);
   eina_str_toupper(&p);
   p = strncpy(env->lower_classname, env->full_classname, PATH_MAX - 1);
   eina_str_tolower(&p);

   /* eo_prefix */
   if (class) eo_prefix = eolian_class_eo_prefix_get(class);
   if (!eo_prefix) eo_prefix = env->full_classname;
   p = strncpy(env->upper_eo_prefix, eo_prefix, PATH_MAX - 1);
   eina_str_toupper(&p);
   p = strncpy(env->lower_eo_prefix, eo_prefix, PATH_MAX - 1);
   eina_str_tolower(&p);

   /* classtype */
   if (class) switch (eolian_class_type_get(class))
     {
        case EOLIAN_CLASS_REGULAR:
        case EOLIAN_CLASS_ABSTRACT:
           strcpy(env->lower_classtype, "class");
           strcpy(env->upper_classtype, "CLASS");
           break;
        case EOLIAN_CLASS_MIXIN:
           strcpy(env->lower_classtype, "mixin");
           strcpy(env->upper_classtype, "MIXIN");
           break;
        case EOLIAN_CLASS_INTERFACE:
           strcpy(env->lower_classtype, "interface");
           strcpy(env->upper_classtype, "INTERFACE");
           break;
        default:
           break;
     }
}

void
_class_func_env_create(const Eolian_Class *class, const char *funcname, Eolian_Function_Type ftype, _eolian_class_func_vars *env)
{
   char *p;
   const char *ret;
   const char *suffix = "";
   const char *legacy = NULL;
   const Eolian_Function *funcid = eolian_class_function_get_by_name(class, funcname, ftype);
   if (ftype == EOLIAN_PROP_GET)
     {
        suffix = "_get";
        legacy = eolian_function_data_get(funcid, EOLIAN_LEGACY_GET);
     }
   if (ftype == EOLIAN_PROP_SET)
     {
        suffix = "_set";
        legacy = eolian_function_data_get(funcid, EOLIAN_LEGACY_SET);
     }
   if (!legacy) legacy = eolian_function_data_get(funcid, EOLIAN_LEGACY);

   _eolian_class_vars tmp_env;
   _class_env_create(class, NULL, &tmp_env);

   p = strncpy(env->upper_func, funcname, PATH_MAX - 1);
   eina_str_toupper(&p);

   ret = eolian_function_full_c_name_get(funcid, tmp_env.upper_eo_prefix);
   sprintf(p = env->upper_eo_func, "%s%s", ret, suffix);
   eina_str_toupper(&p);
   eina_stringshare_del(ret);

   ret = eolian_function_full_c_name_get(funcid, tmp_env.lower_eo_prefix);
   sprintf(p = env->lower_eo_func, "%s%s", ret, suffix);
   eina_str_tolower(&p);
   eina_stringshare_del(ret);

   env->legacy_func[0] = '\0';
   if (legacy && !strcmp(legacy, "null")) goto end;
   if (legacy)
     {
        sprintf(p = env->legacy_func, "%s", legacy);
        goto end;
     }

   legacy = eolian_class_legacy_prefix_get(class);
   if (legacy && !strcmp(legacy, "null")) goto end;

   sprintf(env->legacy_func, "%s_%s%s", legacy?legacy:tmp_env.lower_classname, funcname, suffix);

end:
   return;
}

void
_template_fill(Eina_Strbuf *buf, const char *templ, const Eolian_Class *class, const char *classname, const char *funcname, Eina_Bool reset)
{
   _eolian_class_vars tmp_env;
   _eolian_class_func_vars tmp_func_env;
   _class_env_create(class, classname, &tmp_env);
   if (funcname)
      _class_func_env_create(class, funcname, EOLIAN_UNRESOLVED, &tmp_func_env);
   if (buf)
     {
        if (reset) eina_strbuf_reset(buf);
        if (templ) eina_strbuf_append(buf, templ);
        if (funcname)
          {
             eina_strbuf_replace_all(buf, "@#func", funcname);
             eina_strbuf_replace_all(buf, "@#FUNC", tmp_func_env.upper_func);
          }
        eina_strbuf_replace_all(buf, "@#classtype", tmp_env.lower_classtype);
        eina_strbuf_replace_all(buf, "@#CLASSTYPE", tmp_env.upper_classtype);
        eina_strbuf_replace_all(buf, "@#Class", tmp_env.full_classname);
        eina_strbuf_replace_all(buf, "@#class", tmp_env.lower_classname);
        eina_strbuf_replace_all(buf, "@#CLASS", tmp_env.upper_classname);
     }
}

char*
_nextline(char *str, unsigned int lines)
{
   if (!str) return NULL;

   char *ret = str;
   while (lines--)
     {
        ret= strchr(ret, '\n');
        if (ret) ret++;
        else return NULL;
     }
   return ret;
}

char*
_startline(char *str, char *pos)
{
   if (!str || !pos) return NULL;

   char *ret =  pos;
   while ((ret > str) && (*(ret-1)!='\n')) ret--;

   return ret;
}

char*
_source_desc_get(const char *str)
{
   Eina_Strbuf *part = eina_strbuf_new();
   if (str)
     {
        const char *p = strchr(str, '\n');
        size_t offs = (p) ? (size_t)(p - str) : strlen(str);
        eina_strbuf_append_n(part, str, offs);
        eina_strbuf_replace_all(part, "\\", "\\\\");
        eina_strbuf_replace_all(part, "\"", "\\\"");
     }
   char *ret = eina_strbuf_string_steal(part);
   eina_strbuf_free(part);
   return ret;
}
