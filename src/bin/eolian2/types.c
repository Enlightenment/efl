#include "main.h"

Eina_Strbuf *eo_gen_class_typedef_gen(const char *eof)
{
   const Eolian_Class *cl = eolian_class_get_by_file(eof);
   if (!cl)
     return NULL;
   char *clfn = eo_gen_class_full_name_get(cl);
   if (!clfn)
     return NULL;
   Eina_Strbuf *ret = eina_strbuf_new();
   eina_strbuf_append_printf(ret, "typedef Eo %s;\n", clfn);
   free(clfn);
   return ret;
}
