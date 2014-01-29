#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "evas_engine.h"
#include <dlfcn.h>

static int _init_count = 0;

/* the actual Manager device returned from init of the library */
static void *bufmgr;

/* the actual library we successfully dlopen'd */
static void *libbufmgr;

/* pointers to library functions that we need */
void *(*libbufmgr_init)(int fd);
void (*libbufmgr_shutdown)(void *mgr);

//void *(*libbufmgr_buffer_create)(void *mgr);

int 
evas_buffer_manager_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (++_init_count != 1) return _init_count;

   /* try to dlopen various buffer managers until we find one that works
    * (libgbm, libtbm, etc)
    * 
    * NB: as I am testing this on desktop and do not have libtbm, I am 
    * just going to code for that. Feel free to hack in libtbm support */
   libbufmgr = dlopen("libgbm.so", (RTLD_LAZY | RTLD_GLOBAL));
   if (!libbufmgr) 
     libbufmgr = dlopen("libgbm.so.1", (RTLD_LAZY | RTLD_GLOBAL));
   if (!libbufmgr) 
     libbufmgr = dlopen("libgbm.so.1.0.0", (RTLD_LAZY | RTLD_GLOBAL));
   if (!libbufmgr) return --_init_count;

   /* with the library found, symlink the functions we need */
   libbufmgr_init = dlsym(libbufmgr, "gbm_create_device");
   libbufmgr_shutdown = dlsym(libbufmgr, "gbm_device_destroy");

   return _init_count;
}

int 
evas_buffer_manager_shutdown(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (--_init_count != 0) return _init_count;

   /* call function to shutdown the manager */
   evas_buffer_manager_close();
}

Eina_Bool 
evas_buffer_manager_open(int fd)
{
   Eina_Bool ret = EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (bufmgr) return EINA_TRUE;

   /* call any init functions required by the dlsym'd buffer manager */
   if (libbufmgr_init)
     {
        if ((bufmgr = libbufmgr_init(fd)))
          ret = EINA_TRUE;
     }

   return ret;
}

void 
evas_buffer_manager_close(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* call any shutdown functions required by the dlsym'd buffer manager */
   if (libbufmgr_shutdown)
     libbufmgr_shutdown(bufmgr);

   bufmgr = NULL;
}
