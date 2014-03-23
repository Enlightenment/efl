#include "common_funcs.h"
#include "Eolian.h"

int _eolian_gen_log_dom = -1;

const char *current_classname = NULL;

void
_class_func_names_fill(const char *classname, const char *funcname)
{
   char *p;
   const char *eo_prefix = NULL;
   if (classname != current_classname)
     {
        current_classname = classname;

        /* class/CLASS*/
        strncpy(capclass, classname, sizeof(capclass) - 1);
        p = capclass;
        eina_str_toupper(&p);
        strncpy(lowclass, classname, sizeof(lowclass) - 1);
        p = lowclass;
        eina_str_tolower(&p);

        /* eo_prefix */
        if (eolian_class_exists(classname))
           eo_prefix = eolian_class_eo_prefix_get(classname);
        if (!eo_prefix) eo_prefix = classname;
        strncpy(current_eo_prefix_lower, eo_prefix, sizeof(current_eo_prefix_lower) - 1);
        p = current_eo_prefix_lower;
        eina_str_tolower(&p);
        strncpy(current_eo_prefix_upper, eo_prefix, sizeof(current_eo_prefix_lower) - 1);
        p = current_eo_prefix_upper;
        eina_str_toupper(&p);
     }
   if (funcname)
     {
        strncpy(capfunc, funcname, sizeof(capfunc) - 1);
        p = capfunc;
        eina_str_toupper(&p);
     }
}

void
_template_fill(Eina_Strbuf *buf, const char* templ, const char* classname, const char *funcname, Eina_Bool reset)
{
   _class_func_names_fill(classname, funcname);
   if (buf)
     {
        if (reset) eina_strbuf_reset(buf);
        if (templ) eina_strbuf_append(buf, templ);
        if (funcname) eina_strbuf_replace_all(buf, "@#func", funcname);
        eina_strbuf_replace_all(buf, "@#FUNC", capfunc);
        eina_strbuf_replace_all(buf, "@#Class", classname);
        eina_strbuf_replace_all(buf, "@#class", lowclass);
        eina_strbuf_replace_all(buf, "@#CLASS", capclass);
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

