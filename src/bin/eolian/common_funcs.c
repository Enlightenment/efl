#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

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

static char *
_fill_envs(char *dest, const char *src, size_t bufs) {
    strncpy(dest, src, bufs - 1);
    dest[bufs - 1] = '\0';
    return dest;
}

void
_class_env_create(const Eolian_Class *class, const char *over_classname, _eolian_class_vars *env)
{
   if (!env) return;

   const char *eo_prefix = NULL;
   char *p;

   if (!class)
      _fill_envs(env->full_classname, over_classname, sizeof(env->full_classname));
   else
      _class_name_concatenate(class, env->full_classname);

   /* class/CLASS*/
   p = _fill_envs(env->upper_classname, env->full_classname, sizeof(env->upper_classname));
   eina_str_toupper(&p);
   p = _fill_envs(env->lower_classname, env->full_classname, sizeof(env->lower_classname));
   eina_str_tolower(&p);

   /* eo_prefix */
   if (class) eo_prefix = eolian_class_eo_prefix_get(class);
   if (!eo_prefix) eo_prefix = env->full_classname;
   p = _fill_envs(env->upper_eo_prefix, eo_prefix, sizeof(env->upper_eo_prefix));
   eina_str_toupper(&p);
   p = _fill_envs(env->lower_eo_prefix, eo_prefix, sizeof(env->lower_eo_prefix));
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
   const Eolian_Function *funcid = eolian_class_function_get_by_name(class, funcname, ftype);

   p = _fill_envs(env->upper_func, funcname, sizeof(env->upper_func));
   eina_str_toupper(&p);

   Eolian_Function_Type aftype = ftype;
   if (aftype == EOLIAN_PROPERTY) aftype = EOLIAN_METHOD;

   Eina_Stringshare *fname = eolian_function_full_c_name_get(funcid, aftype, EINA_FALSE);
   p = _fill_envs(env->upper_eo_func, fname, sizeof(env->upper_eo_func));
   eina_str_toupper(&p);
   p = _fill_envs(env->lower_eo_func, fname, sizeof(env->lower_eo_func));
   eina_str_tolower(&p);
   eina_stringshare_del(fname);

   Eina_Stringshare *lname = eolian_function_full_c_name_get(funcid, aftype, EINA_TRUE);
   env->legacy_func[0] = '\0';
   if (!lname) return;
   p = _fill_envs(env->legacy_func, lname, sizeof(env->legacy_func));
   eina_stringshare_del(lname);
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

const char *
_get_add_star(Eolian_Function_Type ftype, Eolian_Parameter_Dir pdir)
{
   switch (ftype)
     {
      case EOLIAN_PROP_GET:
        if (pdir == EOLIAN_REF_PARAM)
          return "**";
        else
          return "*";
      case EOLIAN_PROP_SET:
        if (pdir == EOLIAN_REF_PARAM)
          return "*";
        else
          return "";
      default:
        if (pdir != EOLIAN_IN_PARAM)
          return "*";
        else
          return "";
     }
   return "";
}