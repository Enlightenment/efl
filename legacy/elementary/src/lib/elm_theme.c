#include <Elementary.h>
#include "elm_priv.h"

int
_elm_theme_set(Evas_Object *o, const char *clas, const char *group, const char *style)
{
   char buf[PATH_MAX];
   char buf2[1024];
   int ok;
   
   // FIXME: actually handle themes for real
   snprintf(buf, sizeof(buf), "%s/themes/%s.edj", PACKAGE_DATA_DIR, "default");
   snprintf(buf2, sizeof(buf2), "elm/%s/%s/%s", clas, group, style);
   ok = edje_object_file_set(o, buf, buf2);
   if (ok) return 1;
   snprintf(buf2, sizeof(buf2), "elm/%s/%s/default", clas, group);
   ok = edje_object_file_set(o, buf, buf2);
   return ok;
}

int
_elm_theme_icon_set(Evas_Object *o, const char *group, const char *style)
{
   char buf[PATH_MAX];
   char buf2[1024];
   int w, h;
   int ok;
   
   // FIXME: actually handle themes for real
   snprintf(buf, sizeof(buf), "%s/themes/%s.edj", PACKAGE_DATA_DIR, "default");
   snprintf(buf2, sizeof(buf2), "elm/icon/%s/%s", group, style);
   _els_smart_icon_file_edje_set(o, buf, buf2);
   _els_smart_icon_size_get(o, &w, &h);
   if (w > 0) return 1;
   snprintf(buf2, sizeof(buf2), "elm/icon/%s/default", group);
   _els_smart_icon_file_edje_set(o, buf, buf2);
   _els_smart_icon_size_get(o, &w, &h);
   return (w > 0);
}
