#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

EAPI const Eolian_Class *
eolian_part_class_get(const Eolian_Part *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   return part->klass;
}

EAPI const Eolian_Documentation *
eolian_part_documentation_get(const Eolian_Part *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   return part->doc;
}
