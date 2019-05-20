#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <Ecore_Evas.h>

#if defined(_WIN32) || defined(EXOTIC_NO_SIGNAL)
# define NO_SIGNAL
#else
# include <signal.h>
#endif

#undef EINA_LOG_DOMAIN_DEFAULT
#define EINA_LOG_DOMAIN_DEFAULT _log_dom
static int _log_dom = -1;

const Ecore_Getopt optdesc = {
  "ecore_evas_convert",
  "%prog [options] <filename-source> <filename-destination>",
  PACKAGE_VERSION,
  "(C) 2014 Enlightenment",
  "BSD with advertisement clause",
  "Simple application to convert image.",
  0,
  {
    ECORE_GETOPT_STORE_INT('q', "quality", "define encoding quality in percent."),
    ECORE_GETOPT_STORE_TRUE('c', "compress", "define if data should be compressed."),
    ECORE_GETOPT_STORE_STR('e', "encoding", "define the codec (for TGV files: 'etc1', 'etc2', 'etc1+alpha')"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

typedef struct _Save_Job {
   const char *output;
   const char *flags;
   Evas_Object *im;
   int ret;
} Save_Job;

static Save_Job job = { NULL, NULL, NULL, -1 };

static void
_save_do(void *data EINA_UNUSED)
{
   job.ret = 0;
   if (!evas_object_image_save(job.im, job.output, NULL, job.flags))
     {
        EINA_LOG_ERR("Could not convert file to '%s'.", job.output);
        job.ret = 1;
     }

   ecore_main_loop_quit();
}

#ifndef NO_SIGNAL
static void
_sigint(int sig EINA_UNUSED)
{
   EINA_LOG_ERR("Image save interrupted by SIGINT: '%s'.", job.output);
   // eina_file_unlink
   unlink(job.output);
   exit(-1);
}
#endif

int
main(int argc, char *argv[])
{
   Ecore_Evas *ee;
   Evas *e;
   Evas_Object *im;
   int arg_index;
   int quality = -1;
   int r = -1;
   char *encoding = NULL;
   Eina_Bool compress = 1;
   Eina_Bool quit_option = EINA_FALSE;
   Eina_Strbuf *flags = NULL;

   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_INT(quality),
     ECORE_GETOPT_VALUE_BOOL(compress),
     ECORE_GETOPT_VALUE_STR(encoding),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_NONE
   };

   eina_init();
   _log_dom = eina_log_domain_register(argv[0], EINA_COLOR_CYAN);

   ecore_init();
   ecore_evas_init();

   arg_index = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (quit_option) goto end;

   if (arg_index < 0)
     {
        EINA_LOG_ERR("Could not parse argument.");
        goto end;
     }
   if (arg_index + 2 != argc)
     {
        EINA_LOG_ERR("File not correctly specified.");
        goto end;
     }

   ee = ecore_evas_buffer_new(1, 1);
   e = ecore_evas_get(ee);
   if (!e)
     {
        EINA_LOG_ERR("Impossible to create a canvas to do the conversion.");
        goto end;
     }

   flags = eina_strbuf_new();
   eina_strbuf_append_printf(flags, "compress=%d", compress);
   if (quality >= 0)
     eina_strbuf_append_printf(flags, " quality=%d", quality);
   if (encoding)
     eina_strbuf_append_printf(flags, " encoding=%s", encoding);

   im = evas_object_image_add(e);
   evas_object_image_file_set(im, argv[arg_index], NULL);

   if (evas_object_image_load_error_get(im) != EVAS_LOAD_ERROR_NONE)
     {
        EINA_LOG_ERR("Could not open '%s'. Error was \"%s\".",
                       argv[arg_index],
                       evas_load_error_str(evas_object_image_load_error_get(im)));
        goto end;
     }

#ifndef NO_SIGNAL
   // Brutal way of
   signal(SIGINT, _sigint);
#endif

   job.output = argv[arg_index + 1];
   job.flags = eina_strbuf_string_get(flags);
   job.im = im;
   ecore_job_add(_save_do, NULL);
   ecore_main_loop_begin();
   r = job.ret;

 end:
   if (flags) eina_strbuf_free(flags);
   ecore_evas_shutdown();
   ecore_shutdown();

   return r;
}
