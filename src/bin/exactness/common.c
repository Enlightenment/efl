#include "common.h"

static int _verbose = 0;

void
ex_printf(int verbose, const char *fmt, ...)
{
   va_list ap;
   if (!_verbose || verbose > _verbose) return;

   va_start(ap, fmt);
   vprintf(fmt, ap);
   va_end(ap);
}

int
ex_prg_invoke(const char *full_path, int argc, char **argv, Eina_Bool player)
{
   Eina_Value *ret__;
   int real__;

   void (*efl_main)(void *data, const Efl_Event *ev);
   int (*elm_main)(int argc, char **argv);
   int (*c_main)(int argc, char **argv);
   Eina_Module *h = eina_module_new(full_path);
   if (!h || !eina_module_load(h))
     {
        fprintf(stderr, "Failed loading %s.\n", full_path);
        if (h) eina_module_free(h);
        return 1;
     }
   efl_main = eina_module_symbol_get(h, "efl_main");
   elm_main = eina_module_symbol_get(h, "elm_main");
   c_main = eina_module_symbol_get(h, "main");
   _evas_new = eina_module_symbol_get(h, "evas_new");
   if (!_evas_new)
     {
        fprintf(stderr, "Failed loading symbol 'evas_new' from %s.\n", full_path);
        eina_module_free(h);
        return 1;
     }
   if (efl_main)
     {
        elm_init(argc, argv);
        if (player) elm_theme_overlay_add(NULL, DATA_DIR"/exactness_play.edj");
        efl_event_callback_add(efl_main_loop_get(), EFL_LOOP_EVENT_ARGUMENTS, efl_main, NULL);
        ret__ = efl_loop_begin(efl_main_loop_get());
        real__ = efl_loop_exit_code_process(ret__);
        elm_shutdown();
     }
   else if (elm_main)
     {
        elm_init(argc, argv);
        if (player) elm_theme_overlay_add(NULL, DATA_DIR"/exactness_play.edj");
        real__ = elm_main(argc, argv);
        elm_shutdown();
     }
   else if (c_main)
     {
        real__ = c_main(argc, argv);
     }
   else
     {
        fprintf(stderr, "Failed loading symbol 'efl_main', 'elm_main' or 'main' from %s.\n", full_path);
        eina_module_free(h);
        real__ = 1;
     }
   return real__;
}

Eina_Stringshare *
ex_prg_full_path_guess(const char *prg)
{
   char full_path[PATH_MAX];
   if (strchr(prg, '/')) return eina_stringshare_add(prg);
   char *env_path = eina_strdup(getenv("PATH"));
   Eina_Stringshare *ret = NULL;
   char *paths = env_path;

   while (paths && *paths && !ret)
     {
        char *real_path;
        char *colon = strchr(paths, ':');
        if (colon) *colon = '\0';

        sprintf(full_path, "%s/%s", paths, prg);
        real_path = ecore_file_realpath(full_path);
        if (*real_path)
          {
             ret = eina_stringshare_add(real_path);
             // check if executable
          }
        free(real_path);

        paths += strlen(paths);
        if (colon) paths++;
     }
   free(env_path);
   return ret;
}
