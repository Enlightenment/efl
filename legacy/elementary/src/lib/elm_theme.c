#include <Elementary.h>
#include "elm_priv.h"

int
_elm_theme_set(Evas_Object *o, const char *clas, const char *group)
{
   char buf[PATH_MAX];
   
   // FIXME: actually handle themes for real
   snprintf(buf, sizeof(buf), "%s/themes/%s.edj", PACKAGE_DATA_DIR, 
	    "default");
   return edje_object_file_set(o, buf, group);
}
