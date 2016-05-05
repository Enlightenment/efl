#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_private.h"

typedef struct _Efl_Loop_User_Data Efl_Loop_User_Data;
struct _Efl_Loop_User_Data
{
};

static Efl_Loop *
_efl_loop_user_loop_get(Eo *obj, Efl_Loop_User_Data *pd EINA_UNUSED)
{
   return eo_provider_find(obj, EFL_LOOP_CLASS);
}

static void
_efl_loop_user_eo_base_parent_set(Eo *obj, Efl_Loop_User_Data *pd EINA_UNUSED, Eo_Base *parent)
{
   if (parent != NULL && eo_provider_find(parent, EFL_LOOP_CLASS) == NULL)
     return ;

   eo_parent_set(eo_super(obj, EFL_LOOP_USER_CLASS), parent);
}

#include "efl_loop_user.eo.c"
