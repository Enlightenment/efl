#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_private.h"
/* #include "ecordova_systeminfo.eo.h" */

#include <unistd.h>
#include <dlfcn.h>

static int _ecordova_init_count = 0;
int _ecordova_log_dom = -1;
Eo* _ecordova_systeminfo = NULL;
static Eina_Prefix *_ecordova_pfx = NULL;
static Eina_Array* _registered_engines = NULL;

#ifdef _WIN32
# define ECORDOVA_ENGINE_NAME "module.dll"
#else
# define ECORDOVA_ENGINE_NAME "module.so"
#endif

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORDOVA_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EO_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */


EAPI int
ecordova_init(void)
{
   if (_ecordova_init_count++ > 0)
     return _ecordova_init_count;

   if (!eina_init())
     {
        fputs("Ecordova: Unable to initialize eina\n", stderr);
        return 0;
     }

   _ecordova_log_dom = eina_log_domain_register("ecordova", EINA_COLOR_CYAN);
   if (_ecordova_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create an 'ecordova' log domain");
        goto on_error_1;
     }

   if (!ecore_init())
     {
        ERR("Unable to initialize ecore");
        goto on_error_2;
     }

   if (!eo_init())
     {
        ERR("Unable to initialize ecore");
        goto on_error_2;
     }
   
   /* if (!ecore_file_init()) */
   /*   { */
   /*      ERR("Unable to initialize ecore_file"); */
   /*      goto on_error_3; */
   /*   } */

   /* if (!eio_init()) */
   /*   { */
   /*      ERR("Unable to initialize eio"); */
   /*      goto on_error_4; */
   /*   } */

   /* _ecordova_systeminfo = eo_add(ECORDOVA_SYSTEMINFO_CLASS, NULL, */
   /*                               ecordova_systeminfo_constructor()); */
   /* if (!_ecordova_systeminfo) */
   /*   { */
   /*      ERR("Unable to initialize systeminfo service"); */
   /*      goto on_error_4; */
   /*   } */

   _ecordova_pfx = eina_prefix_new(NULL, ecordova_init,
                                "ECORDOVA", "ecordova", "checkme",
                                PACKAGE_BIN_DIR, PACKAGE_LIB_DIR,
                                PACKAGE_DATA_DIR, PACKAGE_DATA_DIR);
   if (!_ecordova_pfx)
     {
        ERR("Could not get ecordova installation prefix");
        goto on_error_4;
     }
   
   return _ecordova_init_count;

on_error_4:
/*    ecore_file_shutdown(); */

on_error_3:
   ecore_shutdown();

on_error_2:
   eina_log_domain_unregister(_ecordova_log_dom);

on_error_1:
   _ecordova_log_dom = -1;
   eina_shutdown();
   return 0;
}

EAPI int
ecordova_shutdown(void)
{
   if (_ecordova_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        _ecordova_init_count = 0;
        return 0;
     }

   if (--_ecordova_init_count)
     return _ecordova_init_count;

   if(_registered_engines)
     {
       eina_array_free(_registered_engines);
       _registered_engines = NULL;
     }
   
   eo_shutdown();
   /* eo_unref(_ecordova_systeminfo); */
   /* eio_shutdown(); */
   /* ecore_file_shutdown(); */
   ecore_shutdown();
   eina_log_domain_unregister(_ecordova_log_dom);
   _ecordova_log_dom = -1;
   eina_shutdown();
   return 0;
}

static void
_ecordova_engine_load()
{
   Eina_Bool run_in_tree;

   if(!_registered_engines)
     {
        _registered_engines = eina_array_new(4);
        run_in_tree = !!getenv("EFL_RUN_IN_TREE");
        fprintf(stderr, "is in tree? %d\n", (int)run_in_tree); fflush(stderr);

        const char *built_modules[] = {
#ifdef HAVE_TIZEN_CONFIGURATION_MANAGER
          "tizen",
#endif
          "linux",
          NULL
        };
        const char **itr;
        for (itr = built_modules; *itr != NULL; itr++)
          {
             Eina_Module* m;
             char path[PATH_MAX] = {0};

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
             if (getuid() == geteuid())
#endif
               {
                  if (run_in_tree)
                    {
                       fprintf(stderr, "in tree\n"); fflush(stderr);
                       snprintf(path, sizeof(path),
                                "%s/src/modules/ecordova/%s/.libs/" ECORDOVA_ENGINE_NAME,
                                PACKAGE_BUILD_DIR, *itr);
                    }
               }

             if(!path[0])
               snprintf(path, sizeof(path), "%s/ecordova/%s/" ECORDOVA_ENGINE_NAME,
                        eina_prefix_lib_get(_ecordova_pfx), *itr);

             fprintf(stderr, "loading module %s\n", path); fflush(stderr);
             
             m = eina_module_new(path);
             if (!m) continue;

             if (!eina_module_load(m))
               {
                  DBG("Could _NOT_ loaded module %s\n", path);
                  eina_module_free(m);
                  continue;
               }
             fprintf(stderr, "loaded module %s\n", path); fflush(stderr);
             eina_array_push(_registered_engines, m);
          }
     }

   return NULL;
}

void* ecordova_module_symbol_get(const char* symbol)
{
   unsigned int i;
   Eina_Array_Iterator iterator;
   Eina_Module *m;
   void* function = NULL;

   _ecordova_engine_load();

   EINA_ARRAY_ITER_NEXT(_registered_engines, i, m, iterator)
     {
        function = eina_module_symbol_get(m, symbol);
        if(function)
          return function;
     }

   return function;
}



#define ECORDOVA_concat_op_1(a, b) a ## b
#define ECORDOVA_concat_op(a, b) ECORDOVA_concat_op_1(a, b)

#define ECORDOVA_stringize_1(a) #a
#define ECORDOVA_stringize(a) ECORDOVA_stringize_1(a)

#define ECORDOVA_symbol(name)                                           \
  ECORDOVA_stringize(ECORDOVA_concat_op(ECORDOVA_concat_op(ecordova_, name),  _impl_class_get))
#define ECORDOVA_class_get_func(name) ecordova_ ## name ## _class_get

#define ECORDOVA_CLASS_GET(name)                                        \
  EAPI const Eo_Class*                                                  \
  ECORDOVA_class_get_func(name)(void)                                       \
  {                                                                     \
    fprintf(stderr, "%s:%d\n", __func__, __LINE__); fflush(stderr);     \
    static const Eo_Class *(*class_get)(void);                          \
    if(!class_get)                                                      \
      {                                                                 \
    fprintf(stderr, "%s:%d\n", __func__, __LINE__); fflush(stderr);     \
         class_get = ecordova_module_symbol_get(ECORDOVA_symbol(name)); \
         if(!class_get)                                                 \
           {                                                            \
              ERR("Could not find symbol " ECORDOVA_symbol(name) ". returning NULL Eo_Class"); \
              return NULL;                                              \
           }                                                            \
      }                                                                 \
    fprintf(stderr, "%s:%d\n", __func__, __LINE__); fflush(stderr);     \
    return class_get();                                                 \
  }

#define ECORDOVA_define_data(name)              \
  struct ECORDOVA_concat_op(ECORDOVA_concat_op(_Ecordova_, name), _Data) {}; \
  typedef struct ECORDOVA_concat_op(ECORDOVA_concat_op(_Ecordova_, name), _Data) \
  ECORDOVA_concat_op(ECORDOVA_concat_op(Ecordova_, name), _Data);


ECORDOVA_CLASS_GET(console)
ECORDOVA_CLASS_GET(directoryentry)
ECORDOVA_CLASS_GET(directoryreader)
ECORDOVA_CLASS_GET(entry)
ECORDOVA_CLASS_GET(fileentry)
ECORDOVA_CLASS_GET(filewriter)
ECORDOVA_CLASS_GET(file)
ECORDOVA_CLASS_GET(filereader)
ECORDOVA_CLASS_GET(filetransfer)

ECORDOVA_define_data(Console)
ECORDOVA_define_data(DirectoryEntry)
ECORDOVA_define_data(DirectoryReader)
ECORDOVA_define_data(Entry)
ECORDOVA_define_data(FileEntry)
ECORDOVA_define_data(FileWriter)
ECORDOVA_define_data(File)
ECORDOVA_define_data(FileReader)
ECORDOVA_define_data(FileTransfer)

#undef EO_DEFINE_CLASS
#define EO_DEFINE_CLASS(...)

#undef EOAPI
#define EOAPI EAPI

#include "ecordova_console.eo.c"
#include "ecordova_directoryentry.eo.c"
#include "ecordova_directoryreader.eo.c"
#include "ecordova_entry.eo.c"
#include "ecordova_fileentry.eo.c"
#include "ecordova_filewriter.eo.c"
#include "ecordova_file.eo.c"
#include "ecordova_filereader.eo.c"
#include "ecordova_filetransfer.eo.c"
