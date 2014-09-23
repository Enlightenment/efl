#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

void
database_parameter_del(Eolian_Function_Parameter *pdesc)
{
   if (pdesc->base.file) eina_stringshare_del(pdesc->base.file);
   eina_stringshare_del(pdesc->name);

   database_type_del(pdesc->type);
   eina_stringshare_del(pdesc->description);
   free(pdesc);
}
