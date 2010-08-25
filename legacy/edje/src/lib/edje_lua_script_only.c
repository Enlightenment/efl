#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca(size_t);
#endif

#include "edje_private.h"

Eina_Bool
_edje_lua_script_only(Edje *ed)
{
   if ((ed->collection) && (ed->collection->lua_script_only))
      return EINA_TRUE;
   return EINA_FALSE;
}

void
_edje_lua_script_only_init(Edje *ed)
{
   if (ed->collection)
      _edje_lua2_script_init(ed);
}

void
_edje_lua_script_only_shutdown(Edje *ed)
{
   if (ed->collection && ed->L)
      _edje_lua2_script_func_shutdown(ed);
}

void
_edje_lua_script_only_show(Edje * ed)
{
   if (ed->collection && ed->L)
      _edje_lua2_script_func_show(ed);
}

void
_edje_lua_script_only_hide(Edje * ed)
{
   if (ed->collection && ed->L)
      _edje_lua2_script_func_hide(ed);
}

void
_edje_lua_script_only_move(Edje * ed)
{
   if (ed->collection && ed->L)
      _edje_lua2_script_func_move(ed);
}

void
_edje_lua_script_only_resize(Edje * ed)
{
   if (ed->collection && ed->L)
      _edje_lua2_script_func_resize(ed);
}

void
_edje_lua_script_only_message(Edje * ed, Edje_Message * em)
{
   if (ed->collection && ed->L)
      _edje_lua2_script_func_message(ed, em);
}

