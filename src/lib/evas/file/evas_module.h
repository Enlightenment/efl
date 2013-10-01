#ifndef _EVAS_MODULE_H
#define _EVAS_MODULE_H

/* the internals of the module api use this struct to reference a path with a module type
 * instead of deduce the type from the path.
 * */
struct _Evas_Module_Path
{
   Evas_Module_Type	type;
   char		       *path;
};

void         evas_module_paths_init (void);
void         evas_module_init       (void);
Evas_Module *evas_module_find_type  (Evas_Module_Type type, const char *name);
Evas_Module *evas_module_engine_get(int render_method);
void         evas_module_foreach_image_loader(Eina_Hash_Foreach cb, const void *fdata);
int          evas_module_load       (Evas_Module *em);
void         evas_module_unload     (Evas_Module *em);
void         evas_module_ref        (Evas_Module *em);
void         evas_module_unref      (Evas_Module *em);
void         evas_module_use        (Evas_Module *em);
void         evas_module_clean      (void);
void         evas_module_shutdown   (void);

#endif /* _EVAS_MODULE_H */
