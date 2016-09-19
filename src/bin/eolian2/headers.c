#include "main.h"

const char *
_cl_type_str_get(const Eolian_Class *cl, Eina_Bool uc)
{
   switch (eolian_class_type_get(cl))
     {
      case EOLIAN_CLASS_REGULAR:
      case EOLIAN_CLASS_ABSTRACT:
        return uc ? "CLASS" : "class";
      case EOLIAN_CLASS_MIXIN:
        return uc ? "MIXIN" : "mixin";
      case EOLIAN_CLASS_INTERFACE:
        return uc ? "INTERFACE" : "interface";
      default:
        return NULL;
     }
}

static void
_gen_func(const Eolian_Class *cl, const Eolian_Function *fid,
          Eolian_Function_Type ftype, Eina_Strbuf *buf)
{
}

void
eo_gen_header_gen(const Eolian_Class *cl, Eina_Strbuf *buf, Eina_Bool legacy)
{
   if (!cl)
     return;

   char *cname = NULL, *cnameu = NULL, *cnamel = NULL;

   cname = eo_gen_c_full_name_get(eolian_class_full_name_get(cl));
   if (!cname)
     goto end;

   cnameu = strdup(cname);
   if (!cnameu)
     goto end;
   eina_str_toupper(&cnameu);

   cnamel = strdup(cname);
   if (!cnamel)
     goto end;
   eina_str_tolower(&cnamel);

   /* class definition */

   eina_strbuf_append_printf(buf, "#define %s_%s %s_%s_get()\n\n",
                             cnameu, _cl_type_str_get(cl, EINA_TRUE),
                             cnamel, _cl_type_str_get(cl, EINA_FALSE));

   eina_strbuf_append_printf(buf, "EWAPI const Efl_Class *%s_%s_get(void);\n\n",
                             cnamel, _cl_type_str_get(cl, EINA_FALSE));

   /* method section */
   {
      Eina_Iterator *itr = eolian_class_implements_get(cl);
      if (!itr)
        goto events;

      const Eolian_Implement *imp;
      EINA_ITERATOR_FOREACH(itr, imp)
        {
           if (eolian_implement_class_get(imp) != cl)
             continue;
           Eolian_Function_Type ftype = EOLIAN_UNRESOLVED;
           const Eolian_Function *fid = eolian_implement_function_get(imp, &ftype);
           switch (ftype)
             {
              case EOLIAN_PROP_GET:
              case EOLIAN_PROP_SET:
                _gen_func(cl, fid, ftype, buf);
                break;
              case EOLIAN_PROPERTY:
                _gen_func(cl, fid, EOLIAN_PROP_SET, buf);
                _gen_func(cl, fid, EOLIAN_PROP_GET, buf);
                break;
              default:
                _gen_func(cl, fid, EOLIAN_UNRESOLVED, buf);
             }
        }
      eina_iterator_free(itr);
   }

events:
   /* event section */
   {
      Eina_Iterator *itr = eolian_class_events_get(cl);
      Eolian_Event *ev;
      EINA_ITERATOR_FOREACH(itr, ev)
        {
           Eina_Stringshare *evn = eolian_event_c_name_get(ev);
           Eolian_Object_Scope evs = eolian_event_scope_get(ev);

           if (evs == EOLIAN_SCOPE_PRIVATE)
             continue;

           if (eolian_event_is_beta(ev))
             {
                eina_strbuf_append_printf(buf, "\n#ifndef %s_BETA\n", cnameu);
                eina_strbuf_append_printf(buf, "#define %s_BETA\n", cnameu);
             }
           if (evs == EOLIAN_SCOPE_PROTECTED)
             {
                if (!eolian_event_is_beta(ev))
                  eina_strbuf_append_char(buf, '\n');
                eina_strbuf_append_printf(buf, "#ifndef %s_PROTECTED\n", cnameu);
                eina_strbuf_append_printf(buf, "#define %s_PROTECTED\n", cnameu);
             }

           if (!eolian_event_is_beta(ev) && evs == EOLIAN_SCOPE_PUBLIC)
             eina_strbuf_append_char(buf, '\n');

           eina_strbuf_append_printf(buf, "EOAPI extern const "
                                     "Efl_Event_Description _%s;\n", evn);
           eina_strbuf_append_printf(buf, "#define %s (&(_%s))\n", evn, evn);

           if (evs == EOLIAN_SCOPE_PROTECTED)
             eina_strbuf_append(buf, "#endif\n");
           if (eolian_event_is_beta(ev))
             eina_strbuf_append(buf, "#endif\n");

           eina_stringshare_del(evn);
        }
      eina_iterator_free(itr);
   }

end:
   free(cname);
   free(cnameu);
   free(cnamel);
}
