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
_edje_lua_script_only_show(Edje *ed)
{
   if (ed->collection && ed->L)
     _edje_lua2_script_func_show(ed);
}

void
_edje_lua_script_only_hide(Edje *ed)
{
   if (ed->collection && ed->L)
     _edje_lua2_script_func_hide(ed);
}

void
_edje_lua_script_only_move(Edje *ed)
{
   if (ed->collection && ed->L)
     _edje_lua2_script_func_move(ed);
}

void
_edje_lua_script_only_resize(Edje *ed)
{
   if (ed->collection && ed->L)
     _edje_lua2_script_func_resize(ed);
}

void
_edje_lua_script_only_message(Edje *ed, Edje_Message *em)
{
   if (ed->collection && ed->L)
     _edje_lua2_script_func_message(ed, em);
}

