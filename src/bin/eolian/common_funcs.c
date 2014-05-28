#include "common_funcs.h"
#include "Eolian.h"

int _eolian_gen_log_dom = -1;

Eolian_Class current_class;

static void
_class_name_concatenate(const Eolian_Class class, char *buffer)
{
   const Eina_List *list = eolian_class_namespaces_list_get(class), *itr;
   const char *name;
   buffer[0] = '\0';
   EINA_LIST_FOREACH(list, itr, name)
     {
        sprintf(buffer, "%s_", name);
        buffer += (strlen(name) + 1);
     }
   sprintf(buffer, "%s", eolian_class_name_get(class));
}

void
_class_env_create(const Eolian_Class class, const char *over_classname, _eolian_class_vars *env)
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
}

void
_class_func_names_fill(const Eolian_Class class EINA_UNUSED, const char *over_classname EINA_UNUSED, const char *funcname)
{
   char *p;
   if (funcname)
     {
        strncpy(capfunc, funcname, sizeof(capfunc) - 1);
        p = capfunc;
        eina_str_toupper(&p);
     }
}

void
_template_fill(Eina_Strbuf *buf, const char *templ, const Eolian_Class class, const char *classname, const char *funcname, Eina_Bool reset)
{
   _eolian_class_vars tmp_env;
   _class_env_create(class, classname, &tmp_env);
   _class_func_names_fill(NULL, NULL, funcname);
   if (buf)
     {
        if (reset) eina_strbuf_reset(buf);
        if (templ) eina_strbuf_append(buf, templ);
        if (funcname) eina_strbuf_replace_all(buf, "@#func", funcname);
        eina_strbuf_replace_all(buf, "@#FUNC", capfunc);
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

