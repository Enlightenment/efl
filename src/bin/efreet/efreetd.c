#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <Ecore.h>
#include <Ecore_File.h>

#include "efreetd.h"
#include "efreetd_cache.h"
#include "efreetd_ipc.h"

int efreetd_log_dom = -1;
Eina_Mempool *efreetd_mp_stat = NULL;

void
quit(void)
{
   ecore_main_loop_quit();
}

int
main(int argc, char *argv[])
{
   char path[PATH_MAX + 128], buf[PATH_MAX];
   FILE *log;
   int fd;
   const char *log_file_dir = NULL;
   const char *hostname_str = NULL;

#ifdef HAVE_SYS_RESOURCE_H
   setpriority(PRIO_PROCESS, 0, 19);
#elif _WIN32
   SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
#endif

   if (!eina_init()) return 1;

   efreetd_mp_stat = eina_mempool_add("chained_mempool",
                                      "struct stat", NULL,
                                     sizeof(struct stat), 10);
   if (!efreetd_mp_stat) return 1;

   if (!ecore_init()) goto ecore_error;
   ecore_app_args_set(argc, (const char **)argv);
   if (!ecore_file_init()) goto ecore_file_error;
   if (!ipc_init()) goto ipc_error;
   if (!cache_init()) goto cache_error;

   log_file_dir = eina_environment_tmp_get();
   if (gethostname(buf, sizeof(buf)) < 0)
     hostname_str = "";
   else
     hostname_str = buf;
   snprintf(path, sizeof(path), "%s/efreetd_%s_XXXXXX.log",
            log_file_dir, hostname_str);
   fd = eina_file_mkstemp(path, NULL);
   if (fd < 0)
     {
        ERR("Can't create log file '%s'\b", path);
        goto tmp_error;
     }
   log = fdopen(fd, "wb");
   if (!log) goto tmp_error;
   eina_log_print_cb_set(eina_log_print_cb_file, log);
   efreetd_log_dom = eina_log_domain_register("efreetd", EFREETD_DEFAULT_LOG_COLOR);
   if (efreetd_log_dom < 0)
     {
        EINA_LOG_ERR("Efreet: Could not create a log domain for efreetd.");
        goto tmp_error;
     }

   ecore_main_loop_begin();

   eina_mempool_del(efreetd_mp_stat);

   cache_shutdown();
   ipc_shutdown();
   ecore_file_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(efreetd_log_dom);
   efreetd_log_dom = -1;
   eina_shutdown();
   return 0;

tmp_error:
   cache_shutdown();
cache_error:
   ipc_shutdown();
ipc_error:
   ecore_file_shutdown();
ecore_file_error:
   ecore_shutdown();
ecore_error:
   if (efreetd_log_dom >= 0) eina_log_domain_unregister(efreetd_log_dom);
   efreetd_log_dom = -1;
   eina_shutdown();
   return 1;
}
