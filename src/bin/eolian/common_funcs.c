#include "common_funcs.h"

void
_template_fill(Eina_Strbuf *buf, const char* templ, const char* classname, const char *funcname, Eina_Bool reset)
{
   static char capobjclass[0xFF];
   static char lowobjclass[0xFF];
   static char capclass[0xFF];
   static char lowclass[0xFF];
   static char normclass[0xFF];
   static char capfunc[0xFF];
   static char eoprefix[0xFF];

   char *p;

   if (reset) eina_strbuf_reset(buf);
   if (templ) eina_strbuf_append(buf, templ);

   if (strcmp(classname, normclass))
     {
        //Fill cache
        strcpy(normclass, classname);

        strcpy(capclass, classname);
        p = capclass;
        eina_str_toupper(&p);

        strcpy(lowclass, classname);
        p = lowclass;
        eina_str_tolower(&p);

        Eina_Strbuf *classobj = eina_strbuf_new();
        eina_strbuf_append(classobj, classname);

        // More to exclusion list
        if (strcmp(classname, "Eo_Base") && strcmp(classname, "Elm_Widget"))
          eina_strbuf_replace(classobj, "_", "_obj_", 1);

        if (!strcmp(classname, "Evas_Object"))
          {
            eina_strbuf_reset(classobj);
            eina_strbuf_append(classobj, "Evas_Obj");
          }

        strcpy(capobjclass, eina_strbuf_string_get(classobj));
        p = capobjclass;
        eina_str_toupper(&p);

        strcpy(lowobjclass, eina_strbuf_string_get(classobj));
        p = lowobjclass;
        eina_str_tolower(&p);

        strcpy(eoprefix, lowobjclass);

        if (!strcmp(classname, "Elm_Widget"))
          strcpy(eoprefix, "elm_wdg");

        eina_strbuf_free(classobj);
     }

   strcpy(capfunc, funcname);
   p = capfunc; eina_str_toupper(&p);

   eina_strbuf_replace_all(buf, "@#func", funcname);
   eina_strbuf_replace_all(buf, "@#FUNC", capfunc);
   eina_strbuf_replace_all(buf, "@#Class", classname);
   eina_strbuf_replace_all(buf, "@#class", lowclass);
   eina_strbuf_replace_all(buf, "@#CLASS", capclass);
   eina_strbuf_replace_all(buf, "@#OBJCLASS", capobjclass);
   eina_strbuf_replace_all(buf, "@#objclass", lowobjclass);
   eina_strbuf_replace_all(buf, "@#eoprefix", eoprefix);
}

