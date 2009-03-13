#ifndef _EVAS_MODULE_H
#define _EVAS_MODULE_H


/* the module api version */
#define EVAS_MODULE_API_VERSION 1


/* the module types */
typedef enum _Evas_Module_Type
{
   EVAS_MODULE_TYPE_ENGINE,
   EVAS_MODULE_TYPE_IMAGE_LOADER,
   EVAS_MODULE_TYPE_IMAGE_SAVER,
     EVAS_MODULE_TYPE_OBJECT
} Evas_Module_Type;


typedef struct _Evas_Module_Api    Evas_Module_Api;
typedef struct _Evas_Module        Evas_Module;
typedef struct _Evas_Module_Path   Evas_Module_Path;
typedef struct _Evas_Module_Engine Evas_Module_Engine;

/* the module api structure, all modules should define this struct */
struct _Evas_Module_Api
{
   int			version;
   Evas_Module_Type	type;
   const char		*name;
   const char		*author;
};

/* the module structure */
struct _Evas_Module
{
   Evas_Module_Api	*api;
   void			*handle;	/* the dlopen handle */
   char			*path;		/* the path where this modules is */
   char			*name;		/* the name of the dir where this module is */
   struct
     {
	int (*open)(Evas_Module *);
	void (*close)(Evas_Module *);
     } func;
   void		*functions;	/* this are the functions exported by the module */
   void		*data;		/* some internal data for the module i.e the id for engines */

   Evas_Module_Type	type;		/* the type detected by the path */

   int           ref; /* how many refs */
   int           last_used; /* the cycle count when it was last used */

   LK(lock);
//#if defined(HAVE_PTHREAD_H) && defined(BUILD_ASYNC_PRELOAD)
//   pthread_mutex_t lock;
//#endif

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

struct _Evas_Module_Engine
{
   int			id;
};

void         evas_module_paths_init (void);
void         evas_module_init       (void);
Evas_Module *evas_module_find_type  (Evas_Module_Type type, const char *name);
int          evas_module_load       (Evas_Module *em);
void         evas_module_unload     (Evas_Module *em);
void         evas_module_ref        (Evas_Module *em);
void         evas_module_unref      (Evas_Module *em);
void         evas_module_use        (Evas_Module *em);
void         evas_module_clean      (void);
void         evas_module_shutdown   (void);


#endif /* _EVAS_MODULE_H */
