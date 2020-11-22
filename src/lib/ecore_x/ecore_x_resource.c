#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "ecore_x_private.h"

static Eina_Bool _ecore_x_resource_initted = EINA_FALSE;
static XrmDatabase _ecore_x_resource_db = NULL;

static void
_ecore_x_resource_init(void)
{
   if (_ecore_x_resource_initted) return;
   XrmInitialize();
   _ecore_x_resource_initted = EINA_TRUE;
}

void
_ecore_x_resource_shutdown(void)
{
   if (!_ecore_x_disp) return;
   if (!_ecore_x_resource_initted) return;
   if (_ecore_x_resource_db) _ecore_x_resource_db = NULL;
   _ecore_x_resource_initted = EINA_FALSE;
}

EAPI void
ecore_x_rersource_load(const char *file)
{
   XrmDatabase db;

   if (!_ecore_x_disp) return;
   _ecore_x_resource_init();
   db = XrmGetFileDatabase(file);
   if (!db) return;
   if (_ecore_x_resource_db) XrmDestroyDatabase(_ecore_x_resource_db);
   _ecore_x_resource_db = db;
// something smells fishy/broken in xlib - this segfaults in trying to free
// up the previous db bveing used for that display...
//   XrmSetDatabase(_ecore_x_disp, db);
}

EAPI void
ecore_x_resource_db_string_set(const char *key, const char *val)
{
   if (!_ecore_x_disp) return;
   _ecore_x_resource_init();
   if ((!key) || (!val)) return;
   if (!_ecore_x_resource_db)
     _ecore_x_resource_db = XrmGetDatabase(_ecore_x_disp);
   XrmPutStringResource(&_ecore_x_resource_db, key, val);
}

EAPI const char *
ecore_x_resource_db_string_get(const char *key)
{
   char *type = NULL;
   XrmValue xval = { 0, NULL };

   if (!_ecore_x_disp) return NULL;
   _ecore_x_resource_init();
   if (!_ecore_x_resource_db)
     _ecore_x_resource_db = XrmGetDatabase(_ecore_x_disp);
   if (XrmGetResource(_ecore_x_resource_db, key, "String", &type, &xval))
     {
        if (xval.addr && (!strcmp(type, "String")))
          {
             if (xval.size > 0) return xval.addr;
          }
     }
   return NULL;
}

EAPI void
ecore_x_resource_db_flush(void)
{
   Ecore_X_Atom atom, type;
   Ecore_X_Window *roots;
   int i, num, fd;
   char *str;
   Eina_Tmpstr *path = NULL;
   off_t offset;

   if (!_ecore_x_disp) return;
   _ecore_x_resource_init();
   if (!_ecore_x_resource_db) return;
   fd = eina_file_mkstemp("ecore-x-resource-XXXXXX", &path);
   if (fd < 0) return;
   XrmPutFileDatabase(_ecore_x_resource_db, path);
   offset = lseek(fd, 0, SEEK_END);
   if (offset > 0)
     {
        lseek(fd, 0, SEEK_SET);
        str = malloc(offset + 1);
        if (str)
          {
             if (read(fd, str, offset) == offset)
               {
                  str[offset] = 0;
                  atom = XInternAtom(_ecore_x_disp, "RESOURCE_MANAGER", False);
                  type = ECORE_X_ATOM_STRING;
                  roots = ecore_x_window_root_list(&num);
                  if (roots)
                    {
                       for (i = 0; i < num; i++)
                         ecore_x_window_prop_property_set(roots[i],
                                                          atom, type,
                                                          8, str, offset);
                       free(roots);
                    }
               }
             free(str);
          }
     }
   eina_tmpstr_del(path);
   close(fd);
   unlink(path);
}
