#ifndef _EVAS_MODULE_H
#define _EVAS_MODULE_H


/* the module api version */
#define EVAS_MODULE_API_VERSION 2


/* the module types */
typedef enum _Evas_Module_Type
{
   EVAS_MODULE_TYPE_ENGINE = 0,
   EVAS_MODULE_TYPE_IMAGE_LOADER = 1,
   EVAS_MODULE_TYPE_IMAGE_SAVER = 2,
   EVAS_MODULE_TYPE_OBJECT = 3
} Evas_Module_Type;


typedef struct _Evas_Module_Api    Evas_Module_Api;
typedef struct _Evas_Module        Evas_Module;
typedef struct _Evas_Module_Path   Evas_Module_Path;
typedef struct _Evas_Module_Engine Evas_Module_Engine;
typedef struct _Evas_Module_Public Evas_Module_Public;

/* the module api structure, all modules should define this struct */
struct _Evas_Module_Api
{
   int			version;
   const char		*name;
   const char		*author;

   struct
     {
	int (*open)(Evas_Module *);
	void (*close)(Evas_Module *);
     } func;
};

/* the module structure */
struct _Evas_Module
{
   const Evas_Module_Api *definition;

   void		*functions;	/* this are the functions exported by the module */
   int           id_engine;	/* some internal data for the module i.e the id for engines */

   int           ref; /* how many refs */
   int           last_used; /* the cycle count when it was last used */

   LK(lock);

   unsigned char	loaded : 1;
};


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
EAPI Eina_Bool    evas_module_register   (const Evas_Module_Api *module, Evas_Module_Type type);
EAPI Eina_Bool    evas_module_unregister (const Evas_Module_Api *module, Evas_Module_Type type);

#define EVAS_MODULE_DEFINE(Type, Tn, Name)		\
  Eina_Bool evas_##Tn##_##Name##_init(void)		\
  {							\
     return evas_module_register(&evas_modapi, Type);	\
  }							\
  void evas_##Tn##_##Name##_shutdown(void)		\
  {							\
     evas_module_unregister(&evas_modapi, Type);	\
  }

#define EVAS_EINA_MODULE_DEFINE(Tn, Name)	\
  EINA_MODULE_INIT(evas_##Tn##_##Name##_init);	\
  EINA_MODULE_SHUTDOWN(evas_##Tn##_##Name##_shutdown);

#endif /* _EVAS_MODULE_H */
