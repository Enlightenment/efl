#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_private.h"

#define MY_CLASS EFL_LOOP_ARGS_CLASS

typedef struct _Efl_Loop_Args_Data Efl_Loop_Args_Data;
struct _Efl_Loop_Args_Data
{
   int argc;
   const char **argv;
};

EOLIAN static void
_efl_loop_args_args_set(Eo *obj EINA_UNUSED, Efl_Loop_Args_Data *pd, int argc, const char **argv)
{
   int i;

   if (argc < 0) return;
   for (i = 0; i < pd->argc; i++) eina_stringshare_del(pd->argv[i]);
   free(pd->argv);
   pd->argc = argc;
   if (argc > 0)
     {
        pd->argv = malloc(argc * sizeof(const char *));
        for (i = 0; i < argc; i++) pd->argv[i] = eina_stringshare_add(argv[i]);
     }
}

EOLIAN static int
_efl_loop_args_arg_num_get(Eo *obj EINA_UNUSED, Efl_Loop_Args_Data *pd)
{
   return pd->argc;
}

EOLIAN const char *
_efl_loop_args_arg_get(Eo *obj EINA_UNUSED, Efl_Loop_Args_Data *pd, int num)
{
   if ((num < 0) || (num >= pd->argc)) return NULL;
   return pd->argv[num];
}

EOLIAN static void
_efl_loop_args_eo_base_destructor(Eo *obj EINA_UNUSED, Efl_Loop_Args_Data *pd)
{
   int i;

   for (i = 0; i < pd->argc; i++) eina_stringshare_del(pd->argv[i]);
   free(pd->argv);
   pd->argv = NULL;
   pd->argc = 0;
   eo_destructor(eo_super(obj, MY_CLASS));
}

#include "efl_loop_args.eo.c"
