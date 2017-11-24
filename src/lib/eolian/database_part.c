#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

void
database_part_del(Eolian_Part *part)
{
   if (!part) return;
   if (part->name) eina_stringshare_del(part->name);
   database_doc_del(part->doc);
   free(part);
}
