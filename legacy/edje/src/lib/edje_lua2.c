#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

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

//--------------------------------------------------------------------------//
#ifdef LUA2
#define MAX_LUA_MEM (4 * (1024 * 1024))


//--------------------------------------------------------------------------//
typedef struct _Edje_Lua_Alloc Edje_Lua_Alloc;
typedef struct _Edje_Lua_Obj   Edje_Lua_Obj;
typedef struct _Edje_Lua_Timer Edje_Lua_Timer;
typedef struct _Edje_Lua_Anim  Edje_Lua_Anim;

//--------------------------------------------------------------------------//
struct _Edje_Lua_Alloc
{
   size_t max, cur;
};

struct _Edje_Lua_Obj
{
   EINA_INLIST;
   
   Edje         *ed;
   void        (*free_func) (void *obj);
};
  
struct _Edje_Lua_Timer
{
   Edje_Lua_Obj     obj;
   Ecore_Timer     *timer;
   int              fn_ref;
};

struct _Edje_Lua_Anim
{
   Edje_Lua_Obj     obj;
   Ecore_Animator  *anim;
   int              fn_ref;
};


//--------------------------------------------------------------------------//
static int _elua_obj_gc(lua_State *L);
static int _elua_obj_del(lua_State *L);

static int _elua_echo(lua_State *L);

static int _elua_timer(lua_State *L);
static int _elua_anim(lua_State *L);


//--------------------------------------------------------------------------//
static lua_State *lstate = NULL;

static const struct luaL_reg _elua_edje_api [] =
{
     {"echo",  _elua_echo}, // test func - echo (i know we have print. test)
   
     {"del", _elua_obj_del}, // generic del any object created for edje
   
     {"timer", _elua_timer}, // add timer
     {"anim",  _elua_anim}, // add animator (not done yet)
   
     {NULL, NULL} // end
};
static const struct luaL_reg _elua_edje_meta [] =
{
     {"__gc", _elua_obj_gc}, // garbage collector func for edje objects
   
     {NULL, NULL} // end
};
static const luaL_Reg _elua_libs[] =
{
     {"", luaopen_base},
//     {LUA_LOADLIBNAME, luaopen_package}, // disable this lib - don't want
     {LUA_TABLIBNAME, luaopen_table},
//     {LUA_IOLIBNAME, luaopen_io}, // disable this lib - don't want
     {LUA_OSLIBNAME, luaopen_os},
     {LUA_STRLIBNAME, luaopen_string},
     {LUA_MATHLIBNAME, luaopen_math},
//     {LUA_DBLIBNAME, luaopen_debug}, // disable this lib - don't want

     {NULL, NULL} // end
};
static const char *_elua_key = "key";


//--------------------------------------------------------------------------//
static void *
_elua_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
   Edje_Lua_Alloc *ela = ud;
   void *ptr2;
   
   ela->cur += nsize - osize;
   if (ela->cur > ela->max)
     {
        ERR("Edje Lua memory limit of %zu bytes reached (%zu allocated)",
            ela->max, ela->cur);
        return NULL;
     }
   if (nsize == 0)
     {
        free(ptr);
        return NULL;
     }
   
   ptr2 = realloc(ptr, nsize);
   if (ptr2) return ptr2;
   ERR("Edje Lua cannot re-allocate %i bytes\n", nsize);
   return ptr2;
}

static int
_elua_custom_panic(lua_State *L)
{
   ERR("Lua Panic!!!!");
   return 1;
}


//-------------
void
_edje_lua2_error_full(const char *file, const char *fnc, int line,
                      lua_State *L, int err_code)
{
   char *err_type;
   
   switch (err_code)
     {
     case LUA_ERRRUN:
        err_type = "runtime";
        break;
     case LUA_ERRSYNTAX:
        err_type = "syntax";
        break;
     case LUA_ERRMEM:
        err_type = "memory allocation";
        break;
     case LUA_ERRERR:
        err_type = "error handler";
        break;
     default:
        err_type = "unknown";
        break;
     }
   eina_log_print
     (_edje_default_log_dom, EINA_LOG_LEVEL_ERR,  file, fnc, line,
      "Lua %s error: %s", err_type, lua_tostring(L, -1));
}

static void
_elua_init(void)
{
   static Edje_Lua_Alloc ela = { MAX_LUA_MEM, 0 };
   const luaL_Reg *l;
   
   if (lstate) return;
   lstate = lua_newstate(_elua_alloc, &ela); //0
   lua_atpanic(lstate, _elua_custom_panic); //0

// FIXME: figure out optimal gc settings later   
//   lua_gc(lstate, LUA_GCSETPAUSE, 200); //0
//   lua_gc(lstate, LUA_GCSETSTEPMUL, 200); //0

   for (l = _elua_libs; l->func; l++)
     {
        lua_pushcfunction(lstate, l->func);
        lua_pushstring(lstate, l->name);
        lua_call(lstate, 1, 0);
     }
   
   luaL_register(lstate, "edje", _elua_edje_api); //+1
   
   luaL_newmetatable(lstate, "edje"); //+1
   
   luaL_register(lstate, 0, _elua_edje_meta); //0
   
   lua_pushliteral(lstate, "__index"); //+1
   lua_pushvalue(lstate, -3); //+1
   lua_rawset(lstate, -3); //-2
   lua_pop(lstate, 2);
}


//-------------
static void
_elua_table_ptr_set(lua_State *L, const void *key, const void *val)
{
   lua_pushlightuserdata(L, (void *)key); //+1
   lua_pushlightuserdata(L, (void *)val); //+1
   lua_settable(L, LUA_REGISTRYINDEX); //-2
}

static const void *
_elua_table_ptr_get(lua_State *L, const void *key)
{
   const void *ptr;
   lua_pushlightuserdata(L, (void *)key); //+1
   lua_gettable(L, LUA_REGISTRYINDEX); //0
   ptr = lua_topointer(L, -1); //0
   lua_pop(L, 1);
   return ptr;
}

static void
_elua_table_ptr_del(lua_State *L, const void *key)
{
   lua_pushlightuserdata(L, (void *)key); //+1
   lua_pushnil(L); //+1
   lua_settable(L, LUA_REGISTRYINDEX); //-2
}

static void
_elua_gc(lua_State *L)
{
   lua_gc(L, LUA_GCCOLLECT, 0); //0
}

//-------------
static Edje_Lua_Obj *
_elua_obj_new(lua_State *L, Edje *ed, int size)
{
   Edje_Lua_Obj *obj;
   
   obj = (Edje_Lua_Obj *)lua_newuserdata(L, size); //+1
   memset(obj, 0, size);
   ed->lua_objs = eina_inlist_append(ed->lua_objs, EINA_INLIST_GET(obj));
   luaL_getmetatable(L, "edje"); //+1
   lua_setmetatable(L, -2); //-1
   obj->ed = ed;
   return obj;
}

static void
_elua_obj_free(lua_State *L, Edje_Lua_Obj *obj)
{
   if (!obj->free_func) return;
   obj->free_func(obj);
   obj->ed->lua_objs = eina_inlist_remove(obj->ed->lua_objs, EINA_INLIST_GET(obj));
   obj->free_func = NULL;
   obj->ed = NULL;
}

//-------------
static int
_elua_obj_gc(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   if (!obj) return 0;
   _elua_obj_free(L, obj);
   return 0;
}

static int
_elua_obj_del(lua_State *L)
{
   return _elua_obj_gc(L);
}

//-------------
static int
_elua_echo(lua_State *L)
{
   const char *string = luaL_checkstring(L, 1); //0
   printf("%s\n", string);
   return 1;
}


//-------------
static int
_elua_timer_cb(void *data)
{
   Edje_Lua_Timer *elt = data;
   int ret = 0;
   int err;
   
   if (!elt->obj.ed) return 0;
   lua_rawgeti(lstate, LUA_REGISTRYINDEX, elt->fn_ref); //+1
   if ((err = lua_pcall(lstate, 0, 1, 0))) //+1
     {
        _edje_lua2_error(lstate, err); //0
        _elua_obj_free(lstate, (Edje_Lua_Obj *)elt);
        _elua_gc(lstate);
        return 0;
     }
   ret = luaL_checkint(lstate, -1); //0
   lua_pop(lstate, 1);
   if (ret == 0)
     _elua_obj_free(lstate, (Edje_Lua_Obj *)elt);
   _elua_gc(lstate);
   return ret;
}

static void
_elua_timer_free(void *obj)
{
   Edje_Lua_Timer *elt = (Edje_Lua_Timer *)obj;
   luaL_unref(lstate, LUA_REGISTRYINDEX, elt->fn_ref); //0
   elt->fn_ref  = 0;
   ecore_timer_del(elt->timer);
   elt->timer = NULL;
}

static int
_elua_timer(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   Edje_Lua_Timer *elt;
   double val;
   
   val = luaL_checknumber(L, 1);
   luaL_checkany(L, 2);
  
   elt = (Edje_Lua_Timer *)_elua_obj_new(L, ed, sizeof(Edje_Lua_Timer)); //+1
   elt->obj.free_func = _elua_timer_free;
   elt->timer = ecore_timer_add(val, _elua_timer_cb, elt);
   lua_pushvalue(L, 2); //+1
   elt->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX);
//   lua_pop(ed->L, 2); // don't pop - return back to lua
   _elua_gc(lstate);
   return 1;
}


//-------------
static int
_elua_anim(lua_State *L)
{
   // xxxxx
   // not done yet - but do like timer (but 1 arg only - no timeout val)
   // xxxxx
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   return 1;
}


//-------------
void
_edje_lua2_script_init(Edje *ed)
{
   if (ed->L) return;
   _elua_init();
   ed->L = lstate;

   lua_newtable(ed->L); //+1
   ed->lua_ref = luaL_ref(ed->L, LUA_REGISTRYINDEX); //+1
//   lua_pushvalue(ed->L, LUA_GLOBALSINDEX);
//   lua_setfield(ed->L, -2, "__index");
//   lua_setmetatable(ed->L, -2);
//   lua_setfenv(ed->L, -2);
   
   _elua_table_ptr_set(ed->L, _elua_key, ed); //0
//   lua_pop(ed->L, 1); // our current script stack anyway
}

void
_edje_lua2_script_shutdown(Edje *ed)
{
   if (!ed->L) return;
   lua_getfenv(ed->L, -1);
   lua_pushnil(ed->L);
   luaL_unref(ed->L, LUA_REGISTRYINDEX, ed->lua_ref); //0
   lua_gc(ed->L, LUA_GCCOLLECT, 0); //0
   
   while (ed->lua_objs)
     {
        Edje_Lua_Obj *obj = (Edje_Lua_Obj *)ed->lua_objs;
        if (obj->free_func)
          {
             ERR("uncollected Lua object %p", obj);
             _elua_obj_free(ed->L, obj);
          }
        else
          {
             ERR("dangling Lua object %p", obj);
             ed->lua_objs = eina_inlist_remove(ed->lua_objs, ed->lua_objs);
          }
     }
   
   ed->L = NULL;
}

void
_edje_lua2_script_load(Edje_Part_Collection *edc, void *data, int size)
{
   int err;
   
   _elua_init();
   
   err = luaL_loadbuffer(lstate, data, size, "edje_lua_script"); //+1
   if (err)
     {
        if (err == LUA_ERRSYNTAX)
          ERR("lua load syntax error: %s", lua_tostring(lstate, -1)); //0
        else if (err == LUA_ERRMEM)
          ERR("lua load memory allocation error: %s", lua_tostring(lstate, -1)); //0
     }
   if ((err = lua_pcall(lstate, 0, 0, 0))) //0
     _edje_lua2_error(lstate, err); //0
}

void
_edje_lua2_script_unload(Edje_Part_Collection *edc)
{
   lua_gc(lstate, LUA_GCCOLLECT, 0); //0
}

#endif
