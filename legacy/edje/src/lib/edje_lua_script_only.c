/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

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

int
_edje_lua_script_only(Edje * ed)
{
   if ((ed->collection) && (ed->collection->L) &&
       (ed->collection->lua_script_only))
      return 1;
   return 0;
}

void
_edje_lua_script_only_init(Edje * ed)
{
   if (ed->collection && ed->collection->L)
     {
	 ed->L = _edje_lua_new_thread(ed->collection->L); // freed in _edje_lua_script_only_shutdown
	 _edje_lua_new_reg(ed->collection->L, -1, ed->L); // freed in _edje_lua_script_only_shutdown
	 lua_pop(ed->collection->L, 1); /* thread */

	 lua_State *L = ed->L;
	 _edje_lua_script_fn_new(ed);
	 lua_pop(L, 1); /* script */

	 lua_getglobal(L, "init");
	 if (!lua_isnil (L, -1))
	   {
	      int err_code;

	      lua_pushvalue(L, LUA_GLOBALSINDEX); /* set function environment from collection thread to edje object thread */
	      lua_setfenv(L, -2);
	      _edje_lua_get_reg(L, ed);

	      if ((err_code = lua_pcall(L, 1, 0, 0)))
		_edje_lua_error(L, err_code);
	   }
	 else
	   lua_pop(L, 1);
     }
}

void
_edje_lua_script_only_shutdown(Edje * ed)
{
   if (ed->collection && ed->collection->L && ed->L)
     {
	lua_State *L = ed->L;
	lua_getglobal(L, "shutdown");
	if (!lua_isnil (L, -1))
	  {
	     int err_code;

	     lua_pushvalue(L, LUA_GLOBALSINDEX);
	     lua_setfenv(L, -2);
	     _edje_lua_get_reg(L, ed);

	     if ((err_code = lua_pcall(L, 1, 0, 0)))
	       _edje_lua_error(L, err_code);
	  }
	else
	  lua_pop (L, 1);
     }
}

void
_edje_lua_script_only_show(Edje * ed)
{
   if (ed->collection && ed->collection->L && ed->L)
     {
	lua_State *L = ed->L;
	lua_getglobal(L, "show");
	if (!lua_isnil (L, -1))
	  {
	     int err_code;

	     lua_pushvalue(L, LUA_GLOBALSINDEX);
	     lua_setfenv(L, -2);
	     _edje_lua_get_reg(L, ed);

	     if ((err_code = lua_pcall(L, 1, 0, 0)))
	       _edje_lua_error(L, err_code);
	  }
	else
	  lua_pop (L, 1);
     }
}

void
_edje_lua_script_only_hide(Edje * ed)
{
   if (ed->collection && ed->collection->L && ed->L)
     {
	lua_State *L = ed->L;
	lua_getglobal(L, "hide");
	if (!lua_isnil (L, -1))
	  {
	     int err_code;

	     lua_pushvalue(L, LUA_GLOBALSINDEX);
	     lua_setfenv(L, -2);
	     _edje_lua_get_reg(L, ed);

	     if ((err_code = lua_pcall(L, 1, 0, 0)))
	       _edje_lua_error(L, err_code);
	  }
	else
	  lua_pop (L, 1);
     }
}

void
_edje_lua_script_only_move(Edje * ed)
{
   if (ed->collection && ed->collection->L && ed->L)
     {
	lua_State *L = ed->L;
	lua_getglobal(L, "move");
	if (!lua_isnil (L, -1))
	  {
	     int err_code;

	     lua_pushvalue(L, LUA_GLOBALSINDEX);
	     lua_setfenv(L, -2);
	     _edje_lua_get_reg(L, ed);
	     lua_pushnumber(L, ed->x);
	     lua_pushnumber(L, ed->y);

	     if ((err_code = lua_pcall(L, 3, 0, 0)))
	       _edje_lua_error(L, err_code);
	  }
	else
	  lua_pop (L, 1);
     }
}

void
_edje_lua_script_only_resize(Edje * ed)
{
   if (ed->collection && ed->collection->L && ed->L)
     {
	lua_State *L = ed->L;
	lua_getglobal(L, "resize");
	if (!lua_isnil (L, -1))
	  {
	     int err_code;

	     lua_pushvalue(L, LUA_GLOBALSINDEX);
	     lua_setfenv(L, -2);
	     _edje_lua_get_reg(L, ed);
	     lua_pushnumber(L, ed->w);
	     lua_pushnumber(L, ed->h);

	     if ((err_code = lua_pcall(L, 3, 0, 0)))
	       _edje_lua_error(L, err_code);
	  }
	else
	  lua_pop (L, 1);
     }
}

void
_edje_lua_script_only_message(Edje * ed, Edje_Message * em)
{
   if (ed->collection && ed->collection->L && ed->L)
     {
	lua_State *L = ed->L;
	lua_getglobal(L, "message");
	if (!lua_isnil (L, -1))
	  {
	     int nargs = 3;
	     int err_code;
	     int count;
	     int i;

	     lua_pushvalue(L, LUA_GLOBALSINDEX);
	     lua_setfenv(L, -2);
	     _edje_lua_get_reg(L, ed);
	     lua_pushnumber(L, em->type);
	     lua_pushnumber(L, em->id);
	     switch (em->type)
	       {
		case EDJE_MESSAGE_NONE:
		   break;
		case EDJE_MESSAGE_SIGNAL:
		   break;
		case EDJE_MESSAGE_STRING:
		   lua_pushstring(L, ((Edje_Message_String *) em->msg)->str);
		   nargs += 1;
		   break;
		case EDJE_MESSAGE_INT:
		   lua_pushnumber(L, ((Edje_Message_Int *) em->msg)->val);
		   nargs += 1;
		   break;
		case EDJE_MESSAGE_FLOAT:
		   lua_pushnumber(L, ((Edje_Message_Float *) em->msg)->val);
		   nargs += 1;
		   break;
		case EDJE_MESSAGE_STRING_SET:
		   count = ((Edje_Message_String_Set *) em->msg)->count;
		   lua_createtable(L, count, 0);
		   for (i = 0; i < count; i++)
		     {
			lua_pushstring(L, ((Edje_Message_String_Set *) em->msg)->str[i]);
			lua_rawseti(L, -2, i + 1);
		     }
		   nargs += 1;
		   break;
		case EDJE_MESSAGE_INT_SET:
		   count = ((Edje_Message_Int_Set *) em->msg)->count;
		   lua_createtable(L, count, 0);
		   for (i = 0; i < count; i++)
		     {
			lua_pushnumber(L, ((Edje_Message_Int_Set *) em->msg)->val[i]);
			lua_rawseti(L, -2, i + 1);
		     }
		   nargs += 1;
		   break;
		case EDJE_MESSAGE_FLOAT_SET:
		   count = ((Edje_Message_Float_Set *) em->msg)->count;
		   lua_createtable(L, count, 0);
		   for (i = 0; i < count; i++)
		     {
			lua_pushnumber(L, ((Edje_Message_Float_Set *) em->msg)->val[i]);
			lua_rawseti(L, -2, i + 1);
		     }
		   nargs += 1;
		   break;
		case EDJE_MESSAGE_STRING_INT:
		   lua_pushstring(L, ((Edje_Message_String_Int *) em->msg)->str);
		   lua_pushnumber(L, ((Edje_Message_String_Int *) em->msg)->val);
		   nargs += 2;
		   break;
		case EDJE_MESSAGE_STRING_FLOAT:
		   lua_pushstring(L, ((Edje_Message_String_Float *) em->msg)->str);
		   lua_pushnumber(L, ((Edje_Message_String_Float *) em->msg)->val);
		   nargs += 2;
		   break;
		case EDJE_MESSAGE_STRING_INT_SET:
		   lua_pushstring(L, ((Edje_Message_String_Int_Set *) em->msg)->str);
		   count = ((Edje_Message_String_Int_Set *) em->msg)->count;
		   lua_createtable(L, count, 0);
		   for (i = 0; i < count; i++)
		     {
			lua_pushnumber(L, ((Edje_Message_String_Int_Set *) em->msg)->val[i]);
			lua_rawseti(L, -2, i + 1);
		     }
		   nargs += 2;
		   break;
		case EDJE_MESSAGE_STRING_FLOAT_SET:
		   lua_pushstring(L, ((Edje_Message_String_Float_Set *) em->msg)->str);
		   count = ((Edje_Message_String_Float_Set *) em->msg)->count;
		   lua_createtable(L, count, 0);
		   for (i = 0; i < count; i++)
		     {
			lua_pushnumber(L, ((Edje_Message_String_Float_Set *) em->msg)->val[i]);
			lua_rawseti(L, -2, i + 1);
		     }
		   nargs += 2;
		   break;
		default:
		   break;
	       }

	     if ((err_code = lua_pcall(L, nargs, 0, 0)))
	       _edje_lua_error(L, err_code);
	  }
	else
	  lua_pop (L, 1);
     }
}

